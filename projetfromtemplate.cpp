#include "projetfromtemplate.h"

#include <QProcess>
#include <QFileInfo>
#include <QDir>

#include <QtConcurrent>

#include <QtDebug>

ProjetFromTemplate::ProjetFromTemplate(QString projectName,
                                       QString templateText,
                                       QObject *parent,
                                       const char *slotName) :
    QObject(parent), proc(new QProcess(this)), m_project(std::move(projectName)), m_templateText(std::move(templateText))
{
    connect(proc, SIGNAL(started()), this, SLOT(onStarted()));
    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onFinish(int, QProcess::ExitStatus)));
    connect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onError(QProcess::ProcessError)));
    connect(this, SIGNAL(endOfCreation(QString,QString)), parent, slotName);
    connect(proc, &QProcess::readyReadStandardOutput, [this]() {
        qDebug() << "DIFF: " << proc->readAllStandardOutput();
    });
}

void ProjetFromTemplate::start()
{
    if (QDir::root().mkpath(m_project)) {
        proc->setWorkingDirectory(m_project);
        proc->start(QString("patch -p1"));
    } else {
        emit endOfCreation(QString(), tr("Cant create path %1").arg(m_project));
        deleteLater();
    }
}

void ProjetFromTemplate::onStarted()
{
    qDebug() << "writing" << proc->write(m_templateText.toLocal8Bit());
    while (proc->bytesToWrite() > 0) {
        qDebug() << "..." << proc->bytesToWrite();
        if (proc->waitForBytesWritten())
            break;
    }
    proc->closeWriteChannel();
}

void ProjetFromTemplate::onFinish(int ret, QProcess::ExitStatus status)
{
    QString errorString;
    if (status != QProcess::NormalExit) {
        QDir::root().rmdir(m_project);
        errorString = proc->errorString();
    } else if (ret != 0)
        errorString = tr("Diff return %1").arg(ret);
    emit endOfCreation(m_project, errorString);
    deleteLater();
}

void ProjetFromTemplate::onError(QProcess::ProcessError err)
{
    Q_UNUSED(err);
    emit endOfCreation(QString(), QString("DIFF: %1").arg(proc->errorString()));
    deleteLater();
}
