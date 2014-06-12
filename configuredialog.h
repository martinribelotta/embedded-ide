#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

#include "programmsettings.h"

namespace Ui {
class ConfigureDialog;
}

class ConfigureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureDialog(QWidget *parent = 0);
    ~ConfigureDialog();

private slots:

    void on_buttonBox_accepted();

    void on_memLayoutViewRefresh_clicked();

    void on_MemLayoutAddEntry_clicked();

    void on_MemLayoutDeleteEntry_clicked();

private:
    Ui::ConfigureDialog *ui;
};

#endif // DIALOG_H
