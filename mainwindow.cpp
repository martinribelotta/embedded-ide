#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "projectnewdialog.h"

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

void MainWindow::on_projectView_fileOpen(const QString &file)
{
    ui->centralWidget->fileOpen(file);
}

void MainWindow::on_actionProjectNew_triggered()
{
    ProjectNewDialog w(this);
    switch(w.exec()) {
    case QDialog::Accepted:
        do {
            QString error = ui->projectView->newTemplateProject(w.projectPath(), w.templateText());
            if (error.isEmpty()) {
                ui->projectView->openProject(w.projectPath() + QDir::separator() + "Makefile");
            } else
                QMessageBox::critical(this, tr("error"), error);
        } while(0);
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
        ui->projectView->makeTemplate(QFileDialog::
                                      getSaveFileName(this,
                                                      tr("Export file"),
                                                      tr("Unknown.template"),
                                                      tr("Tempalte files (*.template);;"
                                                         "Diff files (*.diff);;"
                                                         "All files (*)")
                                                      )
                                      );
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

}
