#ifndef FINDANDOPENFILEDIALOG_H
#define FINDANDOPENFILEDIALOG_H

#include <QDialog>

namespace Ui {
class FindAndOpenFileDialog;
}

class FindAndOpenFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindAndOpenFileDialog(QWidget *parent = nullptr);
    ~FindAndOpenFileDialog();

    void setFileList(const QString &prefix, const QStringList &list);
    QString selectedFile() const;

    static QStringList findFilesInPath(const QString& file, const QString& path);

private:
    Ui::FindAndOpenFileDialog *ui;
};

#endif // FINDANDOPENFILEDIALOG_H
