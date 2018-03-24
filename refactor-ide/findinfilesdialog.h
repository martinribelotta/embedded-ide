#ifndef FINDINFILESDIALOG_H
#define FINDINFILESDIALOG_H

#include <QDialog>
#include <QFuture>
#include <QEvent>

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
    explicit FindInFilesDialog(const QString& path, QWidget *parent = 0);
    virtual ~FindInFilesDialog();

signals:
    void queryToOpen(const QString& path, int line, int column);

private:
    Ui::FindInFilesDialog *ui;
};

#endif // FINDINFILESDIALOG_H
