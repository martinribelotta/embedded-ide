#ifndef TARGETUPDATEDISCOVER_H
#define TARGETUPDATEDISCOVER_H

#include <QObject>
#include <QStringList>

#include "makefileinfo.h"

class QProcess;

class TargetUpdateDiscover : public QObject
{
    Q_OBJECT
public:
    explicit TargetUpdateDiscover(QObject *parent);

signals:
    void updateFinish(const MakefileInfo& targets);

public slots:
    void start(const QString &project);

private slots:
    void finish(int ret);

private:
    QProcess *proc;
    QString projectName;
};

#endif // TARGETUPDATEDISCOVER_H
