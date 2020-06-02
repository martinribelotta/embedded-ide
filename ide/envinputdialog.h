#ifndef ENVINPUTDIALOG_H
#define ENVINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class EnvInputDialog;
}

class EnvInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EnvInputDialog(QWidget *parent = nullptr);
    ~EnvInputDialog();

    QString envName() const;
    QString envValue() const;

private:
    Ui::EnvInputDialog *ui;

    void openPathSelect();
    void openVarSelect();
    void menuAction(QAction *a);
};

#endif // ENVINPUTDIALOG_H
