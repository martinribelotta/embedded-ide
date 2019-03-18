#include "appconfig.h"
#include "childprocess.h"
#include "clangautocompletionprovider.h"
#include "projectmanager.h"
#include "textmessagebrocker.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpressionMatch>
#include <QTimer>

#include <QtDebug>

static QString getToken(QTextStream *s)
{
    bool escaped = false;
    QChar cuoting = QChar();
    QString token;
    while(!s->atEnd()) {
        QChar c;
        *s >> c;
        if (QString(" \t").contains(c)) {
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
        if (QString("\"'").contains(c)) {
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
        escaped = !cuoting.isNull() && (c == '\\');
        token += c;
    }
    return token;
}

static QStringList cmdLineTokenizer(const QString& line)
{
    QStringList tokens;
    QString token;
    QTextStream stream(line.toLocal8Bit(), QIODevice::ReadOnly);
    while(!(token = getToken(&stream)).isNull())
        tokens.append(token);
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
    for(const QString& line: text.split('\n')) {
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
                    incs->append("-I" + ipath);
            }
        }
    }
}

class ClangAutocompletionProvider::Priv_t
{
public:
    ProjectManager *project{ nullptr };
    QHash<QString, ICodeModelProvider::FileReferenceList> nameMap;
    QStringList includes;
    QStringList defines;
};

ClangAutocompletionProvider::ClangAutocompletionProvider(ProjectManager *proj, QObject *parent):
    QObject(parent), priv(new Priv_t)
{
    priv->project = proj;
}

ClangAutocompletionProvider::~ClangAutocompletionProvider()
{
    delete priv;
}

void ClangAutocompletionProvider::startIndexingProject(const QString &path)
{
    priv->nameMap.clear();
    ChildProcess::create(this)
    .changeCWD(path)
    .makeDeleteLater()
    .onError([](QProcess *ctags, QProcess::ProcessError) {
        TextMessageBrocker::instance().publish("actionLabel", tr("ctags error: %1").arg(ctags->errorString()));
        QTimer::singleShot(3000, []() { TextMessageBrocker::instance().publish("actionLabel", QString()); });
    })
    .onFinished([this](QProcess *ctags, int exitStatus) {
        qDebug() << "ctags end with" << exitStatus;
        ctags->setReadChannel(QProcess::StandardOutput);
        while(ctags->bytesAvailable() > 0) {
            auto line = ctags->readLine();
            auto entry = QJsonDocument::fromJson(line).object();
            if (!entry.isEmpty()) {
                auto name = entry.value("name").toString();
                auto text = entry.value("text").toString();
                ICodeModelProvider::FileReference r = {
                    entry.value("path").toString(),
                    entry.value("line").toInt(), 0,
                    text, // meta
                };
                priv->nameMap[name].append(r);
            }
        }
        TextMessageBrocker::instance().publish("actionLabel", tr("Index finished"));
        QTimer::singleShot(3000, []() { TextMessageBrocker::instance().publish("actionLabel", QString()); });
    })
    .start("universal-ctags", {
                 "--map-R=-.s",
                 "-n", "-R", "-e",
                 "--all-kinds=*",
                 "--extras=*",
                 "--fields=*",
                 "-x",
                 R"(--_xformat={ "name": "%N", "lang": "%l", "type": "%K", "path": "%F", "line": %n, "text": "%C" })"
             });
    TextMessageBrocker::instance().publish("actionLabel", tr("Start indexing"));
}

void ClangAutocompletionProvider::startIndexingFile(const QString &path)
{
    Q_UNUSED(path);
    auto targets = priv->project->targetsOfDependency(path);
    ChildProcess::create(this)
            .makeDeleteLater()
            .changeCWD(priv->project->projectPath())
            .onFinished([this](QProcess *make, int exitCode)
    {
        qDebug() << "make discover exit with" << exitCode;
        QString out = make->readAllStandardOutput();
        QRegularExpression re(R"((\S+[g]*(cc|\+\+))\S*\s+(.*?$))", QRegularExpression::MultilineOption);
        QRegularExpressionMatch m = re.match(out);
        if (m.hasMatch()) {
            QString compiler = m.captured(1);
            QString compiler_type = m.captured(2);
            QString parameters = m.captured(3);
            qDebug() << "CC:" << compiler << ", type " << compiler_type;
            QStringList parameterList = cmdLineTokenizer(parameters);
            QList<int> toRemove;
            int idx = 0;
            for(const QString& arg: parameterList) {
                if (arg.startsWith("-I"))
                    priv->includes.append(arg);
                else if (arg.startsWith("-D"))
                    priv->defines.append(arg);
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
            std::sort(toRemove.begin(), toRemove.end(),
                  [](int a, int b) -> bool { return a > b; });
            for(const auto& i: toRemove)
                parameterList.removeAt(i);
            parameterList.append("-dM");
            parameterList.append("-E");
            parameterList.append("-v");
            qDebug() << parameterList;
            ChildProcess::create(this)
                    .changeCWD(make->workingDirectory())
                    .mergeStdOutAndErr()
                    .makeDeleteLater()
                    .onFinished([this](QProcess *cc, int) {
                QString out = cc->readAll();
                parseCompilerInfo(out, &priv->includes, &priv->defines);
                qDebug() << "Includes:" << priv->includes;
                qDebug() << "Defines:" << priv->defines;
            }).onError([](QProcess *cc, QProcess::ProcessError err) {
                Q_UNUSED(err);
                qDebug() << "CC ERROR: " << cc->program() << cc->arguments() << "\n"
                         << "\t" << cc->errorString();
            }).start(compiler, parameterList);
        }
    }).start("make", QStringList{ "-B", "-n" } + targets);
}

void ClangAutocompletionProvider::referenceOf(const QString &entity, ICodeModelProvider::FindReferenceCallback_t cb)
{
    cb(priv->nameMap.value(entity));
}

static QString parseCompletion(const QString& text)
{
    return text.startsWith("Pattern : ")?
                QString(text).remove("Pattern : ").remove(QRegularExpression(R"([\[|\\<]\#[^\#]*\#[\]|\>])")) :
                text.contains(':')?
                    QString(text.split(':').at(0)).trimmed() : text;
}

void ClangAutocompletionProvider::completionAt(const ICodeModelProvider::FileReference &ref, const QString &unsaved, ICodeModelProvider::CompletionCallback_t cb)
{
    ChildProcess::create(this)
            .makeDeleteLater()
            .changeCWD(priv->project->projectPath())
            .onStarted([unsaved](QProcess *clang) {
        clang->write(unsaved.toLocal8Bit());
        clang->waitForBytesWritten();
        clang->closeWriteChannel();
    }).onError([](QProcess *clang, QProcess::ProcessError err) {
        qDebug() << "clang error:" << clang->errorString() << err;
    }).onFinished([cb](QProcess *clang, int exitStatus) {
        Q_UNUSED(exitStatus);
        clang->deleteLater();
        // qDebug() << "clang finish:" << exitStatus;
        QStringList list;
        QString out = clang->readAllStandardOutput();
        // qDebug() << "clang out:\n" << out;
        QRegularExpression re(R"(^COMPLETION: (.*?)$)", QRegularExpression::MultilineOption);
        auto it = re.globalMatch(out);
        while(it.hasNext()) {
            auto m = it.next();
            list.append(parseCompletion(m.captured(1)));
        }
        cb(list);
    }).start("clang", QStringList{
                 "-x", "c", "-fcolor-diagnostics", "-fsyntax-only",
                 "-Xclang", "-code-completion-macros",
                 "-Xclang", "-code-completion-patterns",
                 "-Xclang", "-code-completion-brief-comments",
                 "-Xclang", QString("-code-completion-at=-:%1:%2").arg(ref.line + 1).arg(ref.column + 1),
                 "-"
             } + priv->defines + priv->includes);
}
