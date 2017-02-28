#ifndef DIALOGCONFIGWORKSPACE_H
#define DIALOGCONFIGWORKSPACE_H

#include <QDialog>

namespace Ui {
class DialogConfigWorkspace;
}

class DialogConfigWorkspace : public QDialog
{
    Q_OBJECT

public:
    explicit DialogConfigWorkspace(QWidget *parent = 0);
    ~DialogConfigWorkspace();

    QString path() const;

private slots:
    void on_toolSelectPath_clicked();

private:
    Ui::DialogConfigWorkspace *ui;
};

#endif // DIALOGCONFIGWORKSPACE_H
