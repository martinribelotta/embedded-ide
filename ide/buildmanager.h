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
#ifndef BUILDMANAGER_H
#define BUILDMANAGER_H

#include <QObject>

class ProcessManager;
class ProjectManager;

class BuildManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildManager)
public:
    static const QString PROCESS_NAME;

    explicit BuildManager(ProjectManager *_proj, ProcessManager *_pman, QObject *parent = nullptr);

signals:
    void buildStarted(const QString& target);
    void buildTerminated(int code, const QString& error);

public slots:
    void startBuild(const QString& target);

private:
    ProjectManager *proj;
    ProcessManager *pman;
};

#endif // BUILDMANAGER_H
