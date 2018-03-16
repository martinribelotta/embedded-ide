#ifndef EXTERNALTOOLMANAGER_H
#define EXTERNALTOOLMANAGER_H

#include <QDialog>

namespace Ui {
class ExternalToolManager;
}

class QMenu;

class ProcessManager;

class ExternalToolManager : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(ExternalToolManager)

public:
    explicit ExternalToolManager(QWidget *parent = 0);
    virtual ~ExternalToolManager();

    static QMenu *makeMenu(QWidget *parent, ProcessManager *pman);

private:
    Ui::ExternalToolManager *ui;
};

#endif // EXTERNALTOOLMANAGER_H
