#include <QSyntaxHighlighter>
#include "qsvsyntaxhighlighterbase.h"

class DefaultHighlighter: public QSyntaxHighlighter, public QsvSyntaxHighlighterBase
{
public:
	DefaultHighlighter( QObject *parent=NULL);
	void highlightBlock(const QString &text);
	virtual void toggleBookmark(QTextBlock &block);
	virtual void removeModification(QTextBlock &block);
	virtual void setBlockModified(QTextBlock &block, bool on);
	virtual bool isBlockModified(QTextBlock &block);
	virtual bool isBlockBookmarked(QTextBlock &block);
	virtual Qate::BlockData::LineFlags getBlockFlags(QTextBlock &block);
	virtual void clearMatchData(QTextBlock &block);
	virtual void addMatchData(QTextBlock &block, Qate::MatchData);
	virtual QList<Qate::MatchData> getMatches(QTextBlock &block);

	virtual QTextBlock getCurrentBlockProxy();

	Qate::BlockData *getBlockData(QTextBlock &block);
};

