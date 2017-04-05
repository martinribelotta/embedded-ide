#include "targetupdatediscover.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QtConcurrent>

#include <QtDebug>

TargetUpdateDiscover::TargetUpdateDiscover(QObject *parent) :
    QObject(parent), proc(new QProcess(this))
{
    connect(proc, SIGNAL(finished(int)), this, SLOT(finish(int)));
}

void TargetUpdateDiscover::start(const QString& project)
{
    QFileInfo f(project);
    if (f.absoluteDir().exists()) {
        QProcessEnvironment env = proc->processEnvironment();
        env.insert("LANG", "C");
        proc->setProcessEnvironment(env);
        proc->setWorkingDirectory(f.absolutePath());
        proc->start("make", QStringList()
                    << "-B"
                    << "-p"
                    << "-r"
                    << "-n"
                    << "-f"
                    << f.fileName());
    }
}

static QHash<QString, QString> findAllTargets(const QString& text) {
    QHash<QString, QString> map;
    QRegularExpression re(R"(^([a-zA-Z0-9 \t\\\/_\.\:\-]*?):(?!=)\s*([^#\r\n]*?)\s*$)",
                          QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while(it.hasNext()) {
        QRegularExpressionMatch me = it.next();
        map.insert(me.captured(1), me.captured(2));
    }
    return map;
}

static QStringList filterTargetHeuristic(const QStringList& all)
{
    QStringList targets;
    QRegularExpression re("^[a-zA-Z0-9_]+$");
    foreach(QString target, all) {
        if (target == QString("Makefile"))
            continue;
        QRegularExpressionMatch m = re.match(target);
        if (m.hasMatch())
            targets.append(target);
    }
    return targets;
}

void TargetUpdateDiscover::finish(int ret)
{
    Q_UNUSED(ret);
    MakefileInfo info;
    QString text = proc->readAllStandardOutput();
    info.workingDir = proc->workingDirectory();
    info.allTargets = findAllTargets(text);
    auto it = info.allTargets.cbegin();
    while (it != info.allTargets.cend()) {
        // qDebug() << it.key() << ':' << it.value();
        ++it;
    }
    info.targets = filterTargetHeuristic(info.allTargets.keys());
    emit updateFinish(info);
}

