#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "projectmanager.h"
#include "filesystemmanager.h"
#include "unsavedfilesdialog.h"
#include "processmanager.h"
#include "consoleinterceptor.h"
#include "buildmanager.h"
#include "idocumenteditor.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QStringListModel>
#include <QScrollBar>
#include <QMessageBox>

#define CURRENT_VERSION "0.7-pre"

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
    ui->labelVersion->setText(ui->labelVersion->text().replace("{{version}}", CURRENT_VERSION));
    resize(800, 600);

    priv->pman = new ProcessManager(this);
    priv->console = new ConsoleInterceptor(ui->logView, priv->pman, BuildManager::PROCESS_NAME, this);
    priv->projectManager = new ProjectManager(ui->actionViewer, priv->pman, this);
    priv->buildManager = new BuildManager(priv->projectManager, priv->pman, this);
    priv->fileManager = new FileSystemManager(ui->fileViewer, this);

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
    connect(ui->buttonCloseProject, &QToolButton::clicked, priv->projectManager, &ProjectManager::closeProject);
    connect(ui->buttonOpenProject, &QToolButton::clicked, [this]() {
        auto lastDir = QDir::homePath();
        auto path = QFileDialog::getOpenFileName(this, tr("Open Project"), lastDir, tr("Makefile (Makefile);;All files (*)"));
        if (!path.isEmpty()) {
            openProject(path);
        }
    });
    connect(priv->projectManager, &ProjectManager::projectOpened, [this](const QString& makefile) {
        for(auto& btn: ui->projectButtons->buttons()) btn->setEnabled(true);
        ui->stackedWidget->setCurrentWidget(ui->mainPage);
        priv->fileManager->openPath(QFileInfo(makefile).absolutePath());
    });
    connect(priv->projectManager, &ProjectManager::projectClosed, [this]() {
        for(auto& btn: ui->projectButtons->buttons()) btn->setEnabled(false);
        ui->stackedWidget->setCurrentWidget(ui->welcomePage);
        ui->documentContainer->closeAll();
        priv->fileManager->closePath();
    });

    auto enableEdition = [this]() {
        auto haveDocuments = ui->documentContainer->documentCount() > 0;
        auto current = ui->documentContainer->documentEditorCurrent();
        auto isModified = current? current->isModified() : false;
        ui->documentSelector->setEnabled(haveDocuments);
        ui->buttonDocumentClose->setEnabled(haveDocuments);
        ui->buttonDocumentCloseAll->setEnabled(haveDocuments);
        ui->buttonDocumentSave->setEnabled(isModified);
        ui->buttonDocumentSaveAll->setEnabled(ui->documentContainer->unsavedDocuments().count() > 0);
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
    connect(ui->buttonDocumentCloseAll, &QToolButton::clicked, ui->documentContainer, &DocumentManager::closeAll);
    connect(ui->buttonDocumentSave, &QToolButton::clicked, ui->documentContainer, &DocumentManager::saveCurrent);
    connect(ui->buttonDocumentSaveAll, &QToolButton::clicked, ui->documentContainer, &DocumentManager::saveAll);

    connect(ui->buttonConfiguration, &QToolButton::clicked, [this]() { ui->stackedWidget->setCurrentWidget(ui->configPage); });
    connect(ui->buttonConfigAccept, &QToolButton::clicked, [this]() { ui->stackedWidget->setCurrentWidget(ui->welcomePage); });
    connect(ui->buttonConfigReject, &QToolButton::clicked, [this]() { ui->stackedWidget->setCurrentWidget(ui->welcomePage); });
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
