#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setWindowTitle(QCoreApplication::applicationName());
    setFixedSize(sizeHint());
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
