#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "projectexporter.h"
#include "projectnewdialog.h"
#include "projetfromtemplate.h"
#include "configdialog.h"
#include "aboutdialog.h"
#include "debuginterface.h"
#include "mainmenuwidget.h"
#include "appconfig.h"
#include "filedownloader.h"
#include "templatedownloader.h"
#include "findinfilesdialog.h"

#include <QRegularExpression>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QCheckBox>
#include <QMenu>
#include <QUrl>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>


#include <QUrlQuery>
#include <QSettings>
#include <QFont>
#include <QFontInfo>
#include <QFontDatabase>
#include <QMimeDatabase>
#include <QDesktopServices>
#include <QToolButton>
#include <QTimer>
#include <QWidgetAction>
#include <QSystemTrayIcon>

#include <functional>

#include <QtDebug>

static QFileInfoList lastProjectsList(bool includeAllInWorkspace = true) {
    QFileInfoList list;
    if (includeAllInWorkspace) {
        QDir prjDir(AppConfig::mutableInstance().buildDefaultProjectPath());
        foreach(QFileInfo dirInfo, prjDir.entryInfoList(QDir::AllDirs|QDir::NoDotAndDotDot)) {
            QDir dir(dirInfo.absoluteFilePath());
            QFileInfo make(dir.absoluteFilePath("Makefile"));
            if (make.exists()) {
                if (!list.contains(make))
                    list.append(make);
            }
        }
    }
    QSettings sets;
    sets.beginGroup("last_projects");
    foreach(QString k, sets.allKeys()) {
        QFileInfo make(sets.value(k).toString());
        if (make.exists() && !list.contains(make))
                list.append(make);
    }

    return list;
}

static void removeFromLastProject(const QString& path) {
    QSettings sets;
    sets.beginGroup("last_projects");
    foreach(QString key, sets.allKeys()) {
        if (sets.value(key) == path) {
            sets.remove(key);
            break;
        }
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    trayIcon(new QSystemTrayIcon(this)),
    templateDownloader(new TemplateDownloader{})
{
    ui->setupUi(this);
    trayIcon->setIcon(QIcon(":/images/embedded-ide.svg"));
    {
        auto m = new QMenu();
        trayIcon->setContextMenu(m);
        auto f = [this]() {
            templateDownloader->download();
            trayIcon->hide();
        };
        auto *refreshAction = m->addAction(QIcon(":/images/actions/view-refresh.svg"), tr("Update"));
        connect(refreshAction, &QAction::triggered, f);
        connect(trayIcon, &QSystemTrayIcon::messageClicked, f);
        connect(templateDownloader, &TemplateDownloader::newUpdatesAvailables, [this]() {
            trayIcon->show();
            trayIcon->showMessage(
                        tr("Updates available"),
                        tr("New project templates available, click here for details"),
                        QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information), 15000);
        });
    }
    connect(templateDownloader, &TemplateDownloader::finished, [this]() {
        templateDownloader->setSilent(false);
    });

    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    ui->dockWidget->setTitleBarWidget(new QWidget(this));
    ui->projectDock->setTitleBarWidget(new QWidget(this));
    connect(ui->loggerCompiler, &LoggerWidget::openEditorIn, this, &MainWindow::loggerOpenPath);
    connect(ui->projectView, &ProjectView::startBuild,
            [this](const QString &target) {
              if (this->goToBuildStage()) {
                QString projectPath =
                    ui->projectView->projectPath().absolutePath();
                QStringList args;
                args << "-f" << ui->projectView->project() << target;
                ui->loggerCompiler->setWorkingDir(projectPath)
                    .startProcess("make", args);
              }
            });
    connect(ui->projectView, &ProjectView::execTool, [this](const QString& command) {
        QString projectPath = ui->projectView->projectPath().absolutePath();
        ui->loggerCompiler->setWorkingDir(projectPath).startProcess(command);
    });
    ui->projectView->getDebugInterface()->setDocumentArea(ui->centralWidget);
    connect(ui->projectView->getDebugInterface(), &DebugInterface::gdbOutput, [this](const QString& text) {
        ui->loggerDebugger->addText(text, Qt::blue);
        ui->loggerDebugger->addText("<br>", Qt::blue);
    });
    connect(ui->projectView->getDebugInterface(), &DebugInterface::gdbMessage, [this](const QString& text) {
        ui->loggerDebugger->addText(text, Qt::red);
        ui->loggerDebugger->addText("<br>", Qt::red);
    });
    connect(ui->projectView->getDebugInterface(), &DebugInterface::applicationOutput, [this](const QString& text) {
        ui->loggerApplication->addText(text, Qt::blue);
        ui->loggerDebugger->addText("<br>", Qt::blue);
    });

    connect(ui->projectView, &ProjectView::debugChange, [this](bool enabled){
        ui->centralWidget->setDebugToolBarVisible(enabled);
    });

    ui->tabWidget->removeTab(2);
    ui->tabWidget->removeTab(1);

    auto mainMenu = new QMenu(this);
    auto wa = new QWidgetAction(mainMenu);
    auto menuWidget = new MainMenuWidget(mainMenu);
    menuWidget->setProjectList(lastProjectsList());
    wa->setDefaultWidget(menuWidget);
    mainMenu->addAction(wa);
    connect(menuWidget, SIGNAL(projectNew()), this, SLOT(projectNew()));
    connect(menuWidget, SIGNAL(projectOpen()), this, SLOT(projectOpen()));
    connect(menuWidget, &MainMenuWidget::projectOpenAs, [this, mainMenu, menuWidget] (const QFileInfo& info) {
        mainMenu->hide();
        QString name = info.absoluteFilePath();
        if (QFileInfo(name).exists()) {
            ui->projectView->openProject(name);
            QString name = ui->projectView->projectPath().dirName();
            QString path = ui->projectView->project();
            QSettings sets;
            sets.beginGroup("last_projects");
            sets.setValue(name, path);
            sets.sync();
        } else {
            QMessageBox::critical(this, tr("Open Project"), tr("Cannot open %1").arg(name));
            removeFromLastProject(name);
            menuWidget->setProjectList(lastProjectsList());
        }
    });
    connect(ui->projectView, &ProjectView::projectOpened, [menuWidget]() {
        menuWidget->setProjectList(lastProjectsList());
    });
    connect(menuWidget, SIGNAL(projectClose()), this, SLOT(projectClose()));
    connect(menuWidget, SIGNAL(configure()), this, SLOT(configureShow()));
    connect(&(AppConfig::mutableInstance()), SIGNAL(configChanged(AppConfig*)),
            this, SLOT(configChanged(AppConfig*)));
    connect(menuWidget, SIGNAL(help()), this, SLOT(helpShow()));
    connect(menuWidget, SIGNAL(exit()), this, SLOT(close()));

    connect(menuWidget, SIGNAL(projectNew()), mainMenu, SLOT(hide()));
    connect(menuWidget, SIGNAL(projectOpen()), mainMenu, SLOT(hide()));
    connect(menuWidget, SIGNAL(projectClose()), mainMenu, SLOT(hide()));
    connect(menuWidget, SIGNAL(configure()), mainMenu, SLOT(hide()));
    connect(menuWidget, SIGNAL(help()), mainMenu, SLOT(hide()));
    connect(menuWidget, SIGNAL(exit()), mainMenu, SLOT(hide()));
    ui->projectView->setMainMenu(mainMenu);

#ifdef DISABLE_DEBUG_UI
    ui->tabWidget->removeTab(2);
    ui->tabWidget->removeTab(1);
#endif
    configChanged(&AppConfig::mutableInstance());
    statusBar()->showMessage(tr("Application ready..."), 1500);
    statusBar()->hide();
    foreach(QToolButton *b, findChildren<QToolButton*>())
        b->setAutoRaise(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (ui->centralWidget->hasUnsavedChanges()) {
        ui->centralWidget->closeAll();
        if (!ui->centralWidget->hasUnsavedChanges())
            e->accept();
        else
            e->ignore();
    } else
        e->accept();
}

void MainWindow::actionNewFromTemplateEnd(const QString &project, const QString &error)
{
    if (error.isEmpty()) {
        ui->projectView->openProject(project + QDir::separator() + "Makefile");
    } else
        QMessageBox::critical(this, tr("Error"), error);
}

void MainWindow::actionExportFinish(const QString &s)
{
    QString dialogTitle(tr("Export"));
    if (s.isEmpty())
        QMessageBox::information(this, dialogTitle, tr("Success"));
    else
        QMessageBox::warning(this, dialogTitle, s);
}

void MainWindow::on_projectView_fileOpen(const QString &file)
{
    qDebug() << file;
    QFileInfo inf(file);
    QMimeType m = QMimeDatabase().mimeTypeForFile(file, QMimeDatabase::MatchDefault);
    if (inf.suffix().toLower() == "map") {
        ui->centralWidget->mapOpen(file);
    } else if (m.inherits("text/plain") || (inf.size() == 0)) {
        ui->centralWidget->fileOpenAt(file, 0, 0, &ui->projectView->makeInfo());
    } else {
        ui->centralWidget->binOpen(file);
    }
}

void MainWindow::projectNew()
{
    ProjectNewDialog w(this);
    switch(w.exec()) {
    case QDialog::Accepted:
        (new ProjetFromTemplate(w.projectPath(), w.templateText(),
                                this, SLOT(actionNewFromTemplateEnd(QString,QString))))->start();
        break;
    default:
        break;
    }
}

void MainWindow::projectOpen()
{
    QString name = QFileDialog::
            getOpenFileName(this,
                            tr("Open Project"),
                            "Makefile",
                            tr("Makefile (Makefile);;"
                               "Make (*.mk);;"
                               "All Files (*)")
                            );
    if (!name.isEmpty()) {
            ui->projectView->openProject(name);
    }
}

void MainWindow::openProject()
{
    QAction *a = qobject_cast<QAction*>(sender());
    if (a) {
        QString name = a->data().toString();
        if (QFileInfo(name).exists()) {
            ui->projectView->openProject(name);
        } else {
            QMessageBox::critical(this, tr("Open Project"), tr("Cannot open %1").arg(a->text()));
            removeFromLastProject(name);
        }
    }
}

void MainWindow::helpShow()
{
    AboutDialog(this).exec();
}

void MainWindow::projectClose()
{
    ui->projectView->closeProject();
    ui->centralWidget->closeAll();
    ui->loggerCompiler->clearText();
    setWindowTitle(tr("Embedded IDE"));
}

void MainWindow::on_projectView_projectOpened()
{
    setWindowTitle(tr("Embedded IDE %1").arg(ui->projectView->project()));
    QString name = ui->projectView->projectPath().dirName();
    QString path = ui->projectView->project();
    QSettings sets;
    sets.beginGroup("last_projects");
    sets.setValue(name, path);
    sets.sync();
}

void MainWindow::configureShow()
{
    ConfigDialog(this).exec();
}

void MainWindow::configChanged(AppConfig* config)
{
  this->setUpProxy();
  if (config && config->projectTmplatesAutoUpdate()) {
    this->checkForUpdates();
  }
}

void MainWindow::loggerOpenPath(const QString& path, int col, int row)
{
    QString file = ui->projectView->projectPath().absoluteFilePath(path);
    qDebug() << "Opening" << file << row << col;
    ui->centralWidget->fileOpenAt(file, row - 1, col, &ui->projectView->makeInfo());
}

void MainWindow::checkForUpdates() {
    templateDownloader->setSilent(true);
    templateDownloader->requestPendantDownloads();
}

bool MainWindow::goToBuildStage() {
  static const QString saveBeforeBuildPrompt = "behavior/savebeforebuildprompt";
  if (ui->centralWidget->hasUnsavedChanges()) {
    bool promptEnabled =
        QSettings().value(saveBeforeBuildPrompt, true).toBool();
    if (promptEnabled) {
      QMessageBox msgbox(
          QMessageBox::Icon::Question, tr("Save files"),
          tr("Save all files before build?"),
          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
      QCheckBox cb(tr("Do not show again"));
      msgbox.setCheckBox(&cb);
      msgbox.exec();
      if (msgbox.result() == QMessageBox::StandardButton::Yes ||
          msgbox.result() == QMessageBox::StandardButton::No) {
        QSettings().setValue(saveBeforeBuildPrompt,
                             !msgbox.checkBox()->isChecked());
        if (msgbox.checkBox()->isChecked()) {
          this->statusBar()->showMessage(tr("This dialog not will show again"),
                                         2000);
          ConfigDialog::setEditorSaveOnAction(
                msgbox.result() == QMessageBox::StandardButton::Yes);
        }
        if (msgbox.result() == QMessageBox::StandardButton::Yes) {
          ui->centralWidget->saveAll();
        }
      } else {
        return false;
      }
    } else {
      if (AppConfig::mutableInstance().editorSaveOnAction()) {
        ui->centralWidget->saveAll();
      }
    }
  }
  return true;
}

void MainWindow::setUpProxy() {
  AppConfig &config = AppConfig::mutableInstance();
  switch (config.networkProxyType()) {
    case AppConfig::NetworkProxyType::None:
      QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
      break;
    case AppConfig::NetworkProxyType::System:
      QNetworkProxyFactory::setUseSystemConfiguration(true);
      break;
    case AppConfig::NetworkProxyType::Custom:
      QNetworkProxy proxy(
          QNetworkProxy::HttpProxy, config.networkProxyHost(),
          static_cast<quint16>(config.networkProxyPort().toInt()));
      if (config.networkProxyUseCredentials()) {
        proxy.setUser(config.networkProxyUsername());
        proxy.setPassword(config.networkProxyPassword());
      }
      QNetworkProxy::setApplicationProxy(proxy);
      break;
  }
}

void MainWindow::on_projectView_openFindDialog()
{
    auto d = new FindInFilesDialog(ui->centralWidget, ui->projectView, this);
    d->show();
    connect(d, &QDialog::finished, d, &QObject::deleteLater);
}
