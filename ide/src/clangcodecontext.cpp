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

const QStringList CLangCodeContext::HANDLE_TYPE = QStringList{ "C", "C++" };

CLangCodeContext::CLangCodeContext(CodeEditor *parent) : QObject(parent), ed(parent)
{
    clangProc = new QProcess(this);
    connect(clangProc, SIGNAL(started()), this, SLOT(clangStarted()));
    connect(clangProc, SIGNAL(finished(int)), this, SLOT(clangTerminated()));
    connect(ed, SIGNAL(updateCodeContext()), this, SLOT(startContextUpdate()));
    connect(this, SIGNAL(completionListUpdate(QStringList)), ed, SLOT(codeContextUpdate(QStringList)));
    connect(this, SIGNAL(discoverCompleted(QStringList,QStringList)), ed, SLOT(discoverCompletion(QStringList,QStringList)));
}

CLangCodeContext::~CLangCodeContext()
= default;

void CLangCodeContext::setWorkingDir(const QString &path)
{
    clangProc->setWorkingDirectory(path);
}

static const QStringList prepend(const QString& s, const QStringList& l) {
    QStringList r;
    for(const auto& x: l) r.append(s + x);
    return r;
}

void CLangCodeContext::startContextUpdate()
{
    clangProc->setProgram("clang");
    int row, col;
    ed->getCursorPosition(&row, &col);
    QStringList argv;
    argv << "-cc1";
    argv << "-code-completion-at";
    argv << QString("-:%1:%2").arg(row + 1).arg(col + 1);
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
    for(const auto& line: text.split('\n')) {
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
            auto make = new QProcess(this);
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
                    QList<int> toRemove;
                    int idx = 0;
                    for(const auto& arg: parameterList) {
                        if (arg.startsWith("-I"))
                            includes.append(QString(arg).remove(0, 2));
                        else if (arg.startsWith("-D"))
                            defines.append(QString(arg).remove(0, 2));
                        else if (QRegularExpression(R"(^-(?:MMD|MM|MG|MP|MD|M)$)").match(arg).hasMatch())
                            toRemove << idx;
                        else if (QRegularExpression(R"(^-(?:MQ|MT|MF)$)").match(arg).hasMatch())
                            toRemove << idx << (idx + 1);
                        else if (arg == "-c")
                            toRemove.append(idx);
                        else if (arg == "-o")
                            toRemove << idx << (idx + 1);
                        idx++;
                    }
                    qSort(toRemove.begin(), toRemove.end(),
                          [](int a, int b) -> bool { return a > b; });
                    for(const auto& i: toRemove)
                        parameterList.removeAt(i);
                    parameterList.append("-dM");
                    parameterList.append("-E");
                    parameterList.append("-v");
                    qDebug() << parameterList;
                    auto cc = new QProcess(this);
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
                            for(const auto& line: defines) {
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
                        emit discoverCompleted(includes, defines);
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
    clangProc->write(ed->text().toLocal8Bit()); // ed->toPlainText().toLocal8Bit());
    clangProc->waitForBytesWritten();
    clangProc->closeWriteChannel();
}

void CLangCodeContext::clangTerminated()
{
    QStringList list;
    QString out = clangProc->readAll();
    // qDebug() << "clang out:\n" << out;
    QRegularExpression re(R"(^COMPLETION: (.*?)$)", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = re.globalMatch(out);
    while(it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        list.append(m.captured(1));
    }
    emit completionListUpdate(list);
}
