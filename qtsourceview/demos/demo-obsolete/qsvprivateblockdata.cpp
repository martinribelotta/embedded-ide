#include "qsvprivateblockdata.h"

QsvPrivateBlockData::QsvPrivateBlockData(  ) 
	: QTextBlockUserData()
{
	// TODO
	m_isCurrentDebugLine = false;
	m_isBookmark = false;
	m_isBreakpoint = false;
	m_isModified = false;
}
