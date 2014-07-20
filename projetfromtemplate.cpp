#include "projetfromtemplate.h"

#include <QProcess>
#include <QFileInfo>
#include <QDir>

ProjetFromTemplate::ProjetFromTemplate(const QString &projectName,
                                       const QString &templateText,
                                       QObject *parent,
                                       const char *slotName) :
    QObject(parent), proc(new QProcess(this)), m_project(projectName), m_templateText(templateText)
{
    connect(proc, SIGNAL(started()), this, SLOT(onStarted()));
    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onFinish(int, QProcess::ExitStatus)));
    connect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onError(QProcess::ProcessError)));
    connect(this, SIGNAL(endOfCreation(QString,QString)), parent, slotName);
}

void ProjetFromTemplate::start()
{
    if (QDir::root().mkpath(m_project)) {
        proc->setWorkingDirectory(m_project);
        proc->start(QString("patch -p0"));
    } else {
        emit endOfCreation(QString(), tr("Cant create path %1").arg(m_project));
        deleteLater();
    }
}

void ProjetFromTemplate::onStarted()
{
    proc->write(m_templateText.toLocal8Bit());
    proc->closeWriteChannel();
}

void ProjetFromTemplate::onFinish(int ret, QProcess::ExitStatus status)
{
    QDir::root().rmdir(m_project);
    QString errorString;
    if (status != QProcess::NormalExit)
        errorString = proc->errorString();
    else if (ret != 0)
        errorString = tr("Diff return %1").arg(ret);
    emit endOfCreation(m_project, errorString);
    deleteLater();
}

void ProjetFromTemplate::onError(QProcess::ProcessError err)
{
    Q_UNUSED(err);
    emit endOfCreation(QString(), proc->errorString());
    deleteLater();
}
