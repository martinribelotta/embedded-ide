#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "projectexporter.h"
#include "projectnewdialog.h"
#include "projetfromtemplate.h"
#include "configdialog.h"
#include "aboutdialog.h"
#include "debuginterface.h"

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

#include <QtDebug>

#include <qsvsh/qsvlangdeffactory.h>

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
    auto b = qobject_cast<QToolButton*>(ui->projectToolbar->widgetForAction(ui->actionProjectOpen));
    b->setPopupMode(QToolButton::MenuButtonPopup);
    //QFont logFont = monoFont();
    //logFont.setPointSize(10);
    //ui->textLog->document()->setDefaultFont(logFont);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    ui->dockWidget->setTitleBarWidget(new QWidget(this));
    ui->projectDock->setTitleBarWidget(new QWidget(this));
    ui->actionProjectOpen->setMenu(lastProjects(this));
    connect(ui->projectView, &ProjectView::projectOpened, this, &MainWindow::projectOpened);
    connect(ui->loggerCompiler, &LoggerWidget::openEditorIn, this, &MainWindow::loggerOpenPath);
    connect(ui->projectView, &ProjectView::startBuild, [this](const QString& target) {
        QString projectPath = ui->projectView->projectPath().absolutePath();
        QStringList args;
        args << "-f" << ui->projectView->project() << target;
        ui->loggerCompiler->setWorkingDir(projectPath).startProcess("make", args);
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

    QsvLangDefFactory::getInstanse()->loadDirectory(":/qsvsh/qtsourceview/data/langs");
    QsvLangDefFactory::getInstanse()->addMimeTypes(":/qsvsh/qtsourceview/data/mime.types");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
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
    QMimeType m = QMimeDatabase().mimeTypeForFile(file, QMimeDatabase::MatchDefault);
    if (m.inherits("text/plain"))
        ui->centralWidget->fileOpenAt(file, 0, 0, &ui->projectView->makeInfo());
    else {
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
    // TODO: Implement me
}

void MainWindow::on_actionHelp_triggered()
{
    AboutDialog(this).exec();
    //QMessageBox::about(this, tr("About IDE"), resourceText(":/help/about.txt"));
}

void MainWindow::on_actionProjectExport_triggered()
{
    if (!ui->projectView->project().isEmpty())
        (new ProjectExporter(
                QFileDialog::
                getSaveFileName(this,
                                tr("Export file"),
                                tr("Unknown.template"),
                                tr("Tempalte files (*.template);;"
                                   "Diff files (*.diff);;"
                                   "All files (*)")
                                ),
                QFileInfo(ui->projectView->project()).absolutePath(),
                this,
                SLOT(actionExportFinish(QString)))
            )->start();
}

void MainWindow::on_actionProjectClose_triggered()
{
    ui->projectView->closeProject();
    ui->centralWidget->closeAll();
    ui->loggerCompiler->clearText();
}

#if 0
void MainWindow::on_buildStop_clicked()
{
    ui->projectView->buildStop();
}

static QString consoleToHtml(const QString& s) {
    return consoleMarkErrorT1(QString(s)
            .replace("\t", "&nbsp;")
            .replace(" ", "&nbsp;"))
            .replace("\r\n", "<br>")
            .replace("\n", "<br>");
}

void MainWindow::on_projectView_buildStdout(const QString &text)
{
    QTextCursor c = ui->textLog->textCursor();
    c.movePosition(QTextCursor::End);
    c.insertHtml(QString("<span style=\"color: green\">%1</span>").arg(consoleToHtml(text)));
    ui->textLog->setTextCursor(c);
}

void MainWindow::on_projectView_buildStderr(const QString &text)
{
    QTextCursor c = ui->textLog->textCursor();
    c.movePosition(QTextCursor::End);
    c.insertHtml(QString("<span style=\"color: red\">%1</span>").arg(consoleToHtml(text)));
    ui->textLog->setTextCursor(c);
}

void MainWindow::on_projectView_buildEnd(int status)
{
    // ui->buildStop->setEnabled(false);
    Q_UNUSED(status);
}
#endif

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
