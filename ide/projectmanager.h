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
#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QVariant>
#include <QFile>

class QListView;

class ProcessManager;
class ICodeModelProvider;

class ProjectManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProjectManager)
public:
    explicit ProjectManager(QListView *view, ProcessManager *pman, QObject *parent = nullptr);
    virtual ~ProjectManager();

    QString projectName() const;
    QString projectPath() const;
    QString projectFile() const;
    bool isProjectOpen() const;
    ICodeModelProvider *codeModel() const;
    void setCodeModelProvider(ICodeModelProvider *modelProvider);

    QStringList dependenciesForTarget(const QString& target);
    QStringList targetsOfDependency(const QString& dep);

    void deleteOnCloseProject(QObject *p) {
        connect(this, &ProjectManager::projectClosed, p, &QObject::deleteLater);
    }

signals:
    void projectOpened(const QString& makePath);
    void projectClosed();
    void targetTriggered(const QString& target);
    void requestFileOpen(const QString& path);
    void exportFinish(const QString& exportMessage);

public slots:
    void createProject(const QString& projectFilePath, const QString& templateFile);
    void createProjectFromTGZ(const QString& projectFilePath, const QString& tgzFile);
    void exportCurrentProjectTo(const QString& fileName);
    void exportToDiff(const QString &patchFile);
    void exportToTarGz(const QString &tgzFile);
    void openProject(const QString& makefile);
    void closeProject();
    void reloadProject();

    void showMessage(const QString& msg);
    void showMessageTimed(const QString& msg, int millis = 3000);
    void clearMessage();
    void clearMessageTimed(int millis = 3000);

private:
    class Priv_t;
    Priv_t *priv;
};

#endif // PROJECTMANAGER_H
