#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "projectexporter.h"
#include "projectnewdialog.h"
#include "projetfromtemplate.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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

void MainWindow::on_projectView_fileOpen(const QString &file)
{
    ui->centralWidget->fileOpen(file);
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

static QString resourceText(const QString& res) {
    QFile f(res);
    f.open(QFile::ReadOnly);
    return f.readAll();
}

void MainWindow::on_actionHelp_triggered()
{
    QMessageBox::about(this, tr("About IDE"), resourceText(":/help/about.txt"));
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

void MainWindow::on_projectView_buildStdout(const QString &text)
{
    QTextCursor c = ui->textLog->textCursor();
    QTextCharFormat fmt = c.charFormat();
    fmt.setForeground(Qt::blue);
    c.movePosition(QTextCursor::End);
    ui->textLog->setTextCursor(c);
    ui->textLog->setCurrentCharFormat(fmt);
    ui->textLog->insertPlainText(text);
}

void MainWindow::on_projectView_buildStderr(const QString &text)
{
    QTextCursor c = ui->textLog->textCursor();
    QTextCharFormat fmt = c.charFormat();
    fmt.setForeground(Qt::red);
    c.movePosition(QTextCursor::End);
    ui->textLog->setTextCursor(c);
    ui->textLog->setCurrentCharFormat(fmt);
    ui->textLog->insertPlainText(text);
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
