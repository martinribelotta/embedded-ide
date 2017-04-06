#ifndef FINDINFILESDIALOG_H
#define FINDINFILESDIALOG_H

#include <QDialog>
#include <QFuture>

class ProjectView;
class DocumentArea;

namespace Ui {
class FindInFilesDialog;
}

class QStandardItemModel;
class QStandardItem;

class FindInFilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindInFilesDialog(DocumentArea *docView, ProjectView *projView, QWidget *parent = 0);
    ~FindInFilesDialog();

private:
    Ui::FindInFilesDialog *ui;
};

#endif // FINDINFILESDIALOG_H
