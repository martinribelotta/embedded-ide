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
#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QObject>
#include <QProcess>

#include <functional>

class ProcessManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProcessManager)
public:
    typedef std::function<void (QProcess *, const QString&)> outputHandler_t;
    typedef std::function<void (QProcess *, int, QProcess::ExitStatus)> terminationHandler_t;
    typedef std::function<void (QProcess *)> startupHandler_t;
    typedef std::function<void (QProcess *, QProcess::ProcessError)> errorHandler_t;

    explicit ProcessManager(QObject *parent = nullptr);
    virtual ~ProcessManager();

    QProcess *processFor(const QString& name);

    void setTerminationHandler(const QString& name, const terminationHandler_t& func);
    void setStartupHandler(const QString& name, const startupHandler_t& func);
    void setErrorHandler(const QString& name, const errorHandler_t& func);
    void setStderrInterceptor(const QString& name, const outputHandler_t& func);
    void setStdoutInterceptor(const QString& name, const outputHandler_t& func);

    bool isRunning(const QString& name) { return processFor(name)->state() == QProcess::Running; }

public slots:
    void start(const QString& name, const QString& command, const QStringList& args = {}, const QHash<QString, QString> &extraEnv = {}, const QString& workingDir = QString());
    bool terminate(const QString& name, bool canKill = false, int timeout = 3000);

signals:
};

#endif // PROCESSMANAGER_H
