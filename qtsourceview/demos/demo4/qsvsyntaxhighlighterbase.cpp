#include "qsvsyntaxhighlighterbase.h"

#include <QDebug>
#include <QSyntaxHighlighter>
#include <QTextBlockUserData>
#include <QTextEdit>

QsvSyntaxHighlighterBase::QsvSyntaxHighlighterBase()
{
}

QsvSyntaxHighlighterBase::~QsvSyntaxHighlighterBase()
{
	
}

void QsvSyntaxHighlighterBase::highlightBlock(const QString &text)
{
	if (text.isEmpty())
		return;

	QTextBlock b = getCurrentBlockProxy();
	clearMatchData(b);
	// look up for each of our machting bracket list
	for ( int bracketIndex=0; bracketIndex<m_matchBracketsList.length(); bracketIndex++ )
	{
		QChar bracket       = m_matchBracketsList[bracketIndex];
		int bracketPosition = text.indexOf(bracket);

		while ( bracketPosition != -1 )
		{
			Qate::MatchData m;
			m.matchedChar = m_matchBracketsList[bracketIndex];
			m.position    = bracketPosition;
			addMatchData(b,m);
			bracketPosition = text.indexOf(bracket, bracketPosition+1);
		}
	}
}

void QsvSyntaxHighlighterBase::setTextDocument(QTextDocument *document)
{
	QSyntaxHighlighter *hl = dynamic_cast<QSyntaxHighlighter*>(this);
	if (hl)
		hl->setDocument(document);
}

void QsvSyntaxHighlighterBase::setMatchBracketList( const QString &m )
{
	m_matchBracketsList = m;
}

const QString QsvSyntaxHighlighterBase::getMatchBracketList()
{
	return m_matchBracketsList;
}
