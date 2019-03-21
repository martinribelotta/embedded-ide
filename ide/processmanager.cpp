#include "processmanager.h"

#ifdef Q_OS_UNIX
#include <signal.h>

#elif defined(Q_OS_WIN)
#error TODO windows unsupported kill method
#else
#error Unsupported kill method
#endif

#include <QtDebug>

ProcessManager::ProcessManager(QObject *parent) :
    QObject(parent)
{
}

ProcessManager::~ProcessManager()
= default;

QProcess *ProcessManager::processFor(const QString &name)
{
    auto proc = findChild<QProcess*>(name);
    if (!proc) {
        proc = new QProcess(this);
        proc->setObjectName(name);
    }
    return proc;
}

void ProcessManager::setTerminationHandler(const QString &name, const ProcessManager::terminationHandler_t& func)
{
    auto proc = processFor(name);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [proc, func](int exitCode, QProcess::ExitStatus exitStatus) {
        func(proc, exitCode, exitStatus);
    });
}

void ProcessManager::setStartupHandler(const QString &name, const ProcessManager::startupHandler_t& func)
{
    auto proc = processFor(name);
    connect(proc, &QProcess::started, [proc, func]() { func(proc); });
}

void ProcessManager::setErrorHandler(const QString &name, const ProcessManager::errorHandler_t& func)
{
    auto proc = processFor(name);
    connect(proc, &QProcess::errorOccurred, [proc, func](QProcess::ProcessError error) { func(proc, error); });
}

void ProcessManager::setStderrInterceptor(const QString &name, const ProcessManager::outputHandler_t& func)
{
    auto proc = processFor(name);
    connect(proc, &QProcess::readyReadStandardError, [proc, func]() { func(proc, QString(proc->readAllStandardError())); });
}

void ProcessManager::setStdoutInterceptor(const QString &name, const ProcessManager::outputHandler_t& func)
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
    qDebug() << "START:" << command << args;
    proc->start(command, args);
}

bool ProcessManager::terminate(const QString &name, bool canKill, int timeout)
{
    auto proc = processFor(name);
#ifdef Q_OS_UNIX
    ::kill(pid_t(proc->pid()), SIGINT);
#elif defined(Q_OS_WIN)
    // TODO
#else
    proc->terminate();
#endif
    if (!proc->waitForFinished(timeout)) {
        if (canKill) {
            proc->kill();
            return true;
        } 
            return false;
    } else
        return true;
}
