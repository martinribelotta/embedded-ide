#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "projectexporter.h"
#include "projectnewdialog.h"
#include "projetfromtemplate.h"
#include "configdialog.h"
#include "aboutdialog.h"

#include <QRegularExpression>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QUrl>
#include <QUrlQuery>

#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    ui->dockWidget->setTitleBarWidget(new QWidget(this));
    ui->projectDock->setTitleBarWidget(new QWidget(this));
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
    Q_UNUSED(s);
}

void MainWindow::on_projectView_fileOpen(const QString &file)
{
    ui->centralWidget->fileOpen(file, 0, 0, &ui->projectView->makeInfo());
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
    ui->projectView->openProject(QFileDialog::
                                 getOpenFileName(this,
                                                 tr("Open Project"),
                                                 "Makefile",
                                                 tr("Makefile (Makefile);;"
                                                    "Make (*.mk);;"
                                                    "All Files (*)")
                                                 )
                                 );
}

#if 0
static QString resourceText(const QString& res) {
    QFile f(res);
    f.open(QFile::ReadOnly);
    return f.readAll();
}
#endif

void MainWindow::on_actionHelp_triggered()
{
    AboutDialog(this).exec();
    //QMessageBox::about(this, tr("About IDE"), resourceText(":/help/about.txt"));
}

void MainWindow::on_actionProjectExport_triggered()
{
    if (!ui->projectView->project().isEmpty())
#if 1
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
#else
        ui->projectView->makeTemplate(QFileDialog::
                                      getSaveFileName(this,
                                                      tr("Export file"),
                                                      tr("Unknown.template"),
                                                      tr("Tempalte files (*.template);;"
                                                         "Diff files (*.diff);;"
                                                         "All files (*)")
                                                      )
                                      );
#endif
}

void MainWindow::on_projectView_startBuild(const QString &target)
{
    ui->textLog->clear();
    ui->projectView->buildStart(target); // Ok! bad back signal!!!
    ui->buildStop->setEnabled(true);
}

void MainWindow::on_actionProjectClose_triggered()
{
    ui->projectView->closeProject();
}

void MainWindow::on_buildStop_clicked()
{
    ui->projectView->buildStop();
}

static QString mkUrl(const QString& p, const QString& x, const QString& y) {
    return QString("file:%1?x=%2&y=%3").arg(p).arg(x).arg(y);
}


static QString errorLink(const QString& s) {
    QString str(s);
    QRegularExpression re("^(.+)\\:(\\d+)\\:(\\d+)\\: \\w+\\: .+$");
    re.setPatternOptions(QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = re.globalMatch(s);
    while(it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        QString text = m.captured(0);
        QString path = m.captured(1);
        QString line = m.captured(2);
        QString col = m.captured(3);
        QString url = mkUrl(path, line, col);
        str.replace(text, QString("<a href=\"%1\">%2</a>").arg(url).arg(text));
    }
    return str;
}

void MainWindow::on_projectView_buildStdout(const QString &text)
{
#if 1
    ui->textLog->append(QString("<font color=\"green\">%1</font>")
                            .arg(text)); //.replace(QRegExp("[\\r\\n]"), "<br>"));
    QTextCursor c = ui->textLog->textCursor();
    c.movePosition(QTextCursor::End);
    ui->textLog->setTextCursor(c);
#else
    QTextCursor c = ui->textLog->textCursor();
    QTextCharFormat fmt = c.charFormat();
    fmt.setForeground(Qt::blue);
    c.movePosition(QTextCursor::End);
    ui->textLog->setTextCursor(c);
    ui->textLog->setCurrentCharFormat(fmt);
    ui->textLog->insertPlainText(text);
#endif
}

void MainWindow::on_projectView_buildStderr(const QString &text)
{
#if 1
    ui->textLog->append(QString("<font color=\"red\">%1</font>")
                            .arg(errorLink(text))); //.replace(QRegExp("[\\r\\n]"), "<br>"));
    QTextCursor c = ui->textLog->textCursor();
    c.movePosition(QTextCursor::End);
    ui->textLog->setTextCursor(c);
#else
    QTextCursor c = ui->textLog->textCursor();
    QTextCharFormat fmt = c.charFormat();
    fmt.setForeground(Qt::red);
    c.movePosition(QTextCursor::End);
    ui->textLog->setTextCursor(c);
    ui->textLog->setCurrentCharFormat(fmt);
    ui->textLog->insertPlainText(text);
#endif
}

void MainWindow::on_projectView_buildEnd(int status)
{
    ui->buildStop->setEnabled(false);
    Q_UNUSED(status);
}

void MainWindow::on_actionSave_All_triggered()
{
    ui->centralWidget->saveAll();
}

void MainWindow::on_actionDocumentNew_triggered()
{
    QString projectPath = QFileInfo(ui->projectView->project()).absolutePath();
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("New file"),
                                                    projectPath,
                                                    tr("C file (*.c);;"
                                                       "C++ file (*.cpp);;"
                                                       "Header (*.h);;"
                                                       "All files (*)"));
    if (!fileName.isEmpty()) {
        QFile f(fileName);
        if (!f.open(QFile::WriteOnly)) {
            QMessageBox::critical(this, tr("Error creating file"), f.errorString());
        } else {
            f.close();
            ui->centralWidget->fileOpen(fileName);
        }
    }
}

void MainWindow::on_actionConfigure_triggered()
{
    ConfigDialog(this).exec();
}

void MainWindow::on_textLog_anchorClicked(const QUrl &url)
{
    QUrlQuery q(url.query());
    int row = q.queryItemValue("x").toInt();
    int col = q.queryItemValue("y").toInt();
    QString file = ui->projectView->projectPath().absoluteFilePath(url.toLocalFile());
    // qDebug() << "Opening" << file << row << col;
    ui->centralWidget->fileOpen(file, row, col);
}
