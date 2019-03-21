#ifndef PASSWORDPROMTDIALOG_H
#define PASSWORDPROMTDIALOG_H

#include "ui_passwordpromtdialog.h"
#include <QString>

class PasswordPromtDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit PasswordPromtDialog(QString title = "", QWidget *parent = 0);
    QString password();

  private:
    Ui::PasswordPromtDialog *ui;
};

#endif // PASSWORDPROMTDIALOG_H
