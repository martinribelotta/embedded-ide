#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "projectexporter.h"
#include "projectnewdialog.h"
#include "projetfromtemplate.h"
#include "configdialog.h"
#include "aboutdialog.h"
#include "debuginterface.h"
#include "mainmenuwidget.h"

#include <QRegularExpression>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QUrl>

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

#include <QtDebug>

static QMenu *lastProjects(QWidget *parent) {
    QMenu *m = new QMenu(parent);
    QSettings sets;
    sets.beginGroup("last_projects");
    foreach(QString k, sets.allKeys()) {
        QAction *a = m->addAction(k, parent, SLOT(openProject()));
        a->setData(QVariant(sets.value(k)));
    }
    return m;
}

static QFileInfoList lastProjectsList() {
    QSettings sets;
    QFileInfoList list;
    sets.beginGroup("last_projects");
    foreach(QString k, sets.allKeys()) {
        QFileInfo info(sets.value(k).toString());
        list.append(info);
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
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->projectToolbar->hide();
    QWidget *spacer = new QWidget(ui->projectToolbar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->projectToolbar->insertWidget(ui->actionHelp, spacer);
    (qobject_cast<QToolButton*>(ui->projectToolbar->widgetForAction(ui->actionProjectOpen)))->
            setPopupMode(QToolButton::MenuButtonPopup);

    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    ui->dockWidget->setTitleBarWidget(new QWidget(this));
    ui->projectDock->setTitleBarWidget(new QWidget(this));
    ui->actionProjectOpen->setMenu(lastProjects(this));
    connect(ui->projectView, &ProjectView::projectOpened, this, &MainWindow::projectOpened);
    connect(ui->loggerCompiler, &LoggerWidget::openEditorIn, this, &MainWindow::loggerOpenPath);
    connect(ui->projectView, &ProjectView::startBuild, [this](const QString& target) {
        if (QSettings().value("editor/saveOnAction").toBool())
            ui->centralWidget->saveAll();
        QString projectPath = ui->projectView->projectPath().absolutePath();
        QStringList args;
        args << "-f" << ui->projectView->project() << target;
        ui->loggerCompiler->setWorkingDir(projectPath).startProcess("make", args);
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

    QMenu *menu = new QMenu(this);
    QWidgetAction *wa = new QWidgetAction(menu);
    MainMenuWidget *menuWidget = new MainMenuWidget(menu);
    menuWidget->setProjectList(lastProjectsList());
    wa->setDefaultWidget(menuWidget);
    menu->addAction(wa);
    connect(menuWidget, SIGNAL(projectNew()), this, SLOT(on_actionProjectNew_triggered()));
    connect(menuWidget, SIGNAL(projectOpen()), this, SLOT(on_actionProjectOpen_triggered()));
    connect(menuWidget, &MainMenuWidget::projectOpenAs, [this, menu, menuWidget] (const QFileInfo& info) {
        menu->hide();
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
    connect(menuWidget, SIGNAL(projectClose()), this, SLOT(on_actionProjectClose_triggered()));
    connect(menuWidget, SIGNAL(configure()), this, SLOT(on_actionConfigure_triggered()));
    connect(menuWidget, SIGNAL(help()), this, SLOT(on_actionHelp_triggered()));
    connect(menuWidget, SIGNAL(exit()), this, SLOT(close()));

    connect(menuWidget, SIGNAL(projectNew()), menu, SLOT(hide()));
    connect(menuWidget, SIGNAL(projectOpen()), menu, SLOT(hide()));
    connect(menuWidget, SIGNAL(projectClose()), menu, SLOT(hide()));
    connect(menuWidget, SIGNAL(configure()), menu, SLOT(hide()));
    connect(menuWidget, SIGNAL(help()), menu, SLOT(hide()));
    connect(menuWidget, SIGNAL(exit()), menu, SLOT(hide()));
    ui->projectView->setMainMenu(menu);

#ifdef CIAA_IDE
    ui->tabWidget->removeTab(1);
    ui->tabWidget->removeTab(2);
    ui->tabWidget->tabBar()->hide();
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    auto dirtyList = ui->centralWidget->documentsDirty();
    if (!dirtyList.isEmpty()) {
        ui->centralWidget->closeAll();
        if (ui->centralWidget->documentsDirty().isEmpty())
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
    } else if (inf.isExecutable()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(file));
    } else if (m.inherits("text/plain") || (inf.size() == 0)) {
        ui->centralWidget->fileOpenAt(file, 0, 0, &ui->projectView->makeInfo());
    } else {
        // QDesktopServices::openUrl(QUrl::fromLocalFile(file));
        ui->centralWidget->binOpen(file);
    }
}

void MainWindow::on_actionProjectNew_triggered()
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

void MainWindow::on_actionProjectOpen_triggered()
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
            ui->actionProjectOpen->setMenu(lastProjects(this));
        }
    }
}

void MainWindow::projectOpened()
{
    setWindowTitle(tr("Embedded IDE %1").arg(ui->projectView->project()));
}

void MainWindow::on_actionHelp_triggered()
{
    AboutDialog(this).exec();
}

void MainWindow::on_actionProjectExport_triggered()
{
}

void MainWindow::on_actionProjectClose_triggered()
{
    ui->projectView->closeProject();
    ui->centralWidget->closeAll();
    ui->loggerCompiler->clearText();
    setWindowTitle(tr("Embedded IDE"));
}

void MainWindow::on_projectView_projectOpened()
{
    QString name = ui->projectView->projectPath().dirName();
    QString path = ui->projectView->project();
    QSettings sets;
    sets.beginGroup("last_projects");
    sets.setValue(name, path);
    sets.sync();
    ui->actionProjectOpen->setMenu(lastProjects(this));
}

void MainWindow::on_actionSave_All_triggered()
{
    ui->centralWidget->saveAll();
}

void MainWindow::on_actionConfigure_triggered()
{
    ConfigDialog(this).exec();
}

void MainWindow::loggerOpenPath(const QString& path, int col, int row)
{
    QString file = ui->projectView->projectPath().absoluteFilePath(path);
    qDebug() << "Opening" << file << row << col;
    ui->centralWidget->fileOpenAt(file, row, col, &ui->projectView->makeInfo());
}
