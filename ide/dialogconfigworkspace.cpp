#include "dialogconfigworkspace.h"
#include "ui_dialogconfigworkspace.h"

#include "configdialog.h"
#include "appconfig.h"

#include <QFileDialog>

DialogConfigWorkspace::DialogConfigWorkspace(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConfigWorkspace)
{
    ui->setupUi(this);
    ui->workspaceEditor->setText(
          AppConfig::mutableInstance().defaultApplicationResources());
}

DialogConfigWorkspace::~DialogConfigWorkspace()
{
    delete ui;
}

QString DialogConfigWorkspace::path() const
{
    return ui->workspaceEditor->text();
}

void DialogConfigWorkspace::on_toolSelectPath_clicked()
{
    QFileDialog d(this);
    d.setWindowTitle(tr("Select workspace PATH"));
    d.setAcceptMode(QFileDialog::AcceptSave);
    d.setConfirmOverwrite(true);
    d.setDefaultSuffix("");
    d.setDirectory(QDir::home());
    d.setFileMode(QFileDialog::AnyFile);
    d.setOption(QFileDialog::ShowDirsOnly);
    d.setNameFilters(QStringList{ tr("All directories (*)") });
    d.setViewMode(QFileDialog::Detail);
    d.setLabelText(QFileDialog::LookIn, tr("Look in"));
    d.setLabelText(QFileDialog::FileName, tr("Directory"));
    d.setLabelText(QFileDialog::FileType, tr("File Type"));
    d.setLabelText(QFileDialog::Accept, tr("Select file"));
    d.setLabelText(QFileDialog::Reject, tr("Cancel"));
    if (d.exec() == QDialog::Accepted) {
        QStringList sel = d.selectedFiles();
        if (sel.count() > 0) {
            ui->workspaceEditor->setText(sel.first());
        }
    }
}

void DialogConfigWorkspace::on_buttonBox_accepted()
{
  QDir wSpace;
  wSpace.setPath(this->path());
  AppConfig::mutableInstance().setWorkspacePath(wSpace.absolutePath());
  AppConfig::mutableInstance().save();
}
