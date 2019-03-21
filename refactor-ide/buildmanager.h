#ifndef BUILDMANAGER_H
#define BUILDMANAGER_H

#include <QObject>

class ProcessManager;
class ProjectManager;

class BuildManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildManager)
public:
    static const QString PROCESS_NAME;

    explicit BuildManager(ProjectManager *_proj, ProcessManager *_pman, QObject *parent = nullptr);

signals:
    void buildStarted(const QString& target);
    void buildTerminated(int code, const QString& error);

public slots:
    void startBuild(const QString& target);

private:
    ProjectManager *proj;
    ProcessManager *pman;
};

#endif // BUILDMANAGER_H
