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
    QStringList list;
    QRegularExpression re(reText, QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while(it.hasNext()) {
        QRegularExpressionMatch me = it.next();
        //qDebug() << text.mid(qMax(0, me.capturedStart()-10), qMin(text.length(), me.capturedLength() + 100));
        list.append(me.captured(1));
    }
    return list;
}

static QStringList unique(const QStringList& list) {
    return QSet<QString>::fromList(list).toList();
}

static const QString TARGETS_RE = "(?<!^# Not a target\\:\\n)^([a-zA-Z0-9][^$#\\\\\\/\\t=\\.]*):(?:[^=]|$)";
static const QString DEFINES_RE = "\\-D(\\S*(\\=\\\"(?:[^\"\\\\]|\\\\.)*\\\")*)";
static const QString INCLUDES_RE = "\\-I(\\S*(\\=\\\"(?:[^\"\\\\]|\\\\.)*\\\")*)";

void TargetUpdateDiscover::finish(int ret)
{
    Q_UNUSED(ret);
    MakefileInfo info;
    QString text = proc->readAllStandardOutput();
    info.targets = unique(parseRe(text, TARGETS_RE));
    info.defines = unique(parseRe(text, DEFINES_RE));
    info.include = unique(parseRe(text, INCLUDES_RE));
    emit updateFinish(info);
    // deleteLater();
}

