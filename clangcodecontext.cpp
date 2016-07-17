#include "clangcodecontext.h"

#include "codeeditor.h"

#include <QProcess>
#include <QTextBlock>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>

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
    const MakefileInfo *mk = ed->makefileInfo();
    QString completionCommand =
            QString("clang -cc1 -code-completion-at -:%1:%2 %3 %4 -")
            .arg(ed->textCursor().blockNumber() + 1)
            .arg(ed->textCursor().columnNumber() + 1)
            .arg(mk? prepend("-D", mk->defines).join(' '): "")
            .arg(mk? prepend("-I", mk->include).join(' '): "")
    ;
    qDebug() << clangProc->workingDirectory();
    qDebug() << completionCommand;
    clangProc->start(completionCommand);
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
    QRegularExpression re("COMPLETION\\: ([^\\n]*)", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = re.globalMatch(out);
    while(it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        list.append(m.captured(1));
    }
    emit completionListUpdate(list);
}
