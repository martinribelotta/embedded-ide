#ifndef EXTERNALTOOLMANAGER_H
#define EXTERNALTOOLMANAGER_H

#include <QDialog>

namespace Ui {
class ExternalToolManager;
}

class QMenu;

class ProcessManager;
class ProjectManager;

class ExternalToolManager : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(ExternalToolManager)

public:
    explicit ExternalToolManager(QWidget *parent = nullptr);
    virtual ~ExternalToolManager();

    static QMenu *makeMenu(QWidget *parent, ProcessManager *pman, ProjectManager *proj);

private:
    Ui::ExternalToolManager *ui;
};

#endif // EXTERNALTOOLMANAGER_H
