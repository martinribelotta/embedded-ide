#ifndef PROJECTNEWDIALOG_H
#define PROJECTNEWDIALOG_H

#include <QDialog>

namespace Ui {
class ProjectNewDialog;
}

class ProjectNewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectNewDialog(QWidget *parent = 0);
    ~ProjectNewDialog();

    QString projectPath() const;
    QString templateText() const;
private slots:
    void refreshProjectName();

    void on_toolFindProjectPath_clicked();

    void on_toolLoadTemplate_clicked();

private:
    Ui::ProjectNewDialog *ui;
};

#endif // PROJECTNEWDIALOG_H
