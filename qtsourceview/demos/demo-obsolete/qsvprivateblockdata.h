#ifndef __QSV_PRIVATEBLOCKDATA_H__
#define __QSV_PRIVATEBLOCKDATA_H__

#include <QTextBlockUserData>

class QsvPrivateBlockData : public QTextBlockUserData
{
public:
	QsvPrivateBlockData();
//private:
	bool m_isCurrentDebugLine;
	bool m_isBookmark;
	bool m_isBreakpoint;
	bool m_isModified;
};

#endif // __QSV_PRIVATEBLOCKDATA_H__
