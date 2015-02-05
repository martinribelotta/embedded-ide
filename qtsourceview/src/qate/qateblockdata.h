#ifndef QATE_BLOCK_DATA
#define QATE_BLOCK_DATA

#include <QTextBlockUserData>
#include <QChar>
#include <QFlags>
#include <QList>

namespace Qate {

class MatchData {
public:
	QChar matchedChar;
	int position;
};

class BlockData : public QTextBlockUserData {
public:
	enum LineFlag{
		Bookmark = 1,
		Debug = 2,
		Executing = 4,
		CompileError = 8,
		SyntaxError = 16
	};
	Q_DECLARE_FLAGS(LineFlags,LineFlag)
//	not using QObject.
//	Q_FLAGS(LineFlags)

	QList<MatchData> matches;
	bool m_isModified;
	LineFlags m_flags;

	BlockData(){
		m_isModified = false;
		m_flags      = 0;
	}

	void setFlag( LineFlags f, bool on)
	{
		if (on)
			m_flags |= f;
		else
			m_flags &= !f;
	}

	void toggleFlag( LineFlag f  )
	{
		setFlag(f,!m_flags.testFlag(f));
	}

	bool testFlag( LineFlag f )
	{
		return m_flags.testFlag(f);
	}

	bool isModified()		{ return m_isModified; }
	void setBookmark( bool on )	{ setFlag(Bookmark,on); }
	void toggleBookmark()		{ toggleFlag(Bookmark); }
	bool isBookmark()		{ return m_flags.testFlag(Bookmark); }
	void setDebug( bool on )	{ setFlag(Debug,on);    }
	bool isDebug()			{ return m_flags.testFlag(Debug); }
	void toggleDebug()		{ toggleFlag(Debug);    }
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Qate::BlockData::LineFlags);

} // end of namespace

#endif // QATE_BLOCK_DATA
