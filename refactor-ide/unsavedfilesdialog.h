#ifndef UNSAVEDFILESDIALOG_H
#define UNSAVEDFILESDIALOG_H

#include <QDialog>

namespace Ui {
class UnsavedFilesDialog;
}

class UnsavedFilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UnsavedFilesDialog(const QStringList &unsaved, QWidget *parent = 0);
    virtual ~UnsavedFilesDialog();

    QStringList checkedForSave() const;

private:
    Ui::UnsavedFilesDialog *ui;
};

#endif // UNSAVEDFILESDIALOG_H
