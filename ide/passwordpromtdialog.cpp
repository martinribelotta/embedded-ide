#include "passwordpromtdialog.h"

PasswordPromtDialog::PasswordPromtDialog(QString title, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::PasswordPromtDialog)
{
  ui->setupUi(this);
  if (!title.isEmpty()) {
    this->setWindowTitle(title);
  }
}

QString PasswordPromtDialog::password()
{
    return ui->lineEdit->text();
}
