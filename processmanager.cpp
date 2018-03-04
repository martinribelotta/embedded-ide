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

template<class Map>
struct RangeWrapper {
    typedef typename Map::const_iterator MapIterator;
    Map &map;

    RangeWrapper(Map & map_) : map(map_) {}

    struct iterator {
        MapIterator mapIterator;
        iterator(const MapIterator &mapIterator_): mapIterator(mapIterator_) {}
        MapIterator operator*() {
            return mapIterator;
        }
        iterator & operator++() {
            ++mapIterator;
            return *this;
        }
        bool operator!=(const iterator & other) {
            return this->mapIterator != other.mapIterator;
        }
    };
    iterator begin() {
        return map.begin();
    }
    iterator end() {
        return map.end();
    }
};

template<class Map>
RangeWrapper<Map> toRange(Map & map)
{
    return RangeWrapper<Map>(map);
}

void ProcessManager::start(const QString &name, const QString &command, const QStringList &args, const QHash<QString,QString> &extraEnv, const QString &workingDir)
{
    auto proc = processFor(name);
    if (!extraEnv.isEmpty()) {
        auto env = proc->processEnvironment();
        for(const auto e: toRange(extraEnv))
            env.insert(e.key(), e.value());
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
