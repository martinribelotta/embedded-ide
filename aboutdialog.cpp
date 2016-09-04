#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "version.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->label_VerBuildDate->setText(tr("Version %1, build date %2")
                                    .arg(VERSION)
                                    .arg(BUILD_DATE));
    setWindowTitle(QCoreApplication::applicationName());
    setFixedSize(sizeHint());
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
