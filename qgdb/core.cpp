#include "core.h"

#include "ini.h"
#include "util.h"
#include "log.h"

#include <QByteArray>
#include <QDebug>
#include <unistd.h>
#include <assert.h>
#include <signal.h>


Core::Core()
: m_inf(NULL)
,m_selectedThreadId(0)
,m_targetState(ICore::TARGET_STOPPED)
,m_lastTargetState(ICore::TARGET_FINISHED)
,m_pid(0)
,m_currentFrameIdx(-1)
 ,m_varWatchLastId(10)
{

	Com& com = Com::getInstance();
	com.setListener(this);

	m_ptsFd = getpt();

	if(grantpt(m_ptsFd))
		errorMsg("Failed to grantpt");
	if(unlockpt(m_ptsFd))
		errorMsg("Failed to unlock pt");
	infoMsg("Using: %s", ptsname(m_ptsFd));

	m_ptsListener = new QSocketNotifier(m_ptsFd, QSocketNotifier::Read);
	connect(m_ptsListener, SIGNAL(activated(int)), this, SLOT(onGdbOutput(int)));
}

Core::~Core()
{
	delete m_ptsListener;

	Com& com = Com::getInstance();
	com.setListener(NULL);

	close(m_ptsFd);
}

int Core::initLocal(Settings *cfg, QString gdbPath, QString programPath, QStringList argumentList)
{
	Com& com = Com::getInstance();
	Tree resultData;

	if(com.init(gdbPath)) {
		errorMsg("Failed to start gdb ('%s')", stringToCStr(gdbPath));
		return -1;
	}

	QString ptsDevPath = ptsname(m_ptsFd);
	com.commandF(&resultData, "-inferior-tty-set %s", stringToCStr(ptsDevPath));

	if(com.commandF(&resultData, "-file-exec-and-symbols %s", stringToCStr(programPath)) == GDB_ERROR)
		errorMsg("Failed to load '%s'", stringToCStr(programPath));

	QString commandStr;
	if(argumentList.size() > 0) {
		commandStr = "-exec-arguments ";
		for(int i = 0;i < argumentList.size();i++)
			commandStr += " " + argumentList[i];
		com.command(NULL, commandStr);
	}

	gdbSetBreakpointAtFunc("main");

	gdbGetFiles();

	// Run the initializing commands
	for(int i = 0;i < cfg->m_initCommands.size();i++) {
		QString cmd = cfg->m_initCommands[i];

		// Remove comments
		if(cmd.indexOf('#') != -1)
			cmd = cmd.left(cmd.indexOf('#'));
		cmd = cmd.trimmed();

		if(!cmd.isEmpty())
			com.commandF(NULL, "%s", stringToCStr(cmd));
	}

	gdbRun();

	return 0;
}

int Core::initRemote(Settings *cfg, QString gdbPath, QString programPath, QString tcpHost, int tcpPort)
{
	Com& com = Com::getInstance();
	Tree resultData;

	if(com.init(gdbPath)) {
		errorMsg("Failed to start gdb");
		return -1;
	}

	com.commandF(&resultData, "-target-select remote %s:%d", stringToCStr(tcpHost), tcpPort); 

	if(!programPath.isEmpty())
		com.commandF(&resultData, "-file-symbol-file %s", stringToCStr(programPath));

	// Run the initializing commands
	for(int i = 0;i < cfg->m_initCommands.size();i++) {
		QString cmd = cfg->m_initCommands[i];

		// Remove comments
		if(cmd.indexOf('#') != -1)
			cmd = cmd.left(cmd.indexOf('#'));
		cmd = cmd.trimmed();

		if(!cmd.isEmpty())
			com.commandF(NULL, "%s", stringToCStr(cmd));
	}

	if(!programPath.isEmpty()) {
		com.commandF(&resultData, "-file-exec-file %s", stringToCStr(programPath));
		com.commandF(&resultData, "-target-download");
	}

	gdbSetBreakpointAtFunc("main");
	gdbGetFiles();
	gdbContinue();

	return 0;
}


void Core::onGdbOutput(int socketFd)
{
	Q_UNUSED(socketFd);
	char buff[128];
	int n =  read(m_ptsFd, buff, sizeof(buff)-1);
	if(n > 0)
		buff[n] = '\0';
	m_inf->ICore_onTargetOutput(buff);
}

void Core::gdbGetFiles()
{
	Com& com = Com::getInstance();
	Tree resultData;
	QMap<QString, bool> fileLookup;

	com.command(&resultData, "-file-list-exec-source-files");

	for(int m = 0;m < m_sourceFiles.size();m++) {
		SourceFile *sourceFile = m_sourceFiles[m];
		delete sourceFile;
	}
	m_sourceFiles.clear();

	for(int k = 0; k < resultData.getRootChildCount(); k++) {
		TreeNode *rootNode = resultData.getChildAt(k);
		QString rootName = rootNode->getName();

		if(rootName == "files") {
			QStringList childList = resultData.getChildList("files");
			for(int j = 0;j < childList.size();j++) {
				QString treePath = "files/" + childList[j];
				QString name = resultData.getString(treePath + "/file");
				QString fullname = resultData.getString(treePath + "/fullname");

				SourceFile *sourceFile = NULL;
				// Already added this file?
				if(!fileLookup.contains(fullname) && name != "<built-in>") {
					fileLookup[fullname] = true;

					sourceFile = new SourceFile; 

					sourceFile->name =name;
					sourceFile->fullName = fullname;

					m_sourceFiles.append(sourceFile);
				}
			}
		}
	}
}

void Core::gdbSetBreakpointAtFunc(QString func)
{
	Com& com = Com::getInstance();
	Tree resultData;

	com.commandF(&resultData, "-break-insert %s", stringToCStr(func));
}

void Core::gdbRun()
{
	Com& com = Com::getInstance();
	Tree resultData;

	if(m_targetState == ICore::TARGET_RUNNING) {
		if(m_inf)
			m_inf->ICore_onMessage("Program is currently running");
		return;
	}

	m_pid = 0;
	com.commandF(&resultData, "-exec-run");
}

/**
 * @brief  Resumes the execution until a breakpoint is encountered, or until the program exits.
 */
void Core::gdbContinue()
{
	Com& com = Com::getInstance();
	Tree resultData;

	if(m_targetState == ICore::TARGET_RUNNING) {
		if(m_inf)
			m_inf->ICore_onMessage("Program is currently running");
		return;
	}

	com.commandF(&resultData, "-exec-continue");
}

void Core::stop()
{
	if(m_targetState != ICore::TARGET_RUNNING) {
		if(m_inf)
			m_inf->ICore_onMessage("Program is not running");
		return;
	}

	if(m_pid != 0)
		kill(m_pid, SIGINT);
	else
		errorMsg("Failed to stop since PID not known");
}

void Core::gdbNext()
{
	Com& com = Com::getInstance();
	Tree resultData;

	if(m_targetState != ICore::TARGET_STOPPED) {
		if(m_inf)
			m_inf->ICore_onMessage("Program is not stopped");
		return;
	}

	com.commandF(&resultData, "-exec-next");
}

void Core::getStackFrames()
{
	Com& com = Com::getInstance();
	Tree resultData;
	com.command(&resultData, "-stack-list-frames");
}

void Core::gdbStepIn()
{
	Com& com = Com::getInstance();
	Tree resultData;

	if(m_targetState != ICore::TARGET_STOPPED) {
		if(m_inf)
			m_inf->ICore_onMessage("Program is not stopped");
		return;
	}

	com.commandF(&resultData, "-exec-step");
	com.commandF(&resultData, "-var-update --all-values *");
}

void Core::gdbStepOut()
{
	Com& com = Com::getInstance();
	Tree resultData;

	if(m_targetState != ICore::TARGET_STOPPED) {
		if(m_inf)
			m_inf->ICore_onMessage("Program is not stopped");
		return;
	}

	com.commandF(&resultData, "-exec-finish");
	com.commandF(&resultData, "-var-update --all-values *");
}

Core& Core::getInstance()
{
	static Core core;
	return core;
}

int Core::gdbAddVarWatch(QString varName, QString *varType, QString *value, int *watchId_)
{
	Com& com = Com::getInstance();
	Tree resultData;
	int watchId = m_varWatchLastId++;
	GdbResult res;
	int rc = 0;

	assert(varName.isEmpty() == false);

	res = com.commandF(&resultData, "-var-create w%d * %s", watchId, stringToCStr(varName));
	if(res != GDB_ERROR) {
		QString varName2 = resultData.getString("name");
		QString varValue2 = resultData.getString("value");
		QString varType2 = resultData.getString("type");


		VarWatch w;
		w.name = varName;
		w.id = watchId;
		m_watchList[watchId] = w;

		*varType = varType2;
		*value = varValue2;
	} else
		rc = -1;

	*watchId_ = watchId;

	return rc;
}

QString Core::gdbGetVarWatchName(int watchId)
{
	return m_watchList[watchId].name;
}

void Core::gdbRemoveVarWatch(int watchId)
{
	Com& com = Com::getInstance();
	Tree resultData;

	assert(watchId != -1);


	QMap <int, VarWatch>::iterator pos = m_watchList.find(watchId);
	if(pos == m_watchList.end())
		assert(0);
	else {
		m_watchList.erase(pos);
		com.commandF(&resultData, "-var-delete w%d", watchId);
	}
}


void Core::onNotifyAsyncOut(Tree &tree, AsyncClass ac)
{
	debugMsg("NotifyAsyncOut> %s", Com::asyncClassToString(ac));

	if(ac == ComListener::AC_BREAKPOINT_MODIFIED) {
		for(int i = 0;i < tree.getRootChildCount();i++) {
			TreeNode *rootNode = tree.getChildAt(i);
			QString rootName = rootNode->getName();

			if(rootName == "bkpt")
				dispatchBreakpointTree(tree);
		}
	} else if (ac == ComListener::AC_BREAKPOINT_CREATED) {
		for(int i = 0;i < tree.getRootChildCount();i++) {
			TreeNode *rootNode = tree.getChildAt(i);
			QString rootName = rootNode->getName();

			if(rootName == "bkpt") {
				dispatchBreakpointTree(tree);
				tree.dump();
			}
		}
	} else if(ac == ComListener::AC_THREAD_CREATED) {
		// A new thread has been created
		gdbGetThreadList();
	}

	tree.dump();
}

ICore::StopReason Core::parseReasonString(QString reasonString)
{
	if(reasonString == "breakpoint-hit")
		return ICore::BREAKPOINT_HIT;
	if(reasonString == "end-stepping-range")
		return ICore::END_STEPPING_RANGE;
	if(reasonString == "signal-received" || reasonString == "exited-signalled")
		return ICore::SIGNAL_RECEIVED;
	if(reasonString == "exited-normally")
		return ICore::EXITED_NORMALLY;
	if(reasonString == "function-finished")
		return ICore::FUNCTION_FINISHED;
	if(reasonString == "exited")
		return ICore::EXITED;

	errorMsg("Received unknown reason (\"%s\").", stringToCStr(reasonString));
	assert(0);

	return ICore::UNKNOWN;
}

void Core::onExecAsyncOut(Tree &tree, AsyncClass ac)
{
	Com& com = Com::getInstance();

	debugMsg("ExecAsyncOut> %s", Com::asyncClassToString(ac));

	//tree.dump();

	// The program has stopped
	if(ac == ComListener::AC_STOPPED) {
		m_targetState = ICore::TARGET_STOPPED;

		if(m_pid == 0)
			com.command(NULL, "-list-thread-groups");

		// Any new or destroyed thread?
		com.commandF(NULL, "-thread-info");

		com.commandF(NULL, "-var-update --all-values *");
		com.commandF(NULL, "-stack-list-locals 1");

		QString p = tree.getString("frame/fullname");
		int lineNo = tree.getInt("frame/line");

		// Get the reason
		QString reasonString = tree.getString("reason");
		ICore::StopReason  reason;
		if(reasonString.isEmpty())
			reason = ICore::UNKNOWN;
		else
			reason = parseReasonString(reasonString);

		if(reason == ICore::EXITED_NORMALLY)
			m_targetState = ICore::TARGET_FINISHED;

		if(m_inf) {
			if(reason == ICore::SIGNAL_RECEIVED) {
				QString signalName = tree.getString("signal-name");
				if(signalName == "SIGSEGV")
					m_targetState = ICore::TARGET_FINISHED;
				m_inf->ICore_onSignalReceived(signalName);  
			} else
				m_inf->ICore_onStopped(reason, p, lineNo);

			m_inf->ICore_onFrameVarReset();

			QStringList childList = tree.getChildList("frame/args");
			for(int i = 0;i < childList.size();i++) {
				QString treePath = "frame/args/" + childList[i];
				QString varName = tree.getString(treePath + "/name");
				QString varValue = tree.getString(treePath + "/value");
				if(m_inf)
					m_inf->ICore_onFrameVarChanged(varName, varValue);
			}

			int frameIdx = tree.getInt("frame/level");
			m_currentFrameIdx = frameIdx;
			m_inf->ICore_onCurrentFrameChanged(frameIdx);
		}
	}
	else if(ac == ComListener::AC_RUNNING) {
		m_targetState = ICore::TARGET_RUNNING;

		debugMsg("is running\n");
	}

	// Get the current thread
	QString threadIdStr = tree.getString("thread-id");
	if(threadIdStr.isEmpty() == false) {
		int threadId = threadIdStr.toInt();
		if(m_inf)
			m_inf->ICore_onCurrentThreadChanged(threadId);
	}

	// State changed?
	if(m_inf && m_lastTargetState != m_targetState) {
		m_inf->ICore_onStateChanged(m_targetState);
		m_lastTargetState = m_targetState;
	}
}


void Core::gdbRemoveBreakpoint(BreakPoint* bkpt)
{
	Com& com = Com::getInstance();
	Tree resultData;

	assert(bkpt != NULL);

	com.commandF(&resultData, "-break-delete %d", bkpt->m_number);    

	m_breakpoints.removeOne(bkpt);

	if(m_inf)
		m_inf->ICore_onBreakpointsChanged();
	delete bkpt;
}

void Core::gdbGetThreadList()
{
	Com& com = Com::getInstance();
	Tree resultData;

	if(m_targetState == ICore::TARGET_RUNNING) {
		if(m_inf)
			m_inf->ICore_onMessage("Program is currently running");
		return;
	}

	com.commandF(&resultData, "-thread-info");    
}


BreakPoint* Core::findBreakPoint(QString fullPath, int lineNo) {
	for(int i = 0;i < m_breakpoints.size();i++) {
		BreakPoint *bkpt = m_breakpoints[i];

		if(bkpt->lineNo == lineNo && fullPath == bkpt->fullname)
			return bkpt;
	}

	return NULL;
}


BreakPoint* Core::findBreakPointByNumber(int number)
{
	for(int i = 0;i < m_breakpoints.size();i++) {
		BreakPoint *bkpt = m_breakpoints[i];

		if(bkpt->m_number == number)
			return bkpt;
	}

	return NULL;
}

void Core::dispatchBreakpointTree(Tree &tree)
{
	int lineNo = tree.getInt("bkpt/line");
	int number = tree.getInt("bkpt/number");

	BreakPoint *bkpt = findBreakPointByNumber(number);
	if(bkpt == NULL) {
		bkpt = new BreakPoint(number);
		m_breakpoints.push_back(bkpt);
	}
	bkpt->lineNo = lineNo;
	bkpt->fullname = tree.getString("bkpt/fullname");
	bkpt->m_funcName = tree.getString("bkpt/func");
	bkpt->m_addr = tree.getLongLong("bkpt/addr");

	if(m_inf)
		m_inf->ICore_onBreakpointsChanged();
}

void Core::onResult(Tree &tree)
{
	debugMsg("Result>");

	for(int i = 0;i < tree.getRootChildCount();i++) {
		TreeNode *rootNode = tree.getChildAt(i);
		QString rootName = rootNode->getName();
		if(rootName == "changelist") {
			debugMsg("Changelist");
			for(int j = 0;j < rootNode->getChildCount();j++) {
				QString path;
				path.sprintf("changelist/%d/name", j+1);
				int watchId = tree.getString(path).mid(1).toInt();
				path.sprintf("changelist/%d/value", j+1);
				QString varValue = tree.getString(path);

				if(!m_watchList.contains(watchId)) {
					assert(!m_watchList.contains(watchId));
				} else {
					VarWatch &w = m_watchList[watchId];

					if(m_inf)
						m_inf->ICore_onWatchVarChanged(watchId, w.name, varValue);
				}
			}
		} else if(rootName == "bkpt") {
			dispatchBreakpointTree(tree);

		} else if(rootName == "threads") {
			m_threadList.clear();

			// Parse the result
			QStringList childList = tree.getChildList("threads");
			for(int i = 0;i < childList.size();i++) {
				QString treePath = "threads/" + childList[i];
				QString threadId = tree.getString(treePath + "/id");
				QString targetId = tree.getString(treePath + "/target-id");
				QString funcName = tree.getString(treePath + "/frame/func");


				ThreadInfo tinfo;
				tinfo.id = atoi(stringToCStr(threadId));
				tinfo.m_name = targetId;
				tinfo.m_func = funcName;
				m_threadList[tinfo.id] = tinfo;
			}

			if(m_inf)
				m_inf->ICore_onThreadListChanged();
		} else if(rootName == "current-thread-id") {
			// Get the current thread
			QString threadIdStr = tree.getString("current-thread-id");
			if(threadIdStr.isEmpty() == false) {
				int threadId = threadIdStr.toInt();
				if(m_inf)
					m_inf->ICore_onCurrentThreadChanged(threadId);
			}
		} else if(rootName == "frame") {
			QString p = tree.getString("frame/fullname");
			int lineNo = tree.getInt("frame/line");
			int frameIdx = tree.getInt("frame/level");
			ICore::StopReason  reason = ICore::UNKNOWN;

			m_currentFrameIdx = frameIdx;

			if(m_inf) {
				m_inf->ICore_onStopped(reason, p, lineNo);

				m_inf->ICore_onFrameVarReset();

				QStringList childList = tree.getChildList("frame/args");
				for(int i = 0;i < childList.size();i++) {
					QString treePath = "frame/args/" + childList[i];
					QString varName = tree.getString(treePath + "/name");
					QString varValue = tree.getString(treePath + "/value");
					if(m_inf)
						m_inf->ICore_onFrameVarChanged(varName, varValue);
				}
			}
		} else if(rootName == "stack") {
			// A stack frame dump?
			int cnt = tree.getChildCount("stack");
			QList<StackFrameEntry> stackFrameList;
			for(int j = 0;j < cnt;j++) {
				QString path;
				path.sprintf("stack/#%d/func", j+1);


				StackFrameEntry entry;
				path.sprintf("stack/#%d/func", j+1);
				entry.m_functionName = tree.getString(path);
				path.sprintf("stack/#%d/line", j+1);
				entry.m_line = tree.getInt(path);
				path.sprintf("stack/#%d/fullname", j+1);
				entry.m_sourcePath = tree.getString(path);
				stackFrameList.push_front(entry);
			}
			if(m_inf) {
				m_inf->ICore_onStackFrameChange(stackFrameList);
				m_inf->ICore_onCurrentFrameChanged(m_currentFrameIdx);
			}
		}
		else if(rootName == "locals") {
			// Local variables?
			if(m_inf) {
				m_inf->ICore_onLocalVarReset();

				int cnt = tree.getChildCount("locals");
				for(int j = 0;j < cnt;j++) {
					QString path;
					path.sprintf("locals/%d/name", j+1);
					QString varName = tree.getString(path);
					path.sprintf("locals/%d/value", j+1);
					QString varData = tree.getString(path);

					m_inf->ICore_onLocalVarChanged(varName, varData);
				}
			}
		} else if(rootName == "msg") {
			QString message = tree.getString("msg");
			if(m_inf)
				m_inf->ICore_onMessage(message);

		} else if(rootName == "groups") {
			if(m_pid == 0)
				m_pid = tree.getInt("groups/1/pid");
		}
	}

	tree.dump();
}

void Core::onStatusAsyncOut(Tree &tree, AsyncClass ac)
{
	infoMsg("StatusAsyncOut> %s", Com::asyncClassToString(ac));
	tree.dump();
}

void Core::onConsoleStreamOutput(QString str)
{
	QStringList list = str.split('\n');
	for(int i = 0; i<list.size(); i++) {
		QString text = list[i];
		if(text.isEmpty() && i+1 == list.size())
			continue;

		debugMsg("GDB | Console-stream | %s", stringToCStr(text));

		if(m_inf)
			m_inf->ICore_onConsoleStream(text);
	}
}


void Core::onTargetStreamOutput(QString str)
{
	QStringList list = str.split('\n');
	for(int i = 0;i < list.size();i++)
		infoMsg("GDB | Target-stream | %s", stringToCStr(list[i]));
}


void Core::onLogStreamOutput(QString str)
{
	QStringList list = str.split('\n');
	for(int i = 0;i < list.size();i++)
		infoMsg("GDB | Log-stream | %s", stringToCStr(list[i]));
}

void Core::gdbSetBreakpoint(QString filename, int lineNo)
{
	Com& com = Com::getInstance();
	Tree resultData;

	assert(filename != "");

	com.commandF(&resultData, "-break-insert %s:%d", stringToCStr(filename), lineNo);

}

QList<ThreadInfo> Core::getThreadList()
{
	return m_threadList.values();
}

void Core::selectThread(int threadId)
{
	if(m_selectedThreadId == threadId)
		return;

	Com& com = Com::getInstance();
	Tree resultData;

	com.commandF(&resultData, "-thread-select %d", threadId);

	m_selectedThreadId = threadId;
}

/**
 * @brief Selects a specific frame
 * @param selectedFrameIdx    The frame to select as active (0=newest frame).
 */
void Core::selectFrame(int selectedFrameIdx)
{
	Com& com = Com::getInstance();
	Tree resultData;

	if(m_targetState == ICore::TARGET_RUNNING)
		return;
	if(m_currentFrameIdx != selectedFrameIdx) {
		com.commandF(NULL, "-stack-select-frame %d", selectedFrameIdx);
		com.commandF(&resultData, "-stack-info-frame");
		com.commandF(NULL, "-stack-list-locals 1");
	}
}

void Core::sendRawCommand(const char *line)
{
	Com &com = Com::getInstance();

	com.commandF(NULL, (char*)line);
}
