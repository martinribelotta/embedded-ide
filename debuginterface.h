#ifndef DEBUGINTERFACE_H
#define DEBUGINTERFACE_H

#include <QWidget>

#include "documentarea.h"
#include "projectview.h"
#include "qgdb/core.h"

namespace Ui {
class DebugInterface;
}

class DebugInterface : public QWidget, ICore
{
    Q_OBJECT

public:
    explicit DebugInterface(QWidget *parent = 0);
    ~DebugInterface();

    void setProjectView(ProjectView *prj) { projectView = prj; }
    void setDocumentArea(DocumentArea *da) { documentArea = da; }

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
    void gdbOutput(const QString& text);
    void gdbMessage(const QString& text);
    void applicationOutput(const QString& text);

private slots:
    void on_buttonStartDebug_clicked();

    void on_buttonStopDebug_clicked();

    void on_buttonDebugRun_clicked();

    void on_buttonDebugStepOver_clicked();

    void on_buttonDebugStepInto_clicked();

    void on_buttonDebugStepOut_clicked();

    void on_buttonWatchAdd_clicked();

    void on_buttonWatchRemove_clicked();

    void on_buttonWatchClear_clicked();

    void on_buttonBreakRemove_clicked();

    void on_buttonBreakClear_clicked();

    void on_listBreakpoints_activated(const QModelIndex &index);

    void on_buttonDebugContinue_clicked();

private:
    struct Priv_t;
    Ui::DebugInterface *ui;
    Priv_t *priv;
    ProjectView *projectView;
    DocumentArea *documentArea;
};

#endif // DEBUGINTERFACE_H
