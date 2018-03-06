#include "buildmanager.h"
#include "processmanager.h"
#include "projectmanager.h"

#include <QFileInfo>

#include <QtDebug>

const QString BuildManager::PROCESS_NAME = "makeBuild";

BuildManager::BuildManager(ProjectManager *_proj, ProcessManager *_pman, QObject *parent) :
    QObject(parent),
    proj(_proj),
    pman(_pman)
{
    pman->setErrorHandler(PROCESS_NAME, [this](QProcess *proc, QProcess::ProcessError err) {
        // TODO
        Q_UNUSED(proc);
        Q_UNUSED(err);
    });
    pman->setTerminationHandler(PROCESS_NAME, [this](QProcess *proc, int code, QProcess::ExitStatus status) {
        emit buildTerminated(code, status == QProcess::NormalExit? tr("Exit normal") : proc->errorString());
    });
}

void BuildManager::startBuild(const QString &target)
{
    pman->start(PROCESS_NAME, "make", { "-f", proj->projectFile(), target }, {}, proj->projectPath());
    emit buildStarted(target);
}
