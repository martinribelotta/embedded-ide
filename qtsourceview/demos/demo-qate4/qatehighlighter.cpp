#include "qatehighlighter.h"


QateHighlighter::QateHighlighter(QTextDocument *parent) :
    Highlighter(parent)
{
	setMatchBracketList("()[]''\"\"");
}


void QateHighlighter::highlightBlock(const QString &text)
{
	Highlighter::highlightBlock(text);
	QsvSyntaxHighlighterBase::highlightBlock(text);
}


void QateHighlighter::toggleBookmark(QTextBlock &block)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == NULL) {
		qDebug("Warning - block uas not QateData, broken highlighter?");
		return;
	}
	data->toggleBookmark();
}


void QateHighlighter::removeModification(QTextBlock &block)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == NULL)
		return;
	data->m_isModified = false;
}


void QateHighlighter::setBlockModified(QTextBlock &block, bool on)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == NULL)
		return;
	data->m_isModified = on;
}

bool QateHighlighter::isBlockModified(QTextBlock &block)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == NULL)
		return false;
	return data->m_isModified;
}


bool QateHighlighter::isBlockBookmarked(QTextBlock &block)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == NULL)
		return false;
	return data->isBookmark();
}


Qate::BlockData::LineFlags QateHighlighter::getBlockFlags(QTextBlock &block)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == NULL)
		return 0;
	return data->m_flags;
}

void QateHighlighter::clearMatchData(QTextBlock &block)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == NULL)
		return;
	data->matches.clear();
}

void QateHighlighter::addMatchData(QTextBlock &block, Qate::MatchData m)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == NULL)
		return;
	data->matches << m;
}

QList<Qate::MatchData> QateHighlighter::getMatches(QTextBlock &block)
{
	Qate::BlockData *data = getBlockData(block);
	if (data == NULL)
		return QList<Qate::MatchData>();
	return data->matches;
}

QTextBlock QateHighlighter::getCurrentBlockProxy()
{
	return currentBlock();
}

Qate::BlockData *QateHighlighter::getBlockData(QTextBlock &block)
{
	QTextBlockUserData *userData  = block.userData();
	Qate::BlockData *blockData = NULL;

	if (userData == NULL){
//		WTF WTF WTF!!!
//		The block data must be Hightligher::BlockData!!!
//		blockData =  new Qate::BlockData();
//		block.setUserData(blockData);
	} else {
		blockData = dynamic_cast<Qate::BlockData*>(userData);
	}
	return blockData;
}
