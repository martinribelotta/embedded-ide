#ifndef FILEPROPERTIESDIALOG_H
#define FILEPROPERTIESDIALOG_H

#include <QDialog>
#include <QFileInfo>

namespace Ui {
class FilePropertiesDialog;
}

class FilePropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilePropertiesDialog(const QFileInfo& info, QWidget *parent = 0);
    ~FilePropertiesDialog();

private slots:
    void on_FilePropertiesDialog_accepted();

private:
    Ui::FilePropertiesDialog *ui;
    QFileInfo thisFile;
};

#endif // FILEPROPERTIESDIALOG_H
