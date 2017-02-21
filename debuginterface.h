#ifndef DEBUGINTERFACE_H
#define DEBUGINTERFACE_H

#include <QWidget>

#include "documentarea.h"
#include "projectview.h"
#include "qgdb/core.h"

namespace Ui {
class DebugInterface;
}

class QTreeWidgetItem;

class DebugInterface : public QWidget, ICore
{
    Q_OBJECT

public:
    enum DispFormat {
        DISP_NATIVE,
        DISP_DEC,
        DISP_BIN,
        DISP_HEX,
        DISP_CHAR,
    };

    explicit DebugInterface(QWidget *parent = 0);
    ~DebugInterface();

    void setProjectView(ProjectView *prj) { projectView = prj; }
    void setDocumentArea(DocumentArea *da) { documentArea = da; }
    void fillInVars();

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


    void fillInStack();

    bool eventFilter(QObject *obj, QEvent *event);
    DispFormat findVarType(QString dataString);
    QString valueDisplay(long long value, DispFormat format);

signals:
    void gdbOutput(const QString& text);
    void gdbMessage(const QString& text);
    void applicationOutput(const QString& text);

private slots:
    void open(const QString& filename);
#if 1
    void onWatchWidgetCurrentItemChanged ( QTreeWidgetItem * current, int column );
    void onThreadWidgetSelectionChanged( );
    void onStackWidgetSelectionChanged();
    void onNext();
    void onStepIn();
    void onStepOut();
    void onStop();
    void onBreakpointsWidgetItemDoubleClicked(QTreeWidgetItem * item,int column);
    void onRun();
    void onContinue();
    void onWatchWidgetItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onAutoWidgetItemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_buttonStartDebug_clicked();
#else

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
#endif

private:
    typedef struct {
        QString orgValue;
        DispFormat orgFormat;
        DispFormat dispFormat;
    } DispInfo;
    struct Priv_t;

    QString m_filename; // Currently displayed file
    QString m_currentFile; //!< The file which the program counter points to.
    int m_currentLine; //!< The linenumber (first=1) which the program counter points to.
    QList<StackFrameEntry> m_stackFrameList;
    QMap<QString, DispInfo> m_watchVarDispInfo;
    QMap<QString, DispInfo> m_autoVarDispInfo;


    Ui::DebugInterface *ui;

    Priv_t *priv;
    ProjectView *projectView;
    DocumentArea *documentArea;
};

#endif // DEBUGINTERFACE_H
