#ifndef FILE__CORE_H
#define FILE__CORE_H

#include "com.h"
#include <QList>
#include <QMap>
#include <QSocketNotifier>
#include <QObject>

#include "settings.h"

struct ThreadInfo {
	int id;
	QString m_name;

	QString m_func;
};

Q_DECLARE_METATYPE(ThreadInfo)

struct StackFrameEntry
{
public:
	QString m_functionName; //!< Eg: "main".
	int m_line; //!< Eg: 1.
	QString m_sourcePath; //!< Eg: "/test/file.c".
};


class SourceFile {
public:
	QString name;
	QString fullName;
};

class BreakPoint {
public:
    BreakPoint(int number) : m_number(number) { }

	int m_number;
	QString fullname;
	int lineNo;
	QString m_funcName;
	unsigned long long m_addr;

private:
    BreakPoint() { }
};

class ICore {
public:

	enum TargetState {
		TARGET_STOPPED, 
		TARGET_RUNNING,
		TARGET_FINISHED 
	}; 

	enum StopReason {
		UNKNOWN,
		END_STEPPING_RANGE,
		BREAKPOINT_HIT,
		SIGNAL_RECEIVED,
		EXITED_NORMALLY,
		FUNCTION_FINISHED,
		EXITED
	};

	enum SignalType {
		SIGINT,
		SIGTERM,
		SIGKILL,
		SIGUNKNOWN
	};
	virtual void ICore_onStopped(StopReason reason, QString path, int lineNo) = 0;
	virtual void ICore_onStateChanged(TargetState state) = 0;
	virtual void ICore_onSignalReceived(QString signalName) = 0;
	virtual void ICore_onLocalVarReset() = 0;
	virtual void ICore_onLocalVarChanged(QString name, QString value) = 0;
	virtual void ICore_onFrameVarReset() = 0;
	virtual void ICore_onFrameVarChanged(QString name, QString value) = 0;
	virtual void ICore_onWatchVarChanged(int watchId, QString name, QString value) = 0;
	virtual void ICore_onConsoleStream(QString text) = 0;
	virtual void ICore_onBreakpointsChanged() = 0;
	virtual void ICore_onThreadListChanged() = 0;
	virtual void ICore_onCurrentThreadChanged(int threadId) = 0;
	virtual void ICore_onStackFrameChange(QList<StackFrameEntry> stackFrameList) = 0;
	virtual void ICore_onMessage(QString message) = 0;
	virtual void ICore_onTargetOutput(QString message) = 0;
	virtual void ICore_onCurrentFrameChanged(int frameIdx) = 0;

};

struct VarWatch {
	QString name;
	int id;
};

class Core : public ComListener {
private:
    Q_OBJECT

private:

	Core();
	~Core();

public:
	static Core& getInstance();
	int initLocal(Settings *cfg, QString gdbPath, QString programPath, QStringList argumentList);
	int initRemote(Settings *cfg, QString gdbPath, QString programPath, QString tcpHost, int tcpPort);

    void setListener(ICore *inf) { m_inf = inf; }

private:
	void onNotifyAsyncOut(Tree &tree, AsyncClass ac);
	void onExecAsyncOut(Tree &tree, AsyncClass ac);
	void onResult(Tree &tree);
	void onStatusAsyncOut(Tree &tree, AsyncClass ac);
	void onConsoleStreamOutput(QString str);
	void onTargetStreamOutput(QString str);
	void onLogStreamOutput(QString str);

	void dispatchBreakpointTree(Tree &tree);
	static ICore::StopReason parseReasonString(QString string);

public:
	void gdbSetBreakpointAtFunc(QString func);
	void gdbNext();
	void gdbStepIn();
	void gdbStepOut();
	void gdbContinue();
	void gdbRun();
	void gdbGetFiles();
	int gdbAddVarWatch(QString varName, QString *varType, QString *value, int *watchId);
	void gdbRemoveVarWatch(int vatchId);
	QString gdbGetVarWatchName(int vatchId);
	void gdbSetBreakpoint(QString filename, int lineNo);
	void gdbGetThreadList();
	void getStackFrames();
	void sendRawCommand(const char *);
	void stop();

	void selectThread(int threadId);
	void selectFrame(int selectedFrameIdx);

	// Breakpoints
    QList<BreakPoint*> getBreakPoints() { return m_breakpoints; }
	BreakPoint* findBreakPoint(QString fullPath, int lineNo);
	BreakPoint* findBreakPointByNumber(int number);
	void gdbRemoveBreakpoint(BreakPoint* bkpt);

	QList<ThreadInfo> getThreadList();
    QVector <SourceFile*> getSourceFiles() { return m_sourceFiles; }

private slots:
	void onGdbOutput(int socketNr);

private:
	ICore *m_inf;
	QList<BreakPoint*> m_breakpoints;
	QVector <SourceFile*> m_sourceFiles;
	QMap <int, ThreadInfo> m_threadList;
	int m_selectedThreadId;
	ICore::TargetState m_targetState;
	ICore::TargetState m_lastTargetState;
	int m_pid;
	int m_currentFrameIdx;
	QMap <int, VarWatch> m_watchList;
	int m_varWatchLastId;

    // int m_ptsFd;
    // QSocketNotifier  *m_ptsListener;

};

#endif // FILE__CORE_H
