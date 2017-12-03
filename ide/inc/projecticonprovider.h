#ifndef PROJECTICONPROVIDER_H
#define PROJECTICONPROVIDER_H

#include <QObject>
#include <QFileIconProvider>

class QMimeDatabase;

class ProjectIconProvider : public QObject, public QFileIconProvider
{
public:
    ProjectIconProvider(QObject *parent = 0l);
    virtual ~ProjectIconProvider();

    virtual QIcon icon(const QFileInfo &info) const;

private:
    QMimeDatabase *db;
};

#endif // PROJECTICONPROVIDER_H
