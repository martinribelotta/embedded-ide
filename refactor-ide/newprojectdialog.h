#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include <QDialog>

namespace Ui {
class NewProjectDialog;
}

class NewProjectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewProjectDialog(QWidget *parent = 0);
    ~NewProjectDialog();

    QString absoluteProjectPath() const;
    QString templateFile() const;
private:
    Ui::NewProjectDialog *ui;
};

#endif // NEWPROJECTDIALOG_H
