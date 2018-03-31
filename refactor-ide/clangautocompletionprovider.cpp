#include "appconfig.h"
#include "childprocess.h"
#include "clangautocompletionprovider.h"
#include "projectmanager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpressionMatch>

#include <QtDebug>

class ClangAutocompletionProvider::Priv_t
{
public:
    ProjectManager *project;
    QHash<QString, ICodeModelProvider::FileReferenceList> nameMap;
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

#if 0
static void refMapToString(const QString& filename, const QHash<QString, ICodeModelProvider::FileReferenceList>& map)
{
    QFile f(filename);
    if (f.open(QFile::WriteOnly)) {
        f.write("{\n");
        for(auto it=map.begin(); it!=map.end(); ++it) {
            f.write(QString("  \"%1\"").arg(it.key()).toLocal8Bit());
            f.write(": [\n");
            for(auto e: it.value())
                f.write(QString("    { \"path\": \"%1\", \"line\": %2, \"col\": %3, \"meta\": \"%4\" },\n")
                         .arg(e.path)
                         .arg(e.line)
                         .arg(e.column)
                         .arg(e.meta).toLocal8Bit());
            f.write("  {} ],\n");
        }
        f.write("{} ]\n");
    } else
        qDebug() << filename << ":" << f.errorString();
}
#endif

void ClangAutocompletionProvider::startIndexingProject(const QString &path)
{
    priv->nameMap.clear();
    auto p = new ChildProcess(this);
    p->changeCWD(path);
    p->onFinished([this](QProcess *ctags, int exitStatus) {
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
    });
    p->start("universal-ctags", {
                 "--map-R=-.s",
                 "-n", "-R", "-e",
                 "--all-kinds=*",
                 "--extras=*",
                 "--fields=*",
                 "-x",
                 R"(--_xformat={ "name": "%N", "lang": "%l", "type": "%K", "path": "%F", "line": %n, "text": "%C" })"
             });
}

void ClangAutocompletionProvider::startIndexingFile(const QString &path)
{
    Q_UNUSED(path);
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
            .changeCWD(priv->project->projectPath())
            .onStarted([this, unsaved](QProcess *clang) {
        clang->write(unsaved.toLocal8Bit());
        clang->waitForBytesWritten();
        clang->closeWriteChannel();
    }).onError([this](QProcess *clang, QProcess::ProcessError err) {
        qDebug() << "clang error:" << clang->errorString() << err;
        clang->deleteLater();
    }).onFinished([this, cb](QProcess *clang, int exitStatus) {
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
    }).start("clang", {
                 "-x", "c", "-fcolor-diagnostics", "-fsyntax-only",
                 "-Xclang", "-code-completion-macros",
                 "-Xclang", "-code-completion-patterns",
                 "-Xclang", "-code-completion-brief-comments",
                 "-Xclang", QString("-code-completion-at=-:%1:%2").arg(ref.line + 1).arg(ref.column + 1),
                 "-"
             });
}
