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
    explicit FindInFilesDialog(DocumentArea *docView, ProjectView *projView, QWidget *parent = 0);
    ~FindInFilesDialog();

protected:
    bool event(QEvent *event) override
    {
        switch (event->type()) {
        case QEvent::WindowActivate:
            setWindowOpacity(1.0);
            break;
        case QEvent::WindowDeactivate:
            setWindowOpacity(0.5);
            break;
        }
        return QDialog::event(event);
    }
private:
    Ui::FindInFilesDialog *ui;
};

#endif // FINDINFILESDIALOG_H
