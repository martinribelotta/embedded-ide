#include "clangcodecontext.h"

#include "codeeditor.h"

#include <QProcess>
#include <QTextBlock>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QProcessEnvironment>
#include <QDir>
#include <QTemporaryFile>

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
    clangProc->setProgram("clang");
    QStringList argv;
    argv << "-cc1";
    argv << "-code-completion-at";
    argv << QString("-:%1:%2")
            .arg(ed->textCursor().blockNumber() + 1)
            .arg(ed->textCursor().columnNumber() + 1);
    argv << prepend("-D", defines);
    argv << prepend("-I", includes);
    qDebug() << "workdir:" << clangProc->workingDirectory();
    qDebug() << "argv:" << argv;
    clangProc->start("clang", argv);
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

static const QRegularExpression EOL(R"([\r\n])");

static void parseCompilerInfo(const QString& text, QStringList *incs, QStringList *defs)
{
#if 0
    QRegularExpression definesRe(R"(^\#define (\S+) (.*?)$)", QRegularExpression::MultilineOption);
    auto it = definesRe.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
#if 1
        QString define = m.captured(0);
        defs->append(define);
#else
        QString symbol = m.captured(1).remove(EOL);
        QString value = m.captured(2).remove(EOL);
        if (value.contains(QRegularExpression("[ \\t]")))
            value = value.prepend('"').append('"');
        defs->append(QString("%1=%2").arg(symbol).arg(value));
#endif
    }
#else
    Q_UNUSED(defs);
#endif
    bool onIncludes = false;
    foreach(QString line, text.split('\n')) {
        if (!onIncludes) {
            if (line.startsWith("#include <")) {
                onIncludes = true;
            }
        } else {
            if (line.startsWith("End of search list"))
                onIncludes = false;
            else {
                QString ipath = line.trimmed();
                if (!incs->contains(ipath))
                    incs->append(ipath);
            }
        }
    }
}

void CLangCodeContext::discoverFor(const QString &path)
{
    QString workDir = clangProc->workingDirectory();
    QString findElement = QString(path).remove(workDir);
    if (findElement.startsWith('\\') || findElement.startsWith('/'))
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
                    foreach(QString arg, parameterList) {
                        if (arg.startsWith("-I"))
                            includes.append(QString(arg).remove(0, 2));
                        else if (arg.startsWith("-D"))
                            defines.append(QString(arg).remove(0, 2));
                    }

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
#if 0
                        QTemporaryFile *temporaryDefines = new QTemporaryFile(this);
                        temporaryDefines->setObjectName("temporaryDefines");
                        if (temporaryDefines->open()) {
                            QTextStream stream(temporaryDefines);
                            foreach(QString line, defines) {
                                stream << QString("%1\n").arg(line);
                            }
                            temporaryDefines->close();
                        } else {
                            qDebug() << "error opening tmp defs: " << temporaryDefines->errorString();
                            temporaryDefines->deleteLater();
                        }
#endif
                        cc->deleteLater();
                        qDebug() << "Includes:" << includes;
                        qDebug() << "Defines:" << defines;
                    });
                    connect(cc, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error), [this, cc](QProcess::ProcessError err) {
                        Q_UNUSED(err);
                        qDebug() << "CC ERROR: " << cc->program() << cc->arguments() << "\n"
                                 << "\t" << cc->errorString();
                        cc->deleteLater();
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