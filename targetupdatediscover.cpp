#include "targetupdatediscover.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QProcess>
#include <QFileInfo>
#include <QDir>

#include <QtDebug>

TargetUpdateDiscover::TargetUpdateDiscover(QObject *parent) :
    QObject(parent), proc(new QProcess(this))
{
    connect(proc, SIGNAL(finished(int)), this, SLOT(finish(int)));
    // connect(this, SIGNAL(updateFinish(QStringList)), parent, slotName);
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

static QStringList parseRe(const QString& text, const QString& reText) {
    QSet<QString> set;
    QRegularExpression re(reText, QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while(it.hasNext()) {
        QRegularExpressionMatch me = it.next();
        //qDebug() << text.mid(qMax(0, me.capturedStart()-10), qMin(text.length(), me.capturedLength() + 100));
        set.insert(me.captured(1));
    }
    return set.toList();
}

static const QString TARGETS_RE = "(?<!^# Not a target\\:\\n)^([a-zA-Z0-9][^$#\\\\\\/\\t=\\.]*):(?:[^=]|$)";
//static const QString DEFINES_RE = "\\-D(\\S*(\\=\\\"(?:[^\"\\\\]|\\\\.)*\\\")*)";
//static const QString INCLUDES_RE = "\\-I(\\S*(\\=\\\"(?:[^\"\\\\]|\\\\.)*\\\")*)";
static const QString DEFINES_RE = "\\-D([^\\s\\$]+)";
static const QString INCLUDES_RE = "\\-I([^\\s\\$]+)";

void TargetUpdateDiscover::finish(int ret)
{
    Q_UNUSED(ret);
    MakefileInfo info;
    QString text = proc->readAllStandardOutput();
    info.targets = parseRe(text, TARGETS_RE);
    info.defines = parseRe(text, DEFINES_RE);
    info.include = parseRe(text, INCLUDES_RE);
    qDebug() << info.targets;
    qDebug() << info.defines;
    qDebug() << info.include;
    info.workingDir = proc->workingDirectory();
    emit updateFinish(info);
    // deleteLater();
}

