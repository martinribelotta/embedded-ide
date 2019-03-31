#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include <QDialog>
#include <QFileInfo>

namespace Ui {
class NewProjectDialog;
}

class NewProjectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewProjectDialog(QWidget *parent = nullptr);
    virtual ~NewProjectDialog();

    QString absoluteProjectPath() const;
    QString templateFile() const;
    QFileInfo selectedTemplateFile() const;
    bool isTemplate() const;
private:
    Ui::NewProjectDialog *ui;
};

#endif // NEWPROJECTDIALOG_H
