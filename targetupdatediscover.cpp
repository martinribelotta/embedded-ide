#include "targetupdatediscover.h"

#include <QProcess>
#include <QFileInfo>
#include <QDir>

TargetUpdateDiscover::TargetUpdateDiscover(QObject *parent, const char *slotName) :
    QObject(parent), proc(new QProcess(this))
{
    connect(proc, SIGNAL(finished(int)), this, SLOT(finish(int)));
    connect(this, SIGNAL(updateFinish(QStringList)), parent, slotName);
}

void TargetUpdateDiscover::start(const QString& project)
{
    QFileInfo f(project);
    if (f.absoluteDir().exists()) {
        proc->setWorkingDirectory(f.absolutePath());
        proc->start("make -prn"); /* make is for p0rn */
    }
}

void TargetUpdateDiscover::finish(int ret)
{
    Q_UNUSED(ret);

    QStringList targets;
    QRegExp re("^(\\w+)\\:");
    QString line;
    while(!(line = proc->readLine()).isEmpty()) {
        if (line.trimmed().startsWith("# "))
            proc->readLine();
        else if (re.indexIn(line) == 0)
            targets += re.cap(1).trimmed().split(' ');
    }
    if (!targets.isEmpty())
        emit updateFinish(targets);
    deleteLater();
}

