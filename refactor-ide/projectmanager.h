#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QVariant>

class QListView;

class ProcessManager;

class ProjectManager : public QObject
{
    Q_OBJECT
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

public slots:
    void openProject(const QString& makefile);
    void closeProject();
    void reloadProject();

private:
    class Priv_t;
    Priv_t *priv;
};

#endif // PROJECTMANAGER_H
