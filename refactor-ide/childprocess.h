#ifndef CHILDPROCESS_H
#define CHILDPROCESS_H

#include <QProcess>

class ChildProcess : public QProcess
{
    Q_OBJECT
public:
    static ChildProcess& create(QObject *parent = nullptr) { return *new ChildProcess(parent); }

    explicit ChildProcess(QObject *parent = nullptr): QProcess(parent) {}
    virtual ~ChildProcess() {}

    ChildProcess& changeCWD(const QString& path) {
        setWorkingDirectory(path);
        return *this;
    }

    ChildProcess& setenv(const QHash<QString, QString> &extraEnv) {
        QProcessEnvironment env = processEnvironment();
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
        connect(this, &QProcess::readyReadStandardOutput, [this, f]() { f(this); });
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
