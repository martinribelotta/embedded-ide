#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QVariant>
#include <QFile>

class QListView;

class ProcessManager;

class ProjectManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProjectManager)
public:
    explicit ProjectManager(QListView *view, ProcessManager *pman, QObject *parent = nullptr);
    virtual ~ProjectManager();

    QString projectName() const;
    QString projectPath() const;
    QString projectFile() const;
    bool isProjectOpen() const;

signals:
    void projectOpened(const QString& makePath);
    void projectClosed();
    void targetTriggered(const QString& target);
    void requestFileOpen(const QString& path);
    void exportFinish(const QString& exportMessage);

public slots:
    void createProject(const QString& projectFilePath, const QString& templateFile);
    void exportCurrentProjectTo(const QString& patchFile);
    void openProject(const QString& makefile);
    void closeProject();
    void reloadProject();

private:
    class Priv_t;
    Priv_t *priv;
};

#endif // PROJECTMANAGER_H
