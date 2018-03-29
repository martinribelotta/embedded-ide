#include "clangautocompletionprovider.h"
#include "processmanager.h"

#include <QProcess>
#include <QRegularExpressionMatch>

#include <QtDebug>

ClangAutocompletionProvider::ClangAutocompletionProvider(ProcessManager *_pman, QObject *parent):
    QObject(parent),
    pman(_pman)
{
}

ClangAutocompletionProvider::~ClangAutocompletionProvider()
{

}

void ClangAutocompletionProvider::startIndexingProject(const QString &path, const QHash<QString, QString> &targetMap)
{

}

void ClangAutocompletionProvider::startIndexingFile(const QString &path)
{

}

void ClangAutocompletionProvider::declarationOf(const QString &entity, ICodeModelProvider::FindReferenceCallback_t cb)
{
    // TODO
}

void ClangAutocompletionProvider::referenceOf(const QString &entity, ICodeModelProvider::FindReferenceCallback_t cb)
{
    // TODO
}

static QString parseCompletion(const QString& text)
{
    QString s(text);
    if (s.startsWith("Pattern : ")) {
        s = s.remove("Pattern : ").remove(QRegularExpression(R"([\[|\\<]\#[^\#]*\#[\]|\>])"));
    } else if (s.contains(':')) {
        s = s.split(':').at(0).trimmed();
    }
    return s;
}

void ClangAutocompletionProvider::completionAt(const ICodeModelProvider::FileReference &ref, const QString &unsaved, ICodeModelProvider::CompletionCallback_t cb)
{
    auto clang = new QProcess(this);
    connect(clang, &QProcess::started, [this, clang, unsaved]() {
        clang->write(unsaved.toLocal8Bit());
        clang->waitForBytesWritten();
        clang->closeWriteChannel();
    });
    connect(clang, &QProcess::errorOccurred, [this, clang](QProcess::ProcessError err) {
        qDebug() << "clang error:" << clang->errorString() << err;
    });
    connect(clang, QOverload<int>::of(&QProcess::finished), [this, clang, cb](int exitStatus) {
        Q_UNUSED(exitStatus);
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
    });
    connect(clang, QOverload<int>::of(&QProcess::finished), clang, &QObject::deleteLater);
    clang->start("clang", {
                     "-x",
                     "c",
                     "-fcolor-diagnostics",
                     "-fsyntax-only",
                     "-Xclang",
                     "-code-completion-macros",
                     "-Xclang",
                     "-code-completion-patterns",
                     "-Xclang",
                     "-code-completion-brief-comments",
                     "-Xclang",
                     QString("-code-completion-at=-:%1:%2").arg(ref.line + 1).arg(ref.column + 1),
                     "-",
                 });
}
