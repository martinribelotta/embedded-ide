#include "buildmanager.h"

#include <QProcess>
#include <QTextBrowser>

struct BuildManager::Priv_t {
    QProcess *proc;
    QTextBrowser *err;
    QTextBrowser *out;
    QString currentErrLine;
    QString currentOutLine;
};

BuildManager::BuildManager(QObject *parent) :
    QObject(parent), priv(new Priv_t)
{
    priv->proc = new QProcess(this);
    priv->proc->setObjectName("proc");
    priv->err = 0;
    priv->out = 0;
    QMetaObject::connectSlotsByName(this);
}

BuildManager::~BuildManager()
{
    delete priv;
}

void BuildManager::setStdout(QTextBrowser *textBrowser)
{
    priv->out = textBrowser;
}

void BuildManager::setStderr(QTextBrowser *textBrowser)
{
    priv->err = textBrowser;
}

void BuildManager::start(const QString& command)
{
    priv->currentErrLine.clear();
    priv->currentOutLine.clear();
    priv->out->append(QString("<font color=blue>EXEC:</font><tt><font color=green>%1</fond></tt><br>").arg(command));
    priv->proc->start(command);
}

void BuildManager::on_proc_readyReadStandardError()
{
    QByteArray data = priv->proc->readAllStandardError();
    if (priv->err)
        priv->err->append(QString("<font color=black><tt>%1</tt></font>").arg(QString(data).replace("\n", "<br>")));
    foreach(QChar c, data) {
        if (c == QChar(QChar::LineSeparator)) {
            emit stderrLine(priv->currentErrLine);
            priv->currentErrLine.clear();
        } else
            priv->currentErrLine.append(c);
    }
}

void BuildManager::on_proc_readyReadStandardOutput()
{
    QByteArray data = priv->proc->readAllStandardOutput();
    if (priv->out)
        priv->out->append(QString("<font color=black><tt>%1</tt></font>").arg(QString(data).replace("\n", "<br>")));
    foreach(QChar c, data) {
        if (c == QChar(QChar::LineSeparator)) {
            emit stdoutLine(priv->currentOutLine);
            priv->currentOutLine.clear();
        } else
            priv->currentOutLine.append(c);
    }
}

static QString statusToColorName(int status) {
    switch(QProcess::ExitStatus(status)) {
    default:
    case QProcess::NormalExit: return "blue";
    case QProcess::CrashExit: return "red";
    }
}

void BuildManager::on_proc_finished(int exitCode, QProcess::ExitStatus status)
{
    if (priv->out)
        priv->out->append(QString("<font color=%1>%2</font><br>")
                              .arg(statusToColorName(status))
                              .arg(exitCode?
                                       QString("Terminate with error code %1").arg(exitCode):
                                       "Terminate without errors"));
}
