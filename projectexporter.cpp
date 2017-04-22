#include "projectexporter.h"

#include <QTemporaryDir>
#include <QtDebug>

static const QString EXCLUDE_LIST = QString(
        "-x .git* "
        "-x *.o "
        "-x *.d "
        "-x *.elf "
        "-x *.lst "
        "-x *.map "
        "-x .config* "
        "-x *.conf "
        "-x autoconf.h");

ProjectExporter::ProjectExporter(const QString& exportFile,
                                 QString projectPath,
                                 QObject *parent,
                                 const char *slotFinish) :
    QObject(parent),
    m_exportFile(exportFile),
    m_projectPath(std::move(projectPath)),
    proc(new QProcess(this)),
    tmpDir("a_XXXXXX")
{
    tmpDir.setAutoRemove(true);
    proc->setObjectName("proc");
    QMetaObject::connectSlotsByName(this);
    connect(this, SIGNAL(end(QString)), parent, slotFinish);
    connect(this, SIGNAL(end(QString)), this, SLOT(deleteLater()));
    proc->setStandardOutputFile(exportFile);
}

void ProjectExporter::start()
{
    if (tmpDir.isValid()) {
        proc->setWorkingDirectory(m_projectPath);
        QString cmd = QString("diff -u -r --unidirectional-new-file %2 %1 .")
                .arg(tmpDir.path())
                .arg(EXCLUDE_LIST);
        //qDebug() << cmd;
        proc->start(cmd);
    } else {
        emit end(tr("Can not create empty tmp directory %1").arg(tmpDir.path()));
    }
}

void ProjectExporter::on_proc_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::NormalExit) {
        emit end(tr("Abnormal termination %1").arg(proc->errorString()));
        return;
    }
    if (exitCode > 1) {
        emit end(tr("End with error code %1").arg(exitCode));
        return;
    }
    emit end(QString());
}

void ProjectExporter::on_proc_error(QProcess::ProcessError error)
{
    Q_UNUSED(error);
    emit end(proc->errorString());
}
