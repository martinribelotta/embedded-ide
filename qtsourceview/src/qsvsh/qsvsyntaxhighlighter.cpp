/**
 * \file qsvsyntaxhighlighter.cpp
 * \brief Implementation of the syntax highlighter
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see QsvSyntaxHighlighter
 */

#include <QString>
#include <QStringList>
#include <QDomDocument>
#include <QDomNode>
#include <QFile>
#include <QTextCharFormat>
#include <QPlainTextEdit>

#include "qorderedmap.h"
#include "qsvlangdef.h"
#include "qsvcolordef.h"
#include "qsvcolordeffactory.h"
#include "qsvsyntaxhighlighter.h"

#include "debug_info.h"

/**
 * \class QsvSyntaxHighlighter
 * \brief a simple syntax highlighter for Qt4
 * 
 * QsvSyntaxHighlighter is a simple syntax highlighter for Qt4, base on QSyntaxHighlighter.
 * 
 * This class depends on 2 different inputs in order to paint a text files:
 * 
 * - The syntax definitions 
 * - The color definitions
 * 
 * This separation gives you the freedom to choose different syntaxs and different
 * colors schemes.
 * 
 * \section  SyntaxDefinitions Syntax definitions
 * The syntax definitions are the same ones used by the project GtkSourview 
 * (http://gtksourceview.sourceforge.net/). From GtkSourview 's home page
 * 
 * \verbatim
GtkSourceView is a text widget that extends the standard gtk+ 2.x text widget.
It improves the gtk+ text widget by implementing syntax highlighting and other 
features typical of a source editor.
It is part of the GNOME Desktop Environment and it is currently 
being used by gedit, MonoDevelop, Glimmer and several other projects.
  \endverbatim
 *
 * This class only implements the syntax drawing, while leaving the other parts to different
 * projects (like line numbering, completition auto-brackets... etc). You can also have a 
 * color definition factory and match a file name to a definition ot have a more dynamic
 * syntax coloring. For more details see QsvColorDefFactory
 * 
 * \section ColorDefinitions Color definitions
 * This class also relies on an external color definition. The color definitions can
 * be loaded from XML files by the class QsvColorDefFactory.
 * 
 * \section HowItWorks The way the highling internally works
 * 
 * The way this syntax highlighter works, is pretty simple. The idea is based
 * on the example from Qt4.1.0. The syntax highlighter contains a group of mappings
 * which define what to draw and how to color it. Those matches are grabbed from the QsvLangDef
 * and the colors come from the QsvColorDefFactory. Generally there is no reason to mess
 * arround with those internal functions, and you need only to create new syntax definitions
 * by writing XML files.
 */

QsvSyntaxHighlighter::QsvSyntaxHighlighter( QsvColorDefFactory *colors, QsvLangDef *lang )
	:QSyntaxHighlighter( (QObject*)NULL)
{
	language = NULL;
	this->colors = colors;
	setHighlight( lang );
}
 
/**
 * \brief construct a syntax highlighter 
 * \param parent the document to which the highlight will be applied
 * \param colors the colors definition for drawing the syntax
 * \param lang the syntax definition of the text in this text editor
 * 
 * This function constructs a syntax highlighter and applies it to the
 * document passed. It will use the colors and language defined.
 */
QsvSyntaxHighlighter::QsvSyntaxHighlighter( QTextDocument *parent, QsvColorDefFactory *colors, QsvLangDef *lang )
	:QSyntaxHighlighter(parent)
{
	language = NULL;
	this->colors = colors;
	setHighlight( lang );
}

/**
 * \brief construct a syntax highlighter 
 * \param parent the text editor to which the highlight will be applied
 * \param colors the colors definition for drawing the syntax
 * \param lang the syntax definition of the text in this text editor
 * 
 * This function constructs a syntax highlighter and applies it to the
 * document of the text editor passed. It will use the colors and language defined.
 */
QsvSyntaxHighlighter::QsvSyntaxHighlighter( QTextEdit *parent, QsvColorDefFactory *colors, QsvLangDef *lang )
	:QSyntaxHighlighter(parent)
{
	language = NULL;
	this->colors = colors;

	setHighlight( lang );
}

QsvSyntaxHighlighter::QsvSyntaxHighlighter( QPlainTextEdit *parent, QsvColorDefFactory *colors, QsvLangDef *lang )
	:QSyntaxHighlighter(parent->document())
{
	language = NULL;
	this->colors = colors;

	setHighlight( lang );
}

/**
 * \brief default destructor
 * 
 * destructors the syntax highlighter
 */
QsvSyntaxHighlighter::~QsvSyntaxHighlighter()
{
}

/**
 * \brief change the syntax definition to be used
 * \param newLang the new langauge definition
 * 
 * This function changes the definition of the syntax highlighter. 
 * The old definition is not disposed, you should take care of freeing 
 * the allocated memory yourself.
 * 
 * There is no need to re-highlight the editor yourself, since this function
 * does this for you.
 * 
 */
void QsvSyntaxHighlighter::setHighlight( QsvLangDef *newLang )
{
	if (newLang == language)
		return;

	QString str;
	language = newLang;
	
	mappings.clear();
	
	if (language == NULL)
		return;
		

	//first match keyword lists
	// TODO: optimizations
	foreach( QsvEntityKeywordList l, language->keywordListDefs )
	{
		foreach( QString s, l.list )
		{
//			TODO use these defintions
// 			if (l.matchEmptyStringAtBeginning)
// 			if (l.matchEmptyStringAtEnd)

			// no whole words
			s = l.startRegex + s + l.endRegex;
			addMappingFromName( s, l.style, false, l.caseSensitive? Qt::CaseSensitive : Qt::CaseInsensitive );
		}
	}

	// syntax itmes...
	foreach( QsvEntityBlockComment l, language->syntaxItemDefs )
	{
		QString s;
		if (l.endRegex == "\\n")
			s  = l.startRegex + ".*$";
		else
			s  = l.startRegex + ".*" + l.endRegex;
		addMappingFromName( s, l.style, false, Qt::CaseSensitive );
	}

	// later, pattern items
	// TODO: optimizations
	foreach( QsvEntityPatternItem l, language->patternItems )
	{
		addMappingFromName( l.regex, l.style, !true, Qt::CaseSensitive );
	}

	// strings...
	foreach( QsvEntityString l, language->stringsDefs )
	{
		if (!l.atEOL)
			continue;

		QString s = l.startRegex + QString("[^%1]*").arg(l.startRegex) + l.endRegex;
		addMappingFromName( s, l.style, false, Qt::CaseSensitive );
	}

	// and finally... line comments...
	// block comments are handeled in the drawing function	
	foreach( QsvEntityLineComment l, language->lineCommentsDefs )
	{
		addMappingFromName( QString("%1.*").arg(l.start), l.style, false, Qt::CaseSensitive );
	}

	// now we need to re-highlight, on qt 4.1, we can use this code:
	// setDocument( document() );
	// but since 4.2 we can use this code:
	rehighlight();
}

/**
 * \brief change the colors used to paint the text
 * \param newColors the new colors to be used
 * 
 * This function changes the colors definition of the syntax highlighter. 
 * The old colros are not disposed, you should take care of freeing 
 * the allocated memory yourself.
 * 
 * There is no need to re-highlight the editor yourself, since this function
 * does this for you.
 * 
 * Old colors are not deleted by this method, and you should take care 
 * of freeing that memory manualy.
 */
void QsvSyntaxHighlighter::setColorsDef( QsvColorDefFactory *newColors )
{
	if (colors == newColors)
		return;
		
	colors = newColors;
	QsvLanguageEntity *entity;
	
	// update available mappings
	for(int i = 0; i < mappings.keys().size(); ++i) 
	{
		entity = &(mappings.keys()[i].value);
		entity->charFormat = colors->getColorDef( entity->name ).toCharFormat();
	}

	// now we need to re-highlight, on qt 4.1, we can use this code:
	// setDocument( document() );
	// but since 4.2 we can use this code:
	rehighlight();
}

/**
 * \brief Highlights the given text block. 
 * \param text the text of the current chunck to colorize
 * 
 * This function is called when necessary by the rich text engine, 
 * i.e. on text blocks which have changed.
 * 
 * \see QSyntaxHighlighter::highlightBlock(const QString &text)
 */
void QsvSyntaxHighlighter::highlightBlock(const QString &text)
{
	QsvColorDef lll;
	
	if (!colors)
	{
		qDebug("%s %d - No color factory found" , __FILE__, __LINE__ );
		return;
	}
	
	if (language == NULL)
	{
#ifdef		__DEBUG_HIGHLIGHT__
		qDebug( "%s %d - no language defined", __FILE__, __LINE__ );
#endif		
		return;
	}
	
	// set the whole text to the default format to begin with
	QOrderedMapNode<QString,QsvLanguageEntity> pattern;
	
	// optimizations...
	if (text.simplified().isEmpty())
		goto HANDLE_BLOCK_COMMENTS;
		
	foreach( QsvEntityLineComment l, language->lineCommentsDefs )
	{
		if (text.startsWith( l.start ))
		{
			setFormat( 0, text.length(), colors->getColorDef("dsComment").toCharFormat() );
			return;
		}
	}
	lll = colors->getColorDef("dsNormal");
	setFormat( 0, text.length(), lll.toCharFormat() );

	// this code draws each line
	foreach ( pattern, mappings.keys() )
		drawText( text, pattern.key, pattern.value.charFormat, pattern.value.cs );

	setCurrentBlockState(0);

HANDLE_BLOCK_COMMENTS:
	// what if not block comments defined...?
	if (language->blockCommentsDefs.count() == 0)
		return;

	QRegExp startExpression( language->blockCommentsDefs.at(0).startRegex );
	QRegExp endExpression  ( language->blockCommentsDefs.at(0).endRegex );
	
	int startIndex = 0;
	if (previousBlockState() != 1)
		startIndex = text.indexOf(startExpression);
	
	while (startIndex >= 0) 
	{
		int endIndex = text.indexOf(endExpression, startIndex);
		int commentLength;
		
		if (endIndex == -1) 
		{
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		} 
		else 
		{
			commentLength = endIndex - startIndex
				+ endExpression.matchedLength();
		}
		setFormat( startIndex, commentLength, colors->getColorDef("dsComment").toCharFormat() );
		startIndex = text.indexOf( startExpression, startIndex + commentLength );
	}
}


void QsvSyntaxHighlighter::addMapping( const QString mappingName, const QString &pattern, const QTextCharFormat &format, bool fullWord, Qt::CaseSensitivity cs )
{	
	QString p = pattern;
	QsvLanguageEntity newEntity;
	
	if (fullWord)
		p = "\\b" + p + "\\b";
		
#ifdef __DEBUG_ADD_MAPPING__
	qDebug( "%s %d new mapping (sensitive)- [%s] %s  (%d)", __FILE__, __LINE__, 
		qPrintable(pattern), 
		qPrintable(format.foreground().color().name()),
		cs==Qt::CaseSensitive?1:0
	);
#endif
	newEntity.charFormat = format;
	newEntity.name = mappingName;
	newEntity.cs = cs;

	mappings.add( p, newEntity );
}

void QsvSyntaxHighlighter::addMappingFromName( const QString &pattern, const QString formatName, bool fullWord, Qt::CaseSensitivity cs )
{
	QString s = formatName;
	if (!colors)
		return;

	// convert GTK formats to Kate
	if (s == "Comment")
		s = "dsComment";
	else if (s == "String")
		s = "dsString";
	else if (s == "Preprocessor")
		s = "dsOthers";
	else if (s == "Keyword")
		s = "dsKeyword";
	else if (s == "Data Type")
		s = "dsDataType";
	else if (s == "Decimal")
		s = "dsDecVal";
	else if (s == "Floating Point")
		s = "dsFloat";
	else if (s == "Base-N Integer")
		s = "dsBaseN";
	else if (s == "Function")
		s = "dsFunction";
	else if (s == "Others 2")
		s = "dsOthers2";
	else if (s == "Others 3")
		s = "dsOthers3";

	// at this stage, s is always a Kate color format
	addMapping( s, pattern, colors->getColorDef(s).toCharFormat(), fullWord, cs );
}

void QsvSyntaxHighlighter::drawText( QString text, QString s, QTextCharFormat &format, Qt::CaseSensitivity caseSensitive )
{
//	using regex is a smart idea, but slow
//	if (s.contains( QRegExp("[^\\\\*+()?]") ))
	if	(
			s.contains('^') || s.contains('+') || s.contains('*') || s.contains('?') || 
			s.contains('(') || s.contains(')') || s.contains('[') || s.contains(']') ||
			s.contains('\\') 
		)
		drawRegExp( text, s, format, caseSensitive );
	else
		drawKeywords( text, s, format, caseSensitive );
}

void QsvSyntaxHighlighter::drawRegExp( QString text, QString s, QTextCharFormat &format, Qt::CaseSensitivity caseSensitive )
{	
	QRegExp expression(s);
	expression.setCaseSensitivity(caseSensitive);
	int index = text.indexOf(expression, 0);

#ifdef __DEBUG_HIGHLIGHT__
	qDebug( "%s %d (drawing regex)	-  %s )", __FILE__, __LINE__, qPrintable(s) );
#endif

	while (index >= 0)
	{
		int length = expression.matchedLength();
		setFormat(index, length, format );
		index = text.indexOf(expression, index + length );
	}
}

void QsvSyntaxHighlighter::drawKeywords( QString text, QString s, QTextCharFormat &format, Qt::CaseSensitivity caseSensitive  )
{
#ifdef __DEBUG_HIGHLIGHT__
	qDebug( "%s %d (drawing keyword=sensitive)	- %s (%d)", __FILE__, __LINE__, qPrintable(s), caseSensitive);
#endif

	int index = text.indexOf(s, 0, caseSensitive);
	int length = s.length();
	int txtLen = text.length();
	
	while ( (index >= 0) && (index < txtLen) )
	{
		// paint keyword, only if its suoorunded by white chars
		// regexp are slow, this code looks bad, but faster :)
		if 
		(
			(
				(index==0) || 
				(!text[index-1].isLetterOrNumber() && (text[index-1] != '_'))
			)
				&&
			(
				(index+length>=txtLen) || 
				(!text[index+length].isLetterOrNumber() && (text[index+length] != '_') )
			)
		)
			setFormat(index, length, format );
		index = text.indexOf(s, index + length, caseSensitive);
	}
}

