#include "projectnewdialog.h"
#include "ui_projectnewdialog.h"

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QPushButton>

#include <QtDebug>

ProjectNewDialog::ProjectNewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectNewDialog)
{
    ui->setupUi(this);
    refreshProjectName();
}

ProjectNewDialog::~ProjectNewDialog()
{
    delete ui;
}

QString ProjectNewDialog::projectPath() const
{
    return ui->projectFileText->text();
}

QString ProjectNewDialog::templateText() const
{
    QFile f(ui->templateFile->lineEdit()->text());
    return f.open(QFile::ReadOnly)? f.readAll() : QString();
}

void ProjectNewDialog::refreshProjectName()
{
    ui->projectFileText->setText(QString("%1%2%3")
                                 .arg(ui->projectPath->text())
                                 .arg(ui->projectPath->text().isEmpty()? "" : QString(QDir::separator()))
                                 .arg(ui->projectName->text()));
    bool en = QFileInfo(ui->templateFile->lineEdit()->text()).exists() &&
            QFileInfo(ui->projectPath->text()).exists() &&
            !ui->projectName->text().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(en);
}
