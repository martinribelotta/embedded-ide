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

#if 0
static const QString TARGETS_RE = R"((?<!^# Not a target\:\n)^([a-zA-Z0-9][^$#\\\/\t=\.]*):(?:[^=]|$))";
static const QString DEFINES_RE = R"(\-D([^\s\$]+))";
static const QString INCLUDES_RE = R"(\-I([^\s\$]+))";
static const QString CC_RE = R"(^\s*([a-zA-Z0-9\-]*gcc\b))";
static const QString CC_CFLAGS_RE = R"(\b([a-zA-Z0-9\-]*gcc.*?-c.*?$))";
static const QString C_CXX_FILE_RE = R"(\S+\.cp?p?)";
static const QString O_FILE_RE = R"(\-o\s*\S+\.o)";
static const QString CC_OUT_INCLUDE_RE = R"(\> search starts here\:(.*?)End of search list)";
static const QString CC_OUT_DEFINES_RE = R"(\#define ([_a-zA-Z0-9\(\,\)]+) (.*?)$)";

static void parseCCOut(const QString& text, QStringList *incs, QStringList *defs) {
    QRegularExpressionMatch m = QRegularExpression(
                CC_OUT_INCLUDE_RE,
                QRegularExpression::DotMatchesEverythingOption|
                QRegularExpression::MultilineOption)
            .match(text);
    if (m.hasMatch()) {
        QString str = m.captured(1);
        QSet<QString> includes;
        foreach(QString s, str.split(QRegularExpression("[\r\n]+"))) {
            s = s.trimmed();
            if (!s.isEmpty())
                includes.insert(s);
        }
        qDebug() << includes;
        *incs = includes.toList();
    }
#if 0
    QSet<QString> set;
    QRegularExpression re(CC_OUT_DEFINES_RE, QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while(it.hasNext()) {
        QRegularExpressionMatch me = it.next();
        set.insert(QString("%1='%2'")
                     .arg(me.captured(1))
                     .arg(me.captured(2)));
    }
    *defs = set.toList();
    qDebug() << *defs;
#else
    Q_UNUSED(defs);
#endif
}
#endif

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
#if 1
    Q_UNUSED(ret);
    MakefileInfo info;
    QString text = proc->readAllStandardOutput();
    info.workingDir = proc->workingDirectory();
    info.allTargets = findAllTargets(text);
    auto it = info.allTargets.cbegin();
    while (it != info.allTargets.cend()) {
        qDebug() << it.key() << ':' << it.value();
        ++it;
    }
    info.targets = filterTargetHeuristic(info.allTargets.keys());
    emit updateFinish(info);

#else
    Q_UNUSED(ret);
    MakefileInfo info;
    QString text = proc->readAllStandardOutput();
    info.workingDir = proc->workingDirectory();
    info.targets = parseRe(text, TARGETS_RE);
    info.defines = parseRe(text, DEFINES_RE);
    info.include = parseRe(text, INCLUDES_RE);

    qDebug() << info.targets;
    qDebug() << info.defines;
    qDebug() << info.include;

    QStringList ccList = parseRe(text, CC_CFLAGS_RE);
    QString cc = ccList.isEmpty()? QString() : ccList.first();
    if (!cc.isEmpty()) {
        info.cc_cflags = cc.remove(QRegularExpression(C_CXX_FILE_RE))
                    .remove(QRegularExpression(O_FILE_RE));
        info.cflags = QString(info.cc_cflags).remove(QRegularExpression(CC_RE));
        qDebug() << "complete CC " << info.cc_cflags;
        qDebug() << "CFLAGS " << info.cflags;
        QtConcurrent::run([this, info]() {
            MakefileInfo _info(info);
            QProcess ccProc;
            ccProc.setWorkingDirectory(info.workingDir);
            QProcessEnvironment env = ccProc.processEnvironment();
            env.insert("LANG", "C");
            ccProc.setProcessEnvironment(env);
            ccProc.setProcessChannelMode(QProcess::MergedChannels);
            qDebug() << "* complete CC " << info.cc_cflags << " at " << ccProc.workingDirectory();
            QString cmd = QString("%1 -xc -dM -E -v -")
                    .arg(QString(info.cc_cflags)
                         .remove("-MMD")
                         .remove(QRegularExpression(R"(-MF \S+)")));
            ccProc.start(cmd);
            qDebug() << "start " << cmd;
            if (ccProc.waitForStarted()) {
                ccProc.closeWriteChannel();
                if (ccProc.waitForFinished()) {
                    QString data(ccProc.readAll());
                    QStringList moreIncs;
                    QStringList moreDefs;
                    parseCCOut(data, &moreIncs, &moreDefs);
                    _info.include += moreIncs;
                    _info.defines += moreDefs;
                } else {
                    qDebug() << "No process terminate. Kill";
                    ccProc.kill();
                }
            } else
                qDebug() << "error at start " << ccProc.errorString();
            emit updateFinish(_info);
        });
    } else
        emit updateFinish(info);
#endif
}

