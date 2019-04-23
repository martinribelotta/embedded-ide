/*
 * This file is part of Embedded-IDE
 * 
 * Copyright 2019 Martin Ribelotta <martinribelotta@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
    pman->setErrorHandler(PROCESS_NAME, [](QProcess *proc, QProcess::ProcessError err) {
        // TODO Implement this (maybe unnecesary?)
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
