#include "clangcodecontext.h"

#include "codeeditor.h"

#include <QProcess>
#include <QTextBlock>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QProcessEnvironment>
#include <QDir>

#include <QtDebug>

CLangCodeContext::CLangCodeContext(CodeEditor *parent) : QObject(parent), ed(parent)
{
    clangProc = new QProcess(this);
    connect(clangProc, SIGNAL(started()), this, SLOT(clangStarted()));
    connect(clangProc, SIGNAL(finished(int)), this, SLOT(clangTerminated()));
    connect(ed, SIGNAL(updateCodeContext()), this, SLOT(startContextUpdate()));
    connect(this, SIGNAL(completionListUpdate(QStringList)), ed, SLOT(codeContextUpdate(QStringList)));
}

CLangCodeContext::~CLangCodeContext()
{
}

void CLangCodeContext::setWorkingDir(const QString &path)
{
    clangProc->setWorkingDirectory(path);
}

static const QStringList prepend(const QString& s, const QStringList& l) {
    QStringList r;
    foreach(QString x, l)
        r.append(s + x);
    return r;
}

void CLangCodeContext::startContextUpdate()
{
    qDebug() << includes << defines;
    QString completionCommand =
            QString("clang -cc1 -code-completion-at -:%1:%2 %3 %4 -")
            .arg(ed->textCursor().blockNumber() + 1)
            .arg(ed->textCursor().columnNumber() + 1)
            .arg(prepend("-D", defines).join(' '))
            .arg(prepend("-I", includes).join(' '))
    ;
    qDebug() << clangProc->workingDirectory();
    qDebug() << completionCommand;
    clangProc->start(completionCommand);
}

static QString findDependency(const QString dep, const MakefileInfo *mk)
{
    auto it = mk->allTargets.cbegin();
    while (it != mk->allTargets.cend()) {
        QString val = it.value();
        if (val.contains(dep))
            return it.key();
        ++it;
    }
    return QString();
}

// static const QString TOKENISER_TEST=R"(-MMD -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16  -Iapp/inc  -Ibase/inc  -Iboard/inc  -Ichip/inc  -DCORE_M4  -D__USE_LPCOPEN  -D__USE_NEWLIB  -DMYVAR="'Hola \" mundo'" -g3 -Os -ffunction-sections -fdata-sections app/src/blinky.c)";
static const QString TOKEN_SEPARATORS(" \t");
static const QString TOKEN_CUOTATIONS(R"("')");
static const QChar ESCAPE_CHAR = '\\';

static QString getToken(QTextStream *s)
{
    bool escaped = false;
    QChar cuoting = QChar();
    QString token;
    while(!s->atEnd()) {
        QChar c;
        *s >> c;
        if (TOKEN_SEPARATORS.contains(c)) {
            if (!cuoting.isNull()) {
                token += c;
                continue;
            }
            if (token.isEmpty()) {
                if (cuoting == QChar())
                    continue;
            } else
                break;
        }
        if (TOKEN_CUOTATIONS.contains(c)) {
            if (!escaped) {
                if (cuoting == c)
                    cuoting = QChar();
                else if (cuoting.isNull())
                    cuoting = c;
            } else
                escaped = false;
            token += c;
            continue;
        }
        escaped = !cuoting.isNull() && (c == ESCAPE_CHAR);
        token += c;
    }
    return token;
}

static QStringList cmdLineTokenizer(const QString& line)
{
    QStringList tokens;
    QString token;
    QTextStream stream(line.toLocal8Bit(), QIODevice::ReadOnly);
    while(!(token = getToken(&stream)).isNull()) {
        tokens.append(token);
    }
    return tokens;
}

static void parseCompilerInfo(const QString& text, QStringList *incs, QStringList *defs)
{
    QRegularExpression definesRe(R"(^\#define (\S+) (.*?$))", QRegularExpression::MultilineOption);
    auto it = definesRe.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        QString symbol = m.captured(1);
        QString value = m.captured(2);
        if (value.contains(QRegularExpression("[ \\t]")))
            value = value.prepend('"').append('"');
        defs->append(QString("%1=%2").arg(symbol).arg(value));
    }
    bool onIncludes = false;
    foreach(QString line, text.split('\n')) {
        if (!onIncludes) {
            if (line.startsWith("#include <")) {
                onIncludes = true;
            }
        } else {
            if (line.startsWith("End of search list"))
                onIncludes = false;
            else
                incs->append(line.trimmed());
        }
    }
}

void CLangCodeContext::discoverFor(const QString &path)
{
    QString workDir = clangProc->workingDirectory();
    QString findElement = QString(path).remove(workDir);
    if (findElement.startsWith(QDir::separator()))
        findElement.remove(0, 1);
    const MakefileInfo *mk = ed->makefileInfo();
    if (mk) {
        QString key = findDependency(findElement, mk);
        if (!key.isEmpty()) {
            qDebug() << "Parsing construction for " << key;
            QProcess *make = new QProcess(this);
            make->setWorkingDirectory(workDir);
            connect( make, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
                    [make](QProcess::ProcessError err) {
                qDebug() << err << make->errorString();
                make->deleteLater();
            });
            connect(make, static_cast<void (QProcess::*)(int)>(&QProcess::finished), [this, make](int ) {
                QString out = make->readAllStandardOutput();
                QRegularExpression re(R"((\S+[g]*(cc|\+\+))\S*\s+(.*?$))", QRegularExpression::MultilineOption);
                QRegularExpressionMatch m = re.match(out);
                make->deleteLater();
                if (m.hasMatch()) {
                    QString compiler = m.captured(1);
                    QString compiler_type = m.captured(2);
                    QString parameters = m.captured(3);
                    qDebug() << "CC:" << compiler << ", type " << compiler_type;
                    QStringList parameterList = cmdLineTokenizer(parameters);
                    parameterList.removeAll("-c");
                    int idx_o;
                    if ((idx_o = parameterList.indexOf("-o")) != -1) {
                            parameterList.removeAt(idx_o);
                            parameterList.removeAt(idx_o);
                    }
                    parameterList.append("-dM");
                    parameterList.append("-E");
                    parameterList.append("-v");
                    qDebug() << parameterList;
                    QProcess *cc = new QProcess(this);
                    cc->setWorkingDirectory(make->workingDirectory());
                    cc->setReadChannelMode(QProcess::MergedChannels);
                    connect(cc, static_cast<void (QProcess::*)(int)>(&QProcess::finished), [this, cc](int ) {
                        QString out = cc->readAll();
                        parseCompilerInfo(out, &includes, &defines);
                        cc->deleteLater();
                        qDebug() << includes;
                        qDebug() << defines;
                    });
                    cc->start(compiler, parameterList);
                }
            });
            make->start(QString("make -Bn %1").arg(key));
        } else
            qDebug() << "not make target for " << findElement;
    } else
        qDebug() << "invalid makefileInfo for " << findElement;
}

void CLangCodeContext::clangStarted()
{
    clangProc->write(ed->toPlainText().toLocal8Bit());
    clangProc->waitForBytesWritten();
    clangProc->closeWriteChannel();
}

void CLangCodeContext::clangTerminated()
{
    QStringList list;
    QString out = clangProc->readAll();
    QRegularExpression re(R"(COMPLETION\: ([^\n]*))", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = re.globalMatch(out);
    while(it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        list.append(m.captured(1));
    }
    emit completionListUpdate(list);
}
