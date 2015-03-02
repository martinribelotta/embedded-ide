#include "targetupdatediscover.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QProcess>
#include <QFileInfo>
#include <QDir>

#include <QtDebug>

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
    QRegularExpression re("^(\\w+)\\:");
    re.setPatternOptions(QRegularExpression::MultilineOption);
    QString line;
    while(!(line = proc->readLine()).isEmpty()) {
        if (line.trimmed().startsWith("#")) {
            proc->readLine();
        } else {
            QRegularExpressionMatch m = re.match(line);
            if (m.hasMatch()) {
                qDebug() << "Found" << line;
                QString tgt = m.captured(1).trimmed();
                if (QFileInfo(tgt).exists())
                    qDebug() << "File" << tgt << "skip";
                else
                    targets.append(tgt);
            }
        }
    }
    if (!targets.isEmpty())
        emit updateFinish(targets);
    deleteLater();
}

