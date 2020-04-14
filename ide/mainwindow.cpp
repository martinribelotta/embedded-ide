/*
 * This file is part of Embedded-IDE
 * 
 * Copyright 2019 Martin Ribelotta <martinribelotta@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
#include "regexhtmltranslator.h"
#include "templatemanager.h"
#include "templateitemwidget.h"
#include "templatefile.h"
#include "processlinebufferizer.h"

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
#include <QTextBrowser>
#include <QDialogButtonBox>

#include <QtDebug>

struct LineRange {
    int first, second, idx;
};

Q_DECLARE_METATYPE(LineRange)

using LineRangeList = QList<LineRange>;

class MainWindow::Priv_t {
public:
    ProjectManager *projectManager;
    FileSystemManager *fileManager;
    ProcessManager *pman;
    ConsoleInterceptor *console;
    BuildManager *buildManager;
    LineRangeList lineRanges;
    QString lastDir;
    bool documentOnly = false;
    QByteArray topSplitterState;
    QByteArray docSplitterState;
    QVector<QString> trackedBuildPath;
};


static constexpr auto MAINWINDOW_SIZE = QSize{900, 600};

static QString kindToIcon(const QString& kind)
{
    static const QHash<QString, QString> map{
        { "array", "variable" },
        { "boolean", "variable" },
        { "chapter", "variable" },
        { "enum", "enum" },
        { "enumerator", "enum" },
        { "externvar", "variable" },
        { "function", "function" },
        { "macro", "macro" },
        { "object", "unknown" },
        { "prototype", "function" },
        { "section", "unknown" },
        { "struct", "class" },
        { "symbol", "class" },
        { "typedef", "class" },
        { "union", "class" },
        { "variable", "variable" },
    };
    return map.value(kind, "unknown");
}

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(std::make_unique<Ui::MainWindow>()),
    priv(std::make_unique<Priv_t>())
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->welcomePage);
    ui->bottomLeftStack->setCurrentWidget(ui->actionViewer);

    const struct { QToolButton *b; const char *icon; } buttonmap[] = {
        { ui->buttonDocumentCloseAll, "document-close-all" },
        { ui->buttonReload, "view-refresh" },
        { ui->buttonCloseProject, "document-close" },
        { ui->buttonExport, "document-export" },
        { ui->buttonFindAll, "edit-find-replace" },
        { ui->buttonTools, "run-build-configure" },
        { ui->buttonConfigurationMain, "configure" },
        { ui->buttonDebugLaunch, "debug-init" },
        { ui->buttonDebugRun, "debug-run-v2" },
        { ui->buttonDebugStepOver, "debug-step-over-v2" },
        { ui->buttonDebugStepInto, "debug-step-into-v2" },
        { ui->buttonDebugRunToEOF, "debug-step-out-v2" },
        { ui->buttonDocumentClose, "document-close" },
        { ui->buttonDocumentSave, "document-save" },
        { ui->buttonDocumentCloseAll, "document-close-all" },
        { ui->buttonDocumentSaveAll, "document-save-all" },
        { ui->buttonDocumentReload, "view-refresh" },
        { ui->updateAvailable, "view-refresh" },
        { ui->buttonQuit, "application-exit" },
        { ui->buttonConfiguration, "configure" },
        // { ui->buttonExternalTools, "run-build-configure" },
        { ui->buttonOpenProject, "document-open" },
        { ui->buttonNewProject, "document-new" },
    };
    auto loadIcons = [this, buttonmap]() {
        for (const auto& e: buttonmap)
            e.b->setIcon(QIcon{AppConfig::resourceImage({ "actions", e.icon })});
        auto l = QWidgetList{
            ui->labelConfiguration,
            ui->labelExit,
            ui->labelNewProject,
            ui->labelOpenProject
        };
        for (QWidget *e: l) {
            auto p = e->palette();
            if (AppConfig::instance().useDarkStyle())
                p.setColor(QPalette::Text, p.color(QPalette::Text).lighter());
            else
                p.setColor(QPalette::Text, p.color(QPalette::Text).darker());
            e->setPalette(p);
        }

    };
    loadIcons();
    connect(&AppConfig::instance(), &AppConfig::configChanged, loadIcons);

    ui->buttonReload->setIcon(QIcon(AppConfig::resourceImage({"actions", "view-refresh"})));

    ui->buttonDebugLaunch->setVisible(AppConfig::instance().useDevelopMode());
    ui->updateAvailable->setVisible(false);

    auto tman = new TemplateManager();
    tman->setRepositoryUrl(AppConfig::instance().templatesUrl());
    connect(tman, &TemplateManager::message, [](const QString& msg) {
        qDebug() << "tman msg:" << msg;
    });
    connect(tman, &TemplateManager::errorMessage, [](const QString& msg) {
        qDebug() << "tman err:" << msg;
    });
    connect(tman, &TemplateManager::haveMetadata, [this, tman]() {
        bool canUpdate = false;
        auto list = tman->itemWidgets();
        for(auto *witem: list) {
            const auto& item = witem->templateItem();
            canUpdate = canUpdate ||
                (item.state() == TemplateItem::State::Updatable) ||
                (item.state() == TemplateItem::State::New);
        }
        ui->updateAvailable->setVisible(canUpdate);
    });
    connect(ui->updateAvailable, &QToolButton::clicked, [this, tman]() {
        static constexpr auto SIZE_GROW = 0.9;
        QDialog d(this);
        d.resize(this->size().scaled(this->size() * SIZE_GROW, Qt::KeepAspectRatio));
        auto l = new QVBoxLayout(&d);
        auto bb = new QDialogButtonBox(QDialogButtonBox::Close, &d);
        l->addWidget(tman);
        l->addWidget(bb);
        connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
        connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
        d.exec();
        ui->updateAvailable->hide();
        tman->setParent(nullptr);
        tman->startUpdate();
    });
    tman->startUpdate();


    ui->stackedWidget->setCurrentWidget(ui->welcomePage);
    ui->documentContainer->setComboBox(ui->documentSelector);
    auto version = tr("%1 build at %2").arg(VERSION, BUILD_DATE);
    ui->labelVersion->setText(ui->labelVersion->text().replace("{{version}}", version));
    resize(MAINWINDOW_SIZE);

    priv->pman = new ProcessManager(this);

    priv->console = new ConsoleInterceptor(ui->logView, this);

    auto currentPathTracker = [this](QTextBrowser *b, QString& s) -> bool {
        Q_UNUSED(b)
        auto &stack = priv->trackedBuildPath;
        static const QRegularExpression re(R"(make\[(\d+)\]\: (Entering|Leaving) directory \'([^\']*)\')");
        auto m = re.match(s);
        if (m.hasMatch()) {
            auto level = m.captured(1).toInt();
            auto path = m.captured(3);
            if (stack.size() < level) {
                stack.resize(level);
            }
            stack[level - 1] = path;
        }
        return false;
    };
    auto errorStringFinder = [this](QTextBrowser *b, QString& s) -> bool {
        Q_UNUSED(b)
        static const QRegularExpression errRe(
            R"(^(?<file>.*?):(?<line>\d+):(?<col>\d+)?(?<ddot>:?)(?<msg>.*?)$)",
            QRegularExpression::MultilineOption
        );
        auto mit = errRe.globalMatch(s);
        int count = 0;
        int pos = 0;
        while (mit.hasNext()) {
            auto mm = mit.next();
            int reStart = mm.capturedStart();
            if (reStart > pos) {
                ConsoleInterceptor::writeMessageTo(b, s.mid(pos, reStart - pos));
            }
            QString file = mm.captured("file");
            if (!priv->trackedBuildPath.isEmpty() && !QFileInfo(file).isAbsolute())
                file.prepend(priv->trackedBuildPath.last() + QDir::separator());
            auto line = mm.captured("line");
            auto col = mm.captured("col");
            auto ddot = mm.captured("ddot");
            auto msg = mm.captured("msg");
            auto url = ICodeModelProvider::FileReference(file, line.toInt(), col.toInt(), msg).encode();
            auto err = QString("%2:%3:%4%5 ").arg(file, line, col, ddot);
            ConsoleInterceptor::writeMessageTo(b, err, Qt::red);
            QTextCharFormat linkFmt;
            linkFmt.setAnchor(true);
            linkFmt.setAnchorHref(url.toString());
            linkFmt.setForeground(b->palette().link().color());
            ConsoleInterceptor::writeMessageTo(b, msg, linkFmt);
            pos = mm.capturedEnd();
            count++;
        }
        if (count > 0 && pos < s.length()) {
            ConsoleInterceptor::writeMessageTo(b, s.mid(pos));
            return true;
        }
        return false;
    };
    priv->console->addStdErrFilter(currentPathTracker);
    priv->console->addStdOutFilter(currentPathTracker);
    priv->console->addStdErrFilter(errorStringFinder);
    priv->console->addStdOutFilter(errorStringFinder);

    connect(priv->console->clearButton(), &QToolButton::clicked,
            ui->logView, &QTextBrowser::clear);

    auto makeProc = priv->pman->processFor(BuildManager::PROCESS_NAME);
    connect(makeProc, &QProcess::stateChanged,
            [this](QProcess::ProcessState state) {
                priv->console->killButton()->setEnabled(state == QProcess::Running);
            });
    auto makeLinerize = new ProcessLineBufferizer(ProcessLineBufferizer::MergedChannel, makeProc);
    connect(makeLinerize, &ProcessLineBufferizer::haveLine, this, [this, makeProc](const QString& line) {
            priv->console->appendToConsole(QProcess::StandardError, makeProc, line);
    });
    connect(priv->buildManager, &BuildManager::buildTerminated, makeLinerize, &ProcessLineBufferizer::flush);

    priv->projectManager = new ProjectManager(ui->actionViewer, priv->pman, this);
    priv->projectManager->setCodeModelProvider(new ClangAutocompletionProvider(priv->projectManager, this));
    ui->documentContainer->setProjectManager(priv->projectManager);

    priv->buildManager = new BuildManager(priv->projectManager, priv->pman, this);
    connect(priv->console->killButton(), &QToolButton::clicked,
            priv->buildManager, &BuildManager::cancelBuild);

    priv->fileManager = new FileSystemManager(ui->fileViewer, this);

    connect(ui->logView, &QTextBrowser::anchorClicked, [this](const QUrl& url) {
        auto ref = ICodeModelProvider::FileReference::decode(url);
        ui->documentContainer->openDocumentHere(ref.path, ref.line, ref.column);
        ui->documentContainer->setFocus();
    });

    TextMessageBrocker::instance().subscribe(TextMessages::STDERR_LOG, [this](const QString& msg) {
        priv->console->writeMessage(msg);
    });

    TextMessageBrocker::instance().subscribe(TextMessages::STDOUT_LOG, [this](const QString& msg) {
        priv->console->writeMessage(msg);
    });

    connect(priv->buildManager, &BuildManager::buildStarted, [this]() {
        priv->trackedBuildPath.clear();
        ui->actionViewer->setEnabled(false);
    });
    connect(priv->buildManager, &BuildManager::buildTerminated, [this]() {
        ui->actionViewer->setEnabled(true);
    });
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
        auto m = dynamic_cast<QStandardItemModel*>(ui->recentProjectsView->model());
        m->clear();
        for(const auto& e: AppConfig::instance().recentProjects()) {
            auto item = new QStandardItem(
                QIcon(FileSystemManager::mimeIconPath("text-x-makefile")), e.dir().dirName());
            item->setData(e.absoluteFilePath());
            item->setToolTip(e.absoluteFilePath());
            m->appendRow(item);
        }
    };
    makeRecentProjects();
    connect(new QFileSystemWatcher({AppConfig::instance().projectsPath()}, this),
            &QFileSystemWatcher::directoryChanged, makeRecentProjects);
    connect(ui->recentProjectsView, &QListView::activated, [this](const QModelIndex& m) {
        openProject(dynamic_cast<const QStandardItemModel*>(m.model())->data(m, Qt::UserRole + 1).toString());
    });

    auto openProjectCallback = [this]() {
        auto lastDir = priv->lastDir;
        if (lastDir.isEmpty())
            lastDir = AppConfig::instance().projectsPath();
        auto path = QFileDialog::getOpenFileName(this, tr("Open Project"), lastDir, tr("Makefile (Makefile);;All files (*)"));
        if (!path.isEmpty())
            openProject(path);
    };
    auto newProjectCallback = [this]() {
        NewProjectDialog d(this);
        if (d.exec() == QDialog::Accepted) {
            if (d.isTemplate()) {
                priv->projectManager->createProject(d.absoluteProjectPath(), d.templateFile());
            } else {
                priv->projectManager->createProjectFromTGZ(d.absoluteProjectPath(),
                                                           d.selectedTemplateFile().absoluteFilePath());
            }
        }
    };
    auto openConfigurationCallback = [this]() {
        ConfigWidget d(this);
        if (d.exec())
            d.save();
    };
    auto exportCallback = [this, clearMessageCallback]() {
        auto path = QFileDialog::getSaveFileName(this, tr("New File"), AppConfig::instance().templatesPath(),
                                                 TemplateFile::TEMPLATE_FILEDIALOG_FILTER);
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

    priv->documentOnly = false;
    connect(new QShortcut(QKeySequence("CTRL+SHIFT+C"), this), &QShortcut::activated, [this]() {
        auto state = priv->documentOnly;
        if (state) {
            ui->horizontalSplitterTop->restoreState(priv->topSplitterState);
            ui->splitterDocumentViewer->restoreState(priv->docSplitterState);
        } else {
            priv->topSplitterState = ui->horizontalSplitterTop->saveState();
            priv->docSplitterState = ui->splitterDocumentViewer->saveState();
            constexpr auto DEFAULT_SPLITTER_SIZE = 100;
            ui->horizontalSplitterTop->setSizes({ 0, DEFAULT_SPLITTER_SIZE });
            ui->splitterDocumentViewer->setSizes({ DEFAULT_SPLITTER_SIZE, 0 });
            ui->documentContainer->setFocus();
        }
        priv->documentOnly = !state;
    });

    connect(ui->buttonReload, &QToolButton::clicked, priv->projectManager, &ProjectManager::reloadProject);
    connect(priv->projectManager, &ProjectManager::projectOpened, [this](const QString& makefile) {
        for(auto& btn: ui->projectButtons->buttons()) btn->setEnabled(true);
        QFileInfo mkInfo(makefile);
        auto filepath = mkInfo.absoluteFilePath();
        auto dirpath = mkInfo.absolutePath();
        ui->stackedWidget->setCurrentWidget(ui->mainPage);
        priv->fileManager->openPath(dirpath);
        AppConfig::instance().appendToRecentProjects(filepath);
        AppConfig::instance().save();
        qputenv("CURRENT_PROJECT_FILE", filepath.toLocal8Bit());
        qputenv("CURRENT_PROJECT_DIR", dirpath.toLocal8Bit());
    });
    connect(priv->projectManager, &ProjectManager::projectClosed, [this, makeRecentProjects]() {
        qputenv("CURRENT_PROJECT_FILE", "");
        qputenv("CURRENT_PROJECT_DIR", "");
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
        ui->symbolSelector->setEnabled(false);
        ui->symbolSelector->clear();
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
    connect(ui->documentContainer, &DocumentManager::documentModified,
            [this](const QString& path, IDocumentEditor *iface, bool modify){
        Q_UNUSED(path)
        Q_UNUSED(iface)
        ui->buttonDocumentSave->setEnabled(modify);
        ui->buttonDocumentSaveAll->setEnabled(ui->documentContainer->unsavedDocuments().count() > 0);
    });
    connect(ui->symbolSelector, qOverload<int>(&QComboBox::activated), [this](int idx) {
        auto var = ui->symbolSelector->itemData(idx);
        if (var.isNull())
            return;
        auto meta = var.value<ICodeModelProvider::Symbol>();
        auto ed = ui->documentContainer->documentEditorCurrent();
        if (ed) {
            ed->setCursor({ meta.ref.column, meta.ref.line });
            ed->widget()->setFocus();
        }
    });
    connect(ui->documentContainer, &DocumentManager::documentPositionModified,
            [this](const QString& path, int l, int c) {
        Q_UNUSED(c)
        Q_UNUSED(path)
        int idx = 0;
        for (const auto& e: priv->lineRanges) {
            if (l >= e.first && l < e.second) {
                idx = e.idx;
                break;
            }
        }
        auto b = ui->symbolSelector->blockSignals(true);
        ui->symbolSelector->setCurrentIndex(idx);
        ui->symbolSelector->blockSignals(b);
    });
    auto requestSymbolsForFile =  [this](const QString& path) {
        priv->projectManager->codeModel()->requestSymbolForFile(
                    path, [this](const ICodeModelProvider::SymbolSetMap& items) {
            ui->symbolSelector->clear();
            priv->lineRanges.clear();
            if (items.isEmpty())
                return;
            auto b = ui->symbolSelector->blockSignals(true);
            auto& lineNumbers = priv->lineRanges;
            ui->symbolSelector->addItem(tr("<Select Symbol>"));
            for (auto it = items.cbegin(); it != items.cend(); ++it) {
                ICodeModelProvider::SymbolSet list = it.value();
                for (const auto& e: list.toList()) {
                    lineNumbers += { e.ref.line, e.ref.line, ui->symbolSelector->count() };
                    ui->symbolSelector->addItem(QIcon{AppConfig::instance().resourceImage(
                                                      { "categories", kindToIcon(e.type) }
                                                  )},
                                                tr("[%1] %2").arg(e.type, e.name),
                                                QVariant::fromValue(e));
                }
            }
            if (!lineNumbers.isEmpty()) {
                std::sort(lineNumbers.begin(), lineNumbers.end(),
                          [](const LineRange& a, const LineRange& b) {
                    return a.first < b.first;
                });
                auto prev = lineNumbers.begin();
                for (auto it = ++lineNumbers.begin(); it != lineNumbers.end(); ++it) {
                    prev->second = it->first;
                    prev = it;
                }
                // FIXME: Ideally, second of the last is line number of the doc,
                // but the max int value work same for this purponse
                prev->second = std::numeric_limits<decltype(lineNumbers.end()->second)>::max();
            }
            ui->symbolSelector->setEnabled(ui->symbolSelector->count() > 0);
            if (ui->symbolSelector->count() > 0)
                ui->symbolSelector->setCurrentIndex(0);
            ui->symbolSelector->blockSignals(b);
        });
    };
    connect(ui->documentContainer, &DocumentManager::documentFocushed, enableEdition);
    connect(ui->documentContainer, &DocumentManager::documentClosed, enableEdition);
    connect(ui->documentContainer, &DocumentManager::documentFocushed, requestSymbolsForFile);
    connect(priv->projectManager, &ProjectManager::indexFinished, [requestSymbolsForFile, this]() {
        requestSymbolsForFile(ui->documentContainer->documentCurrent());
    });

    connect(priv->projectManager, &ProjectManager::requestFileOpen, ui->documentContainer, &DocumentManager::openDocument);
    connect(ui->buttonDocumentClose, &QToolButton::clicked, ui->documentContainer, &DocumentManager::closeCurrent);
    connect(ui->buttonDocumentCloseAll, &QToolButton::clicked, ui->documentContainer, &DocumentManager::aboutToCloseAll);
    connect(ui->buttonDocumentSave, &QToolButton::clicked, ui->documentContainer, &DocumentManager::saveCurrent);
    connect(ui->buttonDocumentSaveAll, &QToolButton::clicked, ui->documentContainer, &DocumentManager::saveAll);
    connect(ui->buttonDocumentReload, &QToolButton::clicked, ui->documentContainer, &DocumentManager::reloadDocumentCurrent);

    auto setExternalTools = [this]() {
        auto m = ExternalToolManager::makeMenu(this, priv->pman, priv->projectManager);
        ui->buttonTools->setMenu(m);
        // ui->buttonExternalTools->setMenu(m);
    };
    setExternalTools();

    connect(&AppConfig::instance(), &AppConfig::configChanged, [this, setExternalTools](AppConfig *cfg) {
        if (ui->buttonTools->menu())
            ui->buttonTools->menu()->deleteLater();
        setExternalTools();
        ui->buttonDebugLaunch->setVisible(cfg->useDevelopMode());
    });

    auto findInFilesDialog = new FindInFilesDialog(this);
    auto findInFilesCallback = [this, findInFilesDialog]() {
        auto path = priv->projectManager->projectPath();
        if (!path.isEmpty()) {
            connect(findInFilesDialog, &FindInFilesDialog::queryToOpen,
                    [this](const QString& path, int line, int column)
            {
                activateWindow();
                ui->documentContainer->setFocus();
                ui->documentContainer->openDocumentHere(path, line, column);
            });
            if (findInFilesDialog->findPath().isEmpty())
                findInFilesDialog->setFindPath(path);
            findInFilesDialog->show();
        }
    };

    connect(ui->buttonFindAll, &QToolButton::clicked, findInFilesCallback);
    connect(new QShortcut(QKeySequence("CTRL+SHIFT+F"), this), &QShortcut::activated, findInFilesCallback);

    connect(ui->buttonQuit, &QToolButton::clicked, this, &MainWindow::close);
    connect(new QShortcut(QKeySequence("ALT+F4"), this), &QShortcut::activated, this, &MainWindow::close);

    connect(ui->buttonDebugLaunch, &QToolButton::toggled, [this](bool en) {
        if (en) {
            ui->bottomLeftStack->setCurrentWidget(ui->pageDebug);
        } else {
            ui->bottomLeftStack->setCurrentWidget(ui->actionViewer);
        }
    });
}

MainWindow::~MainWindow()
{
    TextMessageBrocker::instance().disconnect();
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
