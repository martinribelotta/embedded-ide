#include "mainwindow.h"

#include "ui_mainwindow.h"

#include "appconfig.h"
#include "buildmanager.h"
#include "consoleinterceptor.h"
#include "filesystemmanager.h"
#include "idocumenteditor.h"
#include "externaltoolmanager.h"
#include "processmanager.h"
#include "projectmanager.h"
#include "unsavedfilesdialog.h"
#include "version.h"
#include "newprojectdialog.h"
#include "configwidget.h"
#include "findinfilesdialog.h"
#include "clangautocompletionprovider.h"
#include "textmessagebrocker.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QStringListModel>
#include <QScrollBar>
#include <QMenu>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QShortcut>
#include <QStandardItemModel>
#include <QFileSystemWatcher>

#include <QtDebug>

class MainWindow::Priv_t {
public:
    ProjectManager *projectManager;
    FileSystemManager *fileManager;
    ProcessManager *pman;
    ConsoleInterceptor *console;
    BuildManager *buildManager;
};

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow),
    priv(new Priv_t)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->welcomePage);
    ui->documentContainer->setComboBox(ui->documentSelector);
    auto version = tr("%1 build at %2").arg(VERSION).arg(BUILD_DATE);
    ui->labelVersion->setText(ui->labelVersion->text().replace("{{version}}", version));
    resize(900, 600);

    priv->pman = new ProcessManager(this);
    priv->console = new ConsoleInterceptor(ui->logView, priv->pman, BuildManager::PROCESS_NAME, this);
    priv->projectManager = new ProjectManager(ui->actionViewer, priv->pman, this);
    priv->buildManager = new BuildManager(priv->projectManager, priv->pman, this);
    priv->fileManager = new FileSystemManager(ui->fileViewer, this);
    ui->documentContainer->setProjectManager(priv->projectManager);
    priv->projectManager->setCodeModelProvider(new ClangAutocompletionProvider(priv->projectManager, this));

    TextMessageBrocker::instance().subscribe("stderrLog", [this](const QString& msg) {
        priv->console->writeMessage(msg, Qt::darkRed);
    });

    TextMessageBrocker::instance().subscribe("stdoutLog", [this](const QString& msg) {
        priv->console->writeMessage(msg, Qt::darkBlue);
    });

    auto label = new QLabel(ui->actionViewer);
    auto g = new QGridLayout(ui->actionViewer);
    g->addWidget(label, 1, 1);
    g->setRowStretch(0, 1);
    g->setColumnStretch(0, 1);
    TextMessageBrocker::instance().subscribe("actionLabel", label, &QLabel::setText);

    connect(priv->buildManager, &BuildManager::buildStarted, [this]() { ui->actionViewer->setEnabled(false); });
    connect(priv->buildManager, &BuildManager::buildTerminated, [this]() { ui->actionViewer->setEnabled(true); });
    connect(priv->projectManager, &ProjectManager::targetTriggered, [this](const QString& target) {
        ui->logView->clear();
        auto unsaved = ui->documentContainer->unsavedDocuments();
        if (!unsaved.isEmpty()) {
            UnsavedFilesDialog d(unsaved, this);
            if (d.exec() == QDialog::Rejected)
                return;
            ui->documentContainer->saveDocuments(d.checkedForSave());
        }
        priv->buildManager->startBuild(target);
    });
    connect(priv->fileManager, &FileSystemManager::requestFileOpen, ui->documentContainer, &DocumentManager::openDocument);

    auto showMessageCallback = [this](const QString& msg) { priv->console->writeMessage(msg, Qt::darkGreen); };
    auto clearMessageCallback = [this]() { ui->logView->clear(); };
    connect(priv->projectManager, &ProjectManager::exportFinish, showMessageCallback);

    ui->recentProjectsView->setModel(new QStandardItemModel(ui->recentProjectsView));
    auto makeRecentProjects = [this]() {
        auto m = static_cast<QStandardItemModel*>(ui->recentProjectsView->model());
        m->clear();
        for(const auto& e: AppConfig::instance().recentProjects()) {
            auto item = new QStandardItem(QIcon(":/images/mimetypes/text-x-makefile.svg"), e.dir().dirName());
            item->setData(e.absoluteFilePath());
            item->setToolTip(e.absoluteFilePath());
            m->appendRow(item);
        }
    };
    makeRecentProjects();
    connect(new QFileSystemWatcher({AppConfig::instance().projectsPath()}, this),
            &QFileSystemWatcher::directoryChanged, makeRecentProjects);
    connect(ui->recentProjectsView, &QListView::activated, [this](const QModelIndex& m) {
        openProject(static_cast<const QStandardItemModel*>(m.model())->data(m, Qt::UserRole + 1).toString());
    });

    auto openProjectCallback = [this]() {
        auto lastDir = property("lastDir").toString();
        if (lastDir.isEmpty())
            lastDir = AppConfig::instance().projectsPath();
        auto path = QFileDialog::getOpenFileName(this, tr("Open Project"), lastDir, tr("Makefile (Makefile);;All files (*)"));
        if (!path.isEmpty()) {
            openProject(path);
            setProperty("lastDir", QFileInfo(QFileInfo(path).absolutePath()).absolutePath());
        }
    };
    auto newProjectCallback = [this]() {
        NewProjectDialog d(this);
        if (d.exec() == QDialog::Accepted) {
            priv->projectManager->createProject(d.absoluteProjectPath(), d.templateFile());
        }
    };
    auto openConfigurationCallback = [this]() {
        ConfigWidget d(this);
        if (d.exec())
            d.save();
    };
    auto exportCallback = [this, clearMessageCallback]() {
        auto path = QFileDialog::getSaveFileName(this, tr("New File"), AppConfig::instance().templatesPath(),
                                                 tr("Templates (*.template);;All files (*)"));
        if (!path.isEmpty()) {
            if (QFileInfo(path).suffix().isEmpty())
                path.append(".template");
            clearMessageCallback();
            priv->projectManager->exportCurrentProjectTo(path);
        }
    };
    connect(ui->buttonOpenProject, &QToolButton::clicked, openProjectCallback);
    connect(ui->buttonExport, &QToolButton::clicked, exportCallback);
    connect(ui->buttonNewProject, &QToolButton::clicked, newProjectCallback);
    connect(ui->buttonConfiguration, &QToolButton::clicked, openConfigurationCallback);
    connect(ui->buttonConfigurationMain, &QToolButton::clicked, openConfigurationCallback);
    connect(ui->buttonCloseProject, &QToolButton::clicked, priv->projectManager, &ProjectManager::closeProject);
    connect(new QShortcut(QKeySequence("CTRL+N"), this), &QShortcut::activated, newProjectCallback);
    connect(new QShortcut(QKeySequence("CTRL+O"), this), &QShortcut::activated, openProjectCallback);
    connect(new QShortcut(QKeySequence("CTRL+SHIFT+P"), this), &QShortcut::activated, openConfigurationCallback);
    connect(new QShortcut(QKeySequence("CTRL+SHIFT+Q"), this), &QShortcut::activated, priv->projectManager, &ProjectManager::closeProject);

    connect(ui->buttonReload, &QToolButton::clicked, priv->projectManager, &ProjectManager::reloadProject);
    connect(priv->projectManager, &ProjectManager::projectOpened, [this](const QString& makefile) {
        for(auto& btn: ui->projectButtons->buttons()) btn->setEnabled(true);
        ui->stackedWidget->setCurrentWidget(ui->mainPage);
        priv->fileManager->openPath(QFileInfo(makefile).absolutePath());
        AppConfig::instance().appendToRecentProjects(QFileInfo(makefile).absoluteFilePath());
        AppConfig::instance().save();
    });
    connect(priv->projectManager, &ProjectManager::projectClosed, [this, makeRecentProjects]() {
        bool ok = ui->documentContainer->aboutToCloseAll();
        qDebug() << "can close" << ok;
        if (ok) {
            for(auto& btn: ui->projectButtons->buttons()) btn->setEnabled(false);
            makeRecentProjects();
            ui->stackedWidget->setCurrentWidget(ui->welcomePage);
            priv->fileManager->closePath();
        }
    });

    auto enableEdition = [this]() {
        auto haveDocuments = ui->documentContainer->documentCount() > 0;
        auto current = ui->documentContainer->documentEditorCurrent();
        auto isModified = current? current->isModified() : false;
        ui->documentSelector->setEnabled(haveDocuments);
        ui->buttonDocumentClose->setEnabled(haveDocuments);
        ui->buttonDocumentCloseAll->setEnabled(haveDocuments);
        ui->buttonDocumentReload->setEnabled(haveDocuments);
        ui->buttonDocumentSave->setEnabled(isModified);
        ui->buttonDocumentSaveAll->setEnabled(ui->documentContainer->unsavedDocuments().count() > 0);
        if (current) {
            auto m = qobject_cast<QFileSystemModel*>(ui->fileViewer->model());
            if (m) {
                auto i = m->index(current->path());
                if (i.isValid()) {
                    ui->fileViewer->setCurrentIndex(i);
                }
            }
        }
    };
    connect(ui->documentContainer, &DocumentManager::documentModified, [this](const QString& path, IDocumentEditor *iface, bool modify){
        Q_UNUSED(path);
        Q_UNUSED(iface);
        ui->buttonDocumentSave->setEnabled(modify);
        ui->buttonDocumentSaveAll->setEnabled(ui->documentContainer->unsavedDocuments().count() > 0);
    });

    connect(ui->documentContainer, &DocumentManager::documentFocushed, enableEdition);
    connect(ui->documentContainer, &DocumentManager::documentClosed, enableEdition);

    connect(priv->projectManager, &ProjectManager::requestFileOpen, ui->documentContainer, &DocumentManager::openDocument);
    connect(ui->buttonDocumentClose, &QToolButton::clicked, ui->documentContainer, &DocumentManager::closeCurrent);
    connect(ui->buttonDocumentCloseAll, &QToolButton::clicked, ui->documentContainer, &DocumentManager::aboutToCloseAll);
    connect(ui->buttonDocumentSave, &QToolButton::clicked, ui->documentContainer, &DocumentManager::saveCurrent);
    connect(ui->buttonDocumentSaveAll, &QToolButton::clicked, ui->documentContainer, &DocumentManager::saveAll);
    connect(ui->buttonDocumentReload, &QToolButton::clicked, ui->documentContainer, &DocumentManager::reloadDocumentCurrent);

    ui->buttonTools->setMenu(ExternalToolManager::makeMenu(this, priv->pman, priv->projectManager));

    connect(&AppConfig::instance(), &AppConfig::configChanged, [this]() {
        if (ui->buttonTools->menu())
            ui->buttonTools->menu()->deleteLater();
        ui->buttonTools->setMenu(ExternalToolManager::makeMenu(this, priv->pman, priv->projectManager));
    });

    auto findInFilesCallback = [this]() {
        auto path = priv->projectManager->projectPath();
        if (!path.isEmpty()) {
            auto d = new FindInFilesDialog(path, this);
            connect(d, &FindInFilesDialog::finished, d, &QObject::deleteLater);
            connect(d, &FindInFilesDialog::queryToOpen, [d, this](const QString& path, int line, int column) {
                activateWindow();
                ui->documentContainer->setFocus();
                ui->documentContainer->openDocumentHere(path, line, column);
            });
            d->show();
        }
    };
    connect(ui->buttonFindAll, &QToolButton::clicked, findInFilesCallback);
    connect(new QShortcut(QKeySequence("CTRL+SHIFT+F"), this), &QShortcut::activated, findInFilesCallback);
}

MainWindow::~MainWindow()
{
    delete priv;
    delete ui;
}

void MainWindow::openProject(const QString &path)
{
    priv->projectManager->openProject(path);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    auto unsaved = ui->documentContainer->unsavedDocuments();
    if (unsaved.isEmpty()) {
        event->accept();
    } else {
        UnsavedFilesDialog d(unsaved, this);
        event->setAccepted(d.exec() == QDialog::Accepted);
        if (event->isAccepted())
            ui->documentContainer->saveDocuments(d.checkedForSave());
    }
}
