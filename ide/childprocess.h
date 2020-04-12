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
#ifndef CHILDPROCESS_H
#define CHILDPROCESS_H

#include <QProcess>

class ChildProcess : public QProcess
{
    Q_OBJECT
public:
    static ChildProcess& create(QObject *parent = nullptr) { return *new ChildProcess(parent); }

    static QProcess *safeStop(QProcess *p, int timeoutMilis = 100);

    explicit ChildProcess(QObject *parent = nullptr): QProcess(parent) {}
    virtual ~ChildProcess();

    ChildProcess& changeCWD(const QString& path) {
        setWorkingDirectory(path);
        return *this;
    }

    ChildProcess& makeDeleteLater() {
        connect(this, QOverload<int>::of(&ChildProcess::finished), this, &ChildProcess::deleteLater);
        connect(this, &ChildProcess::errorOccurred, this, &ChildProcess::deleteLater);
        return *this;
    }

    ChildProcess& mergeStdOutAndErr() {
        setReadChannelMode(MergedChannels);
        return *this;
    }

    ChildProcess& setenv(const QHash<QString, QString> &extraEnv) {
        auto env = QProcessEnvironment::systemEnvironment();
        for(auto it = extraEnv.begin(); it != extraEnv.end(); ++it)
            env.insert(it.key(), it.value());
        setProcessEnvironment(env);
        return *this;
    }

    template<typename T> ChildProcess& onFinished(T f)
    {
        connect(this, QOverload<int>::of(&QProcess::finished), [this, f](int e) { f(this, e); });
        return *this;
    }

    template<typename T> ChildProcess& onStarted(T f)
    {
        connect(this, &QProcess::started, [this, f](){ f(this); });
        return *this;
    }

    template<typename T> ChildProcess& onError(T f)
    {
        connect(this, &QProcess::errorOccurred, [this, f](ProcessError e) { f(this, e); });
        return *this;
    }

    template<typename T> ChildProcess& onReadyReadStdout(T f)
    {
        connect(this, &QProcess::readyReadStandardOutput, [this, f]() {
            f(this);
        });
        return *this;
    }

    template<typename T> ChildProcess& onReadyReadStderr(T f)
    {
        connect(this, &QProcess::readyReadStandardError, [this, f]() { f(this); });
        return *this;
    }

    template<typename T> ChildProcess& onStateChange(T f)
    {
        connect(this, &QProcess::stateChanged, [this, f](ProcessState st) { f(this, st); });
        return *this;
    }

signals:

public slots:
};

#endif // CHILDPROCESS_H
