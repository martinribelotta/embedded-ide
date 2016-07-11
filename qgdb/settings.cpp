#include "settings.h"
#include "util.h"
#include "log.h"
#include "ini.h"


Settings::Settings()
: m_connectionMode(MODE_LOCAL)
,m_tcpPort(0)
{
}

void Settings::load(QString filepath)
{
	Ini tmpIni;
	if(tmpIni.appendLoad(filepath))
		infoMsg("Failed to load '%s'. File will be created.", stringToCStr(filepath));

	m_connectionMode = tmpIni.getInt("Mode", MODE_LOCAL) == MODE_LOCAL ? MODE_LOCAL : MODE_TCP;
	m_tcpPort = tmpIni.getInt("TcpPort", 2000);
	m_tcpHost = tmpIni.getString("TcpHost", "localhost");
	m_tcpProgram = tmpIni.getString("TcpProgram", "");
	m_initCommands = tmpIni.getStringList("InitCommands", m_initCommands);
	m_gdbPath = tmpIni.getString("GdpPath", "gdb");
	m_lastProgram = tmpIni.getString("LastProgram", "");
	m_argumentList = tmpIni.getStringList("LastProgramArguments", m_argumentList);

	m_fontFamily = tmpIni.getString("Font","Monospace");
	m_fontSize = tmpIni.getInt("FontSize", 8);

	m_reloadBreakpoints = tmpIni.getBool("ReuseBreakpoints", false);

	QStringList breakpointStringList;
	breakpointStringList = tmpIni.getStringList("Breakpoints", breakpointStringList);
	for(int i = 0;i < breakpointStringList.size();i++) {
		QString str = breakpointStringList[i];
		if(str.indexOf(':') != -1) {
			SettingsBreakpoint bkptCfg;
			bkptCfg.filename = str.left(str.indexOf(':'));
			bkptCfg.lineNo = str.mid(str.indexOf(':')+1).toInt();

			m_breakpoints.push_back(bkptCfg);
		}
	}
}

void Settings::save(QString filepath)
{
	Ini tmpIni;
	tmpIni.appendLoad(filepath);
	tmpIni.setInt("TcpPort", m_tcpPort);
	tmpIni.setString("TcpHost", m_tcpHost);
	tmpIni.setInt("Mode", (int)m_connectionMode);
	tmpIni.setString("LastProgram", m_lastProgram);
	tmpIni.setString("TcpProgram", m_tcpProgram);
	tmpIni.setStringList("InitCommands", m_initCommands);
	tmpIni.setString("GdpPath", m_gdbPath);
	QStringList tmpArgs;
	tmpArgs = m_argumentList;
	tmpIni.setStringList("LastProgramArguments", tmpArgs);

	tmpIni.setString("Font", m_fontFamily);
	tmpIni.setInt("FontSize", m_fontSize);

	tmpIni.setBool("ReuseBreakpoints", m_reloadBreakpoints);

	QStringList breakpointStringList;
	for(int i = 0;i < m_breakpoints.size();i++) {
		SettingsBreakpoint bkptCfg = m_breakpoints[i];
		QString field;
		field = bkptCfg.filename;
		field += ":";
		QString lineNoStr;
		lineNoStr.sprintf("%d", bkptCfg.lineNo);
		field += lineNoStr;
		breakpointStringList.push_back(field);
	}
	tmpIni.setStringList("Breakpoints", breakpointStringList);

	if(tmpIni.save(filepath))
		infoMsg("Failed to save '%s'", stringToCStr(filepath));
}

QStringList Settings::getDefaultKeywordList()
{
	QStringList keywordList;
	keywordList += "if";
	keywordList += "for";
	keywordList += "while";
	keywordList += "switch";
	keywordList += "case";
	keywordList += "else";
	keywordList += "do";
	keywordList += "false";
	keywordList += "true";

	keywordList += "unsigned";
	keywordList += "bool";
	keywordList += "int";
	keywordList += "short";
	keywordList += "long";
	keywordList += "float";
	keywordList += "double";
	keywordList += "void";
	keywordList += "char";
	keywordList += "struct";

	keywordList += "class";
	keywordList += "static";
	keywordList += "volatile";
	keywordList += "return";
	keywordList += "new";
	keywordList += "const";


	keywordList += "uint32_t";
	keywordList += "uint16_t";
	keywordList += "uint8_t";
	keywordList += "int32_t";
	keywordList += "int16_t";
	keywordList += "int8_t";

	return keywordList;
}
