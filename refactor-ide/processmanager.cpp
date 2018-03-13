#include "processmanager.h"

class ProcessManager::Priv_t
{
public:
};

ProcessManager::ProcessManager(QObject *parent) :
    QObject(parent),
    priv(new Priv_t)
{
}

ProcessManager::~ProcessManager()
{
    delete priv;
}

QProcess *ProcessManager::processFor(const QString &name)
{
    auto proc = findChild<QProcess*>(name);
    if (!proc) {
        proc = new QProcess(this);
        proc->setObjectName(name);
    }
    return proc;
}

void ProcessManager::setTerminationHandler(const QString &name, ProcessManager::terminationHandler_t func)
{
    auto proc = processFor(name);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [proc, func](int exitCode, QProcess::ExitStatus exitStatus) { func(proc, exitCode, exitStatus); });
}

void ProcessManager::setStartupHandler(const QString &name, ProcessManager::startupHandler_t func, bool weak)
{
    auto proc = processFor(name);
    auto conn_normal = connect(proc, &QProcess::started, [proc, func]() { func(proc); });
    if (weak) {
        auto* conn_delete = new QMetaObject::Connection();
        *conn_delete = connect(proc, &QProcess::started, [conn_normal, conn_delete]() {
            QObject::disconnect(conn_normal);
            QObject::disconnect(*conn_delete);
            delete conn_delete;
        });
    }
}

void ProcessManager::setErrorHandler(const QString &name, ProcessManager::errorHandler_t func)
{
    auto proc = processFor(name);
    connect(proc, &QProcess::errorOccurred, [proc, func](QProcess::ProcessError error) { func(proc, error); });
}

void ProcessManager::setStderrInterceptor(const QString &name, ProcessManager::outputHandler_t func)
{
    auto proc = processFor(name);
    connect(proc, &QProcess::readyReadStandardError, [proc, func]() { func(proc, QString(proc->readAllStandardError())); });
}

void ProcessManager::setStdoutInterceptor(const QString &name, ProcessManager::outputHandler_t func)
{
    auto proc = processFor(name);
    connect(proc, &QProcess::readyReadStandardOutput, [proc, func]() { func(proc, QString(proc->readAllStandardOutput())); });
}

void ProcessManager::start(const QString &name, const QString &command, const QStringList &args, const QHash<QString,QString> &extraEnv, const QString &workingDir)
{
    auto proc = processFor(name);
    if (!extraEnv.isEmpty()) {
        auto env = proc->processEnvironment();
        for (auto it = extraEnv.begin(); it != extraEnv.end(); it++)
            env.insert(it.key(), it.value());
        proc->setProcessEnvironment(env);
    }
    proc->setWorkingDirectory(workingDir);
    proc->start(command, args);
}

bool ProcessManager::terminate(const QString &name, bool canKill, int timeout)
{
    auto proc = processFor(name);
    proc->terminate();
    if (!proc->waitForFinished(timeout)) {
        if (canKill) {
            proc->kill();
            return true;
        } else
            return false;
    } else
        return true;
}
