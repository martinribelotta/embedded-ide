#ifndef __QATE_HIGHLIGHTER__
#define __QATE_HIGHLIGHTER__

#include <QSyntaxHighlighter>

//#include "qsvtextedit.h"
#include "../demo4/qsvsyntaxhighlighterbase.h"
#include "../demo4/qsvtextoperationswidget.h"
#include "qate/highlighter.h"

class QateHighlighter: public TextEditor::Internal::Highlighter, public QsvSyntaxHighlighterBase
{
public:
	QateHighlighter(QTextDocument *parent=NULL);
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

#endif //  __QATE_HIGHLIGHTER__
