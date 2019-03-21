#ifndef FILESYSTEMMANAGER_H
#define FILESYSTEMMANAGER_H

#include <QFileInfo>
#include <QObject>

class QTreeView;

class FileSystemManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileSystemManager)

public:
    explicit FileSystemManager(QTreeView *v, QObject *parent = nullptr);
    virtual ~FileSystemManager();

    static QIcon iconForFile(const QFileInfo &info);

signals:
    void requestFileOpen(const QString& path);

public slots:
    void openPath(const QString& path);
    void closePath();

private slots:
    void customContextMenu(const QPoint& pos);

    void menuNewFile();
    void menuNewDirectory();
    void menuNewSymlink();
    void menuItemProperties();
    void menuItemExecute();
    void menuItemOpenExternal();
    void menuItemRename();
    void menuItemDelete();

private:
    QTreeView *view;
};

#endif // FILESYSTEMMANAGER_H
