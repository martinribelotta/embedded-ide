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

    void on_templateFile_editTextChanged(const QString &fileName);

private:
    Ui::ProjectNewDialog *ui;

    QString replaceTemplates(QString text) const;
};

#endif // PROJECTNEWDIALOG_H
