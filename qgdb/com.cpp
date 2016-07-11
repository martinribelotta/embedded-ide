#include "com.h"
#include <QByteArray>
#include <QDebug>
#include <unistd.h>
#include "log.h"
#include "util.h"
#include <assert.h>
#include <unistd.h>
#include "config.h"

const char* Com::asyncClassToString(ComListener::AsyncClass ac)
{
	const char *str = "?";
	switch(ac) {
		case ComListener::AC_STOPPED: str = "stopped"; break;
		case ComListener::AC_RUNNING: str = "running"; break;
		case ComListener::AC_THREAD_CREATED: str = "thread_created"; break;
		case ComListener::AC_THREAD_GROUP_ADDED: str = "thread_group_added"; break;
		case ComListener::AC_THREAD_GROUP_STARTED: str = "thread_group_started"; break;
		case ComListener::AC_LIBRARY_LOADED: str = "library_loaded"; break;
		case ComListener::AC_BREAKPOINT_CREATED: str = " breakpoint_created";break;
		case ComListener::AC_BREAKPOINT_MODIFIED: str = " breakpoint_modified";break;
		case ComListener::AC_THREAD_EXITED: str = "thread_exited"; break;
		case ComListener::AC_THREAD_GROUP_EXITED: str = "thread_group_exited"; break;
		case ComListener::AC_LIBRARY_UNLOADED: str = "library_unloaded"; break;
		case ComListener::AC_THREAD_SELECTED: str = "thread_selected"; break;
		case ComListener::AC_DOWNLOAD: str = "download"; break;
	}
	return str;
}

const char *Token::toString()
{
	strcpy(m_tmpBuff, (const char*)stringToCStr(text));

	return m_tmpBuff;
};

const char *Token::typeToString(Type type)
{
	const char *str = "?";
	switch(type) {
		case UNKNOWN: str = "unknown"; break;
		case C_STRING:str = "c_string"; break;
		case KEY_EQUAL:str = "="; break;
		case KEY_LEFT_BRACE:str = "{"; break;
		case KEY_RIGHT_BRACE:str = "}"; break;
		case KEY_LEFT_BAR:str = "["; break;
		case KEY_RIGHT_BAR:str = "]"; break;
		case KEY_UP:str = "^"; break;
		case KEY_PLUS:str = "+"; break;
		case KEY_COMMA:str = ","; break;
		case KEY_TILDE:str = "~"; break;
		case KEY_SNABEL:str = "@"; break;
		case KEY_STAR:str = "*"; break;
		case KEY_AND:str = "&"; break;
		case END_CODE: str = "endcode"; break;
		case VAR: str = "string"; break;
	}
	return str;
}



bool Resp::isResult()
{
	return (m_type == RESULT) ? true : false;
}


Com::Com()
: m_listener(NULL)
#ifdef ENABLE_GDB_LOG
,m_logFile(GDB_LOG_FILE)
#endif
,m_busy(0)
{
#ifdef ENABLE_GDB_LOG
	m_logFile.open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text);
#endif

	connect(&m_process, SIGNAL(readyReadStandardOutput ()), this, SLOT(onReadyReadStandardOutput()));
}

Com::~Com()
{
	// Send the command to gdb to exit cleanly
	QString text = "-gdb-exit\n";
	m_process.write((const char*)text.toLatin1());

	m_process.terminate();
	m_process.waitForFinished();

	// Free tokens
	while(!m_freeTokens.isEmpty()) {
		Token *token = m_freeTokens.takeFirst();
		delete token;
	}
}

GdbResult Com::commandF(Tree *resultData, const char *cmdFmt, ...)
{
	va_list ap;
	char buffer[1024];

	va_start(ap, cmdFmt);
	vsnprintf(buffer, sizeof(buffer), cmdFmt, ap);

	GdbResult res = command(resultData, buffer);

	va_end(ap);

	return res;
}

/**
 * @brief Creates tokens from a single GDB output row.
 */
QList<Token*> Com::tokenize(QString str)
{
	enum { IDLE, END_CODE, STRING, VAR} state = IDLE;
	QList<Token*> list;
	Token *cur = NULL;
	QChar prevC = ' ';

	if(str.isEmpty())
		return list;

	for(int i = 0;i < str.size();i++) {
		QChar c = str[i];
		switch(state) {
			case IDLE:
				if(c == '"') {
					cur = new Token;
					list.push_back(cur);
					cur->type = Token::C_STRING;
					state = STRING;
				} else if(c == '(') {
					cur = new Token;
					list.push_back(cur);
					cur->type = Token::END_CODE;
					cur->text += c;
					state = END_CODE;
				} else if(c == '=' || c == '{' || c == '}' || c == ',' ||
						c == '[' || c == ']' || c == '+' || c == '^' ||
						c == '~' || c == '@' || c == '&' || c == '*') {
					cur = new Token;
					list.push_back(cur);
					cur->text += c;
					cur->type = Token::UNKNOWN;
					if(c == '=')
						cur->type = Token::KEY_EQUAL;
					if(c == '{')
						cur->type = Token::KEY_LEFT_BRACE;
					if(c == '}')
						cur->type = Token::KEY_RIGHT_BRACE;
					if(c == '[')
						cur->type = Token::KEY_LEFT_BAR;
					if(c == ']')
						cur->type = Token::KEY_RIGHT_BAR;
					if(c == ',')
						cur->type = Token::KEY_COMMA;
					if(c == '^')
						cur->type = Token::KEY_UP;
					if(c == '+')
						cur->type = Token::KEY_PLUS;
					if(c == '~')
						cur->type = Token::KEY_TILDE;
					if(c == '@')
						cur->type = Token::KEY_SNABEL;
					if(c == '&')
						cur->type = Token::KEY_AND;
					if(c == '*')
						cur->type = Token::KEY_STAR;
					state = IDLE;
				} else if( c != ' ') {
					cur = new Token;
					list.push_back(cur);
					cur->type = Token::VAR;
					cur->text = c;
					state = VAR;
				}
				break;
			case END_CODE:
				{
					QString codeEndStr = "(gdb)";
					cur->text += c;

					if(cur->text.length() == codeEndStr.length()) {
						state = IDLE;
					} else if(cur->text.compare(codeEndStr.left(cur->text.length())) != 0) {
						cur->type = Token::VAR;
						state = IDLE;
					}
				};
				break;
			case STRING:
				{
					if(prevC != '\\' && c == '\\') {}
					else if(prevC == '\\')
						cur->text += (c == 'n') ? '\n' : c;
					else if(c == '"')
						state = IDLE;
					else
						cur->text += c;
				};
				break;
			case VAR:
				{
					if(c == '=' || c == ',' || c == '{' || c == '}') {
						i--;
						cur->text = cur->text.trimmed();
						state = IDLE;
					} else
						cur->text += c;
				};
				break;
		}
		prevC = c;
	}
	if(cur && cur->type == Token::VAR)
		cur->text = cur->text.trimmed();

	return list;
}

Token* Com::pop_token()
{
	if(m_list.empty())
		return NULL;

	Token *tok = m_list.takeFirst();
	m_freeTokens += tok;    

	return tok;
}


Token* Com::peek_token()
{
	readTokens();

	if(m_list.empty())
		return NULL;

	return m_list.first();
}


/**
 * @brief Parses 'ASYNC-OUTPUT'
 * @return 0 on success
 */
int Com::parseAsyncOutput(Resp *resp, ComListener::AsyncClass *ac)
{
	Token *tokVar;
	int rc = 0;

	// Get the class
	tokVar = eatToken(Token::VAR);
	if(tokVar == NULL)
		return -1;

	QString acString = tokVar->getString();

	if(acString == "stopped")
		*ac = ComListener::AC_STOPPED;
	else if(acString == "running")
		*ac = ComListener::AC_RUNNING;
	else if(acString == "thread-created")
		*ac = ComListener::AC_THREAD_CREATED;
	else if(acString == "thread-group-added")
		*ac = ComListener::AC_THREAD_GROUP_ADDED;
	else if(acString == "thread-group-started")
		*ac = ComListener::AC_THREAD_GROUP_STARTED;
	else if(acString == "library-loaded")
		*ac = ComListener::AC_LIBRARY_LOADED;
	else if(acString == "breakpoint-modified")
		*ac = ComListener::AC_BREAKPOINT_MODIFIED;
	else if (acString == "breakpoint-created")
		*ac = ComListener::AC_BREAKPOINT_CREATED;
	else if(acString == "thread-exited")
		*ac = ComListener::AC_THREAD_EXITED;
	else if(acString == "thread-group-exited")
		*ac = ComListener::AC_THREAD_GROUP_EXITED;
	else if(acString == "library-unloaded")
		*ac = ComListener::AC_LIBRARY_UNLOADED;
	else if(acString == "thread-selected")
		*ac = ComListener::AC_THREAD_SELECTED;
	else if(acString == "download")
		*ac = ComListener::AC_DOWNLOAD;
	else {
		errorMsg("Unexpected response '%s'", stringToCStr(acString));
		//assert(0);
		rc = -1;
	}

	while(checkToken(Token::KEY_COMMA))
		parseResult(resp->tree.getRoot());

	return rc;
}

Resp* Com::parseExecAsyncOutput()
{
	Resp *resp = NULL;

	// Parse 'token'
	checkToken(Token::VAR);

	if(checkToken(Token::KEY_STAR) == NULL)
		return NULL;

	resp = new Resp;
	resp->setType(Resp::EXEC_ASYNC_OUTPUT);

	parseAsyncOutput(resp, &resp->reason);

	return resp;
}

Resp *Com::parseStatusAsyncOutput()
{
	Resp *resp = NULL;

	// Parse 'token'
	checkToken(Token::VAR);

	if(checkToken(Token::KEY_PLUS) == NULL)
		return NULL;

	resp = new Resp;
	resp->setType(Resp::STATUS_ASYNC_OUTPUT);

	parseAsyncOutput(resp, &resp->reason);
	return resp;
}


Resp *Com::parseNotifyAsyncOutput()
{
	Resp *resp = NULL;

	// Parse 'token'
	checkToken(Token::VAR);

	if(checkToken(Token::KEY_EQUAL) == NULL)
		return NULL;

	resp = new Resp;
	resp->setType(Resp::NOTIFY_ASYNC_OUTPUT);

	parseAsyncOutput(resp, &resp->reason);
	return resp;
}

Resp *Com::parseAsyncRecord()
{
	Resp *resp = NULL;
	if(isTokenPending() && resp == NULL)
		resp = parseExecAsyncOutput();
	if(isTokenPending() && resp == NULL)
		resp = parseStatusAsyncOutput();
	if(isTokenPending() && resp == NULL)
		resp = parseNotifyAsyncOutput();
	return resp;
}

Resp *Com::parseStreamRecord()
{
	Resp *resp = NULL;
	Token *tok;
	if(checkToken(Token::KEY_TILDE)) {
		resp = new Resp;
		tok = eatToken(Token::C_STRING);

		resp->setType(Resp::CONSOLE_STREAM_OUTPUT);
		resp->setString(tok->getString());

	} else if(checkToken(Token::KEY_SNABEL)) {
		resp = new Resp;
		tok = eatToken(Token::C_STRING);

		resp->setType(Resp::TARGET_STREAM_OUTPUT);
		resp->setString(tok->getString());

	} else if(checkToken(Token::KEY_AND)) {
		resp = new Resp;
		tok = eatToken(Token::C_STRING);

		resp->setType(Resp::LOG_STREAM_OUTPUT);
		resp->setString(tok->getString());
	}

	return resp;
}

Token* Com::eatToken(Token::Type type)
{
	Token *tok = peek_token();
	while(tok == NULL) {
		m_process.waitForReadyRead(100);
		readTokens();
		tok = peek_token();
	}

	if(tok == NULL || tok->getType() != type) {
		errorMsg("Expected '%s' but got '%s'",
				Token::typeToString(type), tok ? stringToCStr(tok->getString()) : "<NULL>");
		return NULL;
	}

	pop_token();
	return tok;
}

/**
 * @brief Checks if the read queue is empty.
 */
bool Com::isTokenPending()
{
	Token *tok = peek_token();

	return (tok != NULL);
}

/**
 * @brief Checks and pops a token if the kind is as expected.
 * @return The found token or NULL if no hit.
 */
Token* Com::checkToken(Token::Type type)
{
	Token *tok = peek_token();
	if(tok == NULL)
		readTokens();

	if(tok == NULL || tok->getType() != type)
		return NULL;

	pop_token();

	return tok;
}

/**
 * @brief Parses 'VALUE'
 * @return 0 on success.
 */
int Com::parseValue(TreeNode *item)
{
	Token *tok;

	tok = pop_token();

	// Const?
	if(tok->getType() == Token::C_STRING)
		item->setData(tok->getString());
	// Tuple?
	else if(tok->getType() == Token::KEY_LEFT_BRACE) {
		do {
			parseResult(item);
		} while(checkToken(Token::KEY_COMMA) != NULL);

		if(eatToken(Token::KEY_RIGHT_BRACE) == NULL)
			return -1;
	}
	// List?
	else if(tok->getType() == Token::KEY_LEFT_BAR) {
		if(checkToken(Token::KEY_RIGHT_BAR) != NULL)
			return 0;

		tok = peek_token();
		if(tok->getType() == Token::VAR) {
			do {
				parseResult(item);
			} while(checkToken(Token::KEY_COMMA) != NULL);
		} else {
			int idx = 1;
			QString name;

			do {
				TreeNode *node = new TreeNode;
				name.sprintf("%d", idx++);
				node->setName(name);
				item->addChild(node); 
				parseValue(node);
			} while(checkToken(Token::KEY_COMMA) != NULL);
		}

		if(eatToken(Token::KEY_RIGHT_BAR) == NULL)
			return -1;
	}
	else
		errorMsg("Unexpected token: '%s'", stringToCStr(tok->getString()));

	return 0;
}

/**
 * @brief Parses 'RESULT'
 * @return 0 on success.
 */
int Com::parseResult(TreeNode *parent)
{
	TreeNode *item = new TreeNode;
	parent->addChild(item);

	Token *tok = peek_token();
	if(tok != NULL && tok->getType() == Token::KEY_LEFT_BRACE)
		parseValue(item);
	else {
		Token *tokVar = eatToken(Token::VAR);
		if(tokVar == NULL)
			return -1;

		item->setName(tokVar->getString());

		if(eatToken(Token::KEY_EQUAL) == NULL)
			return -1;

		parseValue(item);
	}

	return 0;
}

Resp *Com::parseResultRecord()
{
	Resp *resp = NULL;
	int rc = 0;

	// Parse 'token'
	checkToken(Token::VAR);

	// Parse '^'
	if(checkToken(Token::KEY_UP) == NULL)
		return NULL;

	// Parse 'result class'
	Token *tok = eatToken(Token::VAR);
	if(tok == NULL)
		return NULL;

	resp = new Resp;
	QString resultClass = tok->getString();
	GdbResult res;
	if(resultClass == "done")
		res = GDB_DONE;
	else if(resultClass == "running")
		res = GDB_RUNNING;
	else if(resultClass == "connected")
		res = GDB_CONNECTED;
	else if(resultClass == "error")
		res = GDB_ERROR;
	else if(resultClass == "exit")
		res = GDB_EXIT;
	else {
		delete resp;
		errorMsg("Invalid result class found: %s", stringToCStr(resultClass));
		return NULL;
	}
	resp->m_result = res;

	while(checkToken(Token::KEY_COMMA) != NULL && rc == 0)
		rc = parseResult(resp->tree.getRoot());

	assert(m_pending.isEmpty() == false);
	if(!m_pending.isEmpty()) {
		PendingCommand cmd = m_pending.takeFirst();

		debugMsg("%s done", stringToCStr(cmd.m_cmdText));
	}

	resp->setType(Resp::RESULT);

	return resp;
}

Resp* Com::parseOutOfBandRecord()
{
	Resp *resp = NULL;

	if(isTokenPending() && resp == NULL)
		resp = parseAsyncRecord();
	if(isTokenPending() && resp == NULL)
		resp = parseStreamRecord();

	return resp;
}

Resp *Com::parseOutput()
{
	Resp *resp = NULL;

	if(isTokenPending())
		resp = parseOutOfBandRecord();

	if(isTokenPending() && resp == NULL)
		resp = parseResultRecord();

	if(isTokenPending() && resp == NULL) {
		resp = new Resp;
		Token *token = checkToken(Token::END_CODE);
		if(token)
			resp->setType(Resp::TERMINATION);
	}

	return resp;
}

/**
 * @brief Reads output from GDB.
 */
void Com::readTokens()
{
	m_inputBuffer += m_process.readAllStandardOutput();

	// Any characters received?
	while(m_inputBuffer.size() > 0) {
		// Newline received?
		int subLen = m_inputBuffer.indexOf('\n');
		if(subLen != -1) {
			QString row = QString(m_inputBuffer.left(subLen));
			m_inputBuffer = m_inputBuffer.mid(subLen+1);

			if(!row.isEmpty()) {
				debugMsg("row:%s", stringToCStr(row));

#ifdef ENABLE_GDB_LOG
				QString logText;
				logText = ">> ";
				logText += row;
				logText += "\n";
				m_logFile.write(stringToCStr(logText), logText.length());
				m_logFile.flush();
#endif

				QList<Token*> list;
				char firstChar = row[0].toLatin1();
				if(firstChar == '(' ||
						firstChar == '^' ||
						firstChar == '*' ||
						firstChar == '+' ||
						firstChar == '~' ||
						firstChar == '@' ||
						firstChar == '&' ||
						firstChar == '=') {
					list = tokenize(row);
					m_list += list;
				} else if(m_listener)
					m_listener->onTargetStreamOutput(row);
			}
		}

		// Half a line received?
		if(m_inputBuffer.isEmpty() == false) {
			int timeout = 20;
			// Wait for the complete line to be received
			while(m_inputBuffer.indexOf('\n') == -1) {
				m_process.waitForReadyRead(100);
				m_inputBuffer += m_process.readAllStandardOutput();
				timeout--;
				assert(timeout > 0);
			}
		}
	}
}


void Com::readFromGdb(GdbResult *m_result, Tree *m_resultData)
{
	Resp *resp = NULL;

	if(m_result == NULL) {
		// Parse any data received from GDB
		resp = parseOutput();
		if(resp == NULL)
			m_process.waitForReadyRead(100);

		while(!m_freeTokens.isEmpty()) {
			Token *token = m_freeTokens.takeFirst();
			delete token;
		}

		if(resp) {
			m_respQueue.push_back(resp);

			if(resp->getType() == Resp::RESULT) {
				assert(m_resultData != NULL);

				m_resultData->copy(resp->tree);
			}
		}
	} else {
		do {
			do {
				resp = parseOutput();
				if(resp == NULL)
					m_process.waitForReadyRead(100);
			} while(resp == NULL);

			while(!m_freeTokens.isEmpty()) {
				Token *token = m_freeTokens.takeFirst();
				delete token;
			}

			m_respQueue.push_back(resp);

			if(resp->getType() == Resp::RESULT) {
				assert(m_resultData != NULL);

				if(m_result)
					*m_result = resp->m_result;
				m_resultData->copy(resp->tree);
			}

		} while(m_result != NULL && resp->getType() != Resp::TERMINATION);
	}

	// Dump all stderr content
	QByteArray stderrBuffer = m_process.readAllStandardError();
	if(!stderrBuffer.isEmpty()) {
		QString respString = QString(stderrBuffer);
		QStringList respList = respString.split("\n");
		for(int r = 0;r < respList.size();r++) {
			QString row = respList[r];
			if(!row.isEmpty())
				debugMsg("GDB|E>%s", stringToCStr(row));
		}
	} 
}

GdbResult Com::command(Tree *resultData, QString text)
{
	Tree resultDataNull;

	assert(m_busy == 0);

	m_busy++;

	if(resultData == NULL)
		resultData = &resultDataNull;

	debugMsg("# Cmd: %s", stringToCStr(text));

	GdbResult result;

	assert(resultData != NULL);

	resultData->removeAll();

	PendingCommand cmd;
	cmd.m_cmdText = text;
	m_pending.push_back(cmd);

	// Send the command to gdb
	text += "\n";
	m_process.write((const char*)text.toLatin1());

#ifdef ENABLE_GDB_LOG
	//
	QString logText;
	logText = "\n<< ";
	logText += text + "\n";
	m_logFile.write(stringToCStr(logText), logText.length());
	m_logFile.flush();
#endif

	do {
		readFromGdb(&result,resultData);
	} while(!m_pending.isEmpty());

	while(!m_list.isEmpty())
		readFromGdb(NULL,resultData);

	m_busy--;

	dispatchResp();
	onReadyReadStandardOutput();

	return result;
}

int Com::init(QString gdbPath)
{
	QString commandLine;
	commandLine.sprintf("%s --interpreter=mi2", stringToCStr(gdbPath));

	m_process.start(commandLine);//gdb ./testapp/test");
	m_process.waitForStarted();

	if(m_process.state() == QProcess::NotRunning)
		return 1;

	return 0;
}

int Com::getPid()
{
	return m_process.pid();
}

Com& Com::getInstance()
{
	static Com core;
	return core;
}

void Com::onReadyReadStandardOutput ()
{
	if(m_busy != 0)
		return;

	while(m_process.bytesAvailable() || m_list.isEmpty() == false) {
		Tree resultDataNull;
		readFromGdb(NULL, &resultDataNull);

		assert(m_pending.isEmpty() == true);
	}

	dispatchResp();
}

void Com::dispatchResp()
{
	// Dispatch the response
	while(!m_respQueue.isEmpty()) {
		Resp *resp = m_respQueue.takeFirst();
		assert(resp != NULL);

		// Dispatch the response
		if(m_listener) {
			if(resp->getType() == Resp::EXEC_ASYNC_OUTPUT)
				m_listener->onExecAsyncOut(resp->tree, resp->reason);
			if(resp->getType() == Resp::STATUS_ASYNC_OUTPUT)
				m_listener->onStatusAsyncOut(resp->tree, resp->reason);
			if(resp->getType() == Resp::NOTIFY_ASYNC_OUTPUT)
				m_listener->onNotifyAsyncOut(resp->tree,resp->reason);
			if(resp->getType() == Resp::LOG_STREAM_OUTPUT)
				m_listener->onLogStreamOutput(resp->getString());
			if(resp->getType() == Resp::TARGET_STREAM_OUTPUT)
				m_listener->onTargetStreamOutput(resp->getString());
			if(resp->getType() == Resp::CONSOLE_STREAM_OUTPUT)
				m_listener->onConsoleStreamOutput(resp->getString());
			if(resp->getType() == Resp::RESULT)
				m_listener->onResult(resp->tree);
		}
		delete resp;
	}
}
