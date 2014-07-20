#ifndef TARGETUPDATEDISCOVER_H
#define TARGETUPDATEDISCOVER_H

#include <QObject>

class QProcess;

class TargetUpdateDiscover : public QObject
{
    Q_OBJECT
public:
    explicit TargetUpdateDiscover(QObject *parent, const char *slotName);

signals:
    void updateFinish(const QStringList& targets);

public slots:
    void start(const QString &project);

private slots:
    void finish(int ret);

private:
    QProcess *proc;
    QString projectName;
};

#endif // TARGETUPDATEDISCOVER_H
