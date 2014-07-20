#include "projectexporter.h"

#include <QTemporaryFile>

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
                                 const QString &projectPath,
                                 QObject *parent,
                                 const char *slotFinish) :
    QObject(parent),
    m_exportFile(exportFile),
    m_projectPath(projectPath),
    proc(new QProcess(this))
{
    proc->setObjectName("proc");
    QMetaObject::connectSlotsByName(this);
    connect(this, SIGNAL(end(QString)), parent, slotFinish);
    connect(this, SIGNAL(end(QString)), this, SLOT(deleteLater()));
    proc->setStandardOutputFile(exportFile);
}

static QString tmpName() {
    QTemporaryFile f("empty-XXXXXX");
    f.open();
    QString s = QFileInfo(f).fileName();
    f.remove();
    return s;
}

void ProjectExporter::start()
{
    tmpDirName = tmpName();
    if (QDir::root().mkpath(tmpDirName)) {
        proc->setWorkingDirectory(m_projectPath);
        proc->start(QString("diff -Naur %2 %1 .").arg(tmpDirName).arg(EXCLUDE_LIST));
    } else {
        emit end(tr("Can not create empty tmp directory %1").arg(tmpDirName));
    }
}

void ProjectExporter::on_proc_finished(int exitCode, QProcess::ExitStatus exitStatus)
{

    if (QFileInfo(tmpDirName).exists())
        if (!QDir::root().rmpath(tmpDirName)) {
            emit end(tr("Can not remove %1 path").arg(tmpDirName));
            return;
        }
    if (exitStatus != QProcess::NormalExit) {
        emit end(tr("Abnormal termination %1").arg(proc->errorString()));
        return;
    }
    if (exitCode != 0) {
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
