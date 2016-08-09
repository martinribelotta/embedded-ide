#include "debugmanager.h"
#if 0
#include "codeeditor.h"
#include "documentarea.h"
#include "documentview.h"

struct DebugManager::DebugManagerPriv {
    DocumentView *projectManager;
};

DebugManager::DebugManager(DocumentView *v, QObject *parent) :
    QObject(parent), priv(new DebugManagerPriv)
{
    priv->projectManager = v;
    Core::getInstance().setParent(this);
    Core::getInstance().setListener(this);
}

DebugManager::~DebugManager()
{
    delete priv;
}

void DebugManager::ICore_onStopped(StopReason reason, QString path, int lineNo)
{
}

void DebugManager::ICore_onStateChanged(TargetState state)
{
}

void DebugManager::ICore_onSignalReceived(QString signalName)
{
}

void DebugManager::ICore_onLocalVarReset()
{
}

void DebugManager::ICore_onLocalVarChanged(QString name, QString value)
{
}

void DebugManager::ICore_onFrameVarReset()
{
}

void DebugManager::ICore_onFrameVarChanged(QString name, QString value)
{
}

void DebugManager::ICore_onWatchVarChanged(int watchId, QString name, QString value)
{
}

void DebugManager::ICore_onConsoleStream(QString text)
{
}

void DebugManager::ICore_onBreakpointsChanged()
{
}

void DebugManager::ICore_onThreadListChanged()
{
}

void DebugManager::ICore_onCurrentThreadChanged(int threadId)
{
}

void DebugManager::ICore_onStackFrameChange(QList<StackFrameEntry> stackFrameList)
{
}

void DebugManager::ICore_onMessage(QString message)
{
}

void DebugManager::ICore_onTargetOutput(QString message)
{
}

void DebugManager::ICore_onCurrentFrameChanged(int frameIdx)
{
}

#endif
