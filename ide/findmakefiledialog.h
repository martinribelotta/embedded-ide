#ifndef FINDMAKEFILEDIALOG_H
#define FINDMAKEFILEDIALOG_H

#include <QDialog>

namespace Ui {
class FindMakefileDialog;
}

class FindMakefileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindMakefileDialog(const QString& path, QWidget *parent = nullptr);
    ~FindMakefileDialog();

    QString fileName() const;

private:
    Ui::FindMakefileDialog *ui;

    QStringList findInPath(const QString& path);
};

#endif // FINDMAKEFILEDIALOG_H
