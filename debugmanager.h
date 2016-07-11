#ifndef DEBUGMANAGER_H
#define DEBUGMANAGER_H

#include <QObject>

#include <qgdb/core.h>

class DocumentView;

class DebugManager : public QObject, public ICore
{
    Q_OBJECT

public:
    explicit DebugManager(DocumentView *v, QObject *parent = 0);
    virtual ~DebugManager();

    virtual void ICore_onStopped(StopReason reason, QString path, int lineNo);
    virtual void ICore_onStateChanged(TargetState state);
    virtual void ICore_onSignalReceived(QString signalName);
    virtual void ICore_onLocalVarReset();
    virtual void ICore_onLocalVarChanged(QString name, QString value);
    virtual void ICore_onFrameVarReset();
    virtual void ICore_onFrameVarChanged(QString name, QString value);
    virtual void ICore_onWatchVarChanged(int watchId, QString name, QString value);
    virtual void ICore_onConsoleStream(QString text);
    virtual void ICore_onBreakpointsChanged();
    virtual void ICore_onThreadListChanged();
    virtual void ICore_onCurrentThreadChanged(int threadId);
    virtual void ICore_onStackFrameChange(QList<StackFrameEntry> stackFrameList);
    virtual void ICore_onMessage(QString message);
    virtual void ICore_onTargetOutput(QString message);
    virtual void ICore_onCurrentFrameChanged(int frameIdx);

signals:


public slots:


private:
    struct DebugManagerPriv;
    DebugManagerPriv *priv;
};

#endif // DEBUGMANAGER_H
