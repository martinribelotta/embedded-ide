#ifndef FINDINFILESDIALOG_H
#define FINDINFILESDIALOG_H

#include <QDialog>
#include <QFuture>

class ProjectView;

namespace Ui {
class FindInFilesDialog;
}

class FindInFilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindInFilesDialog(ProjectView *view, QWidget *parent = 0);
    ~FindInFilesDialog();

private slots:
    void on_buttonFind_clicked();

private:
    Ui::FindInFilesDialog *ui;
    ProjectView *projectView;
    QFuture<void> future;

    void setStatus(const QString& message);
};

#endif // FINDINFILESDIALOG_H
