#ifndef FILESYSTEMMANAGER_H
#define FILESYSTEMMANAGER_H

#include <QObject>

class QTreeView;

class FilesystemManager : public QObject
{
    Q_OBJECT
public:
    explicit FilesystemManager(QTreeView *v, QObject *parent = nullptr);

signals:
    void requestFileOpen(const QString& path);

public slots:
    void openPath(const QString& path);
    void closePath();

private:
    QTreeView *view;
};

#endif // FILESYSTEMMANAGER_H
