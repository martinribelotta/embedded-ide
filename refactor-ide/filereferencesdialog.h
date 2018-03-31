#ifndef FILEREFERENCESDIALOG_H
#define FILEREFERENCESDIALOG_H

#include <QDialog>
#include "icodemodelprovider.h"

namespace Ui {
class FileReferencesDialog;
}

class FileReferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileReferencesDialog(const ICodeModelProvider::FileReferenceList& refList, QWidget *parent = 0);
    ~FileReferencesDialog();

signals:
    void itemClicked(const QString& path, int line);

private:
    Ui::FileReferencesDialog *ui;
};

#endif // FILEREFERENCESDIALOG_H
