#include <QPainter>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextCodec>
#include <QTextLayout>
#include <QScrollBar>
#include <QPushButton>
#include <QAction>
#include <QKeySequence>
#include <QTimer>
#include <QPalette>
#include <QFile>
#include <QTextStream>
#include <QStyle>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>

#include <QDebug>

#include "qsveditor.h"
#include "qsveditorpanel.h"

#include "qsvprivateblockdata.h"
#include "qsvsyntaxhighlighter.h"
#include "qsvcolordeffactory.h"
#include "qsvcolordef.h"
#include "transparentwidget.h"

const int floatingWidgetTimeout = 0;

bool isFullWord( QString s1, QString s2, int location )
{
	bool startOk = false;
	bool endOk = false;
	
	if (location == 0)
		startOk = true;
	else
		startOk = ! s1.at(location-1).isLetterOrNumber();

	if (location + s2.length() >= s1.length())
		endOk = true;
	else
		if (location + s2.length() +1 == s1.length())
			endOk = false;
		else 
			endOk = ! s1.at(location + s2.length() + 1).isLetterOrNumber();
	
	return startOk & endOk;
}

QsvEditor::QsvEditor( QWidget *p ) : QTextEditorControl(p)
{
	currentLineColor	= QColor( "#DCE4F9" );
	bookmarkLineColor	= QColor( "#0000FF" );
	breakpointLineColor	= QColor( "#FF0000" );
	matchBracesColor	= QColor( "#FFFF00" ); // was: QColor( "#FF0000" );
	searchFoundColor	= QColor( "#DDDDFF" ); //QColor::fromRgb( 220, 220, 255)
	searchNotFoundColor	= QColor( "#FFAAAA" ); //QColor::fromRgb( 255, 102, 102) "#FF6666"
	whiteSpaceColor		= QColor( "#E0E0E0" );
	highlightCurrentLine	= true;
	showWhiteSpaces		= true;
	showMatchingBraces	= true;
	showPrintingMargins	= true;
	usingSmartHome		= true;
	usingAutoBrackets	= true;
	insertSpacesInsteadOfTabs = false;
	modificationsLookupEnabled = true;
	printMarginWidth	= 80;
	matchStart		= -1;
	matchEnd		= -1;
	matchingString		= "(){}[]\"\"''``";
	textCodec		= NULL;

	actionFind		= NULL;
	actionFindNext		= NULL;
	actionFindPrev		= NULL;
	actionCapitalize	= NULL;
	actionLowerCase		= NULL;
	actionChangeCase	= NULL;
	actionToggleBookmark	= NULL;
	actionTogglebreakpoint	= NULL;
	
	panel = new QsvEditorPanel( this );
	panel->m_panelColor	= QColor( "#FFFFD0" );
	panel->m_modifiedColor	= QColor( "#00FF00" );
	panel->setVisible( true );

	setFrameStyle( QFrame::NoFrame );
	setLineWrapMode( QTextEditorControl::NoWrap );
#if QT_VERSION < 0x040400
	setAcceptRichText( false );
#endif
	QTimer::singleShot( 0, this, SLOT(adjustMarginWidgets()));
	syntaxHighlighter = NULL;

#ifdef WIN32
	endOfLine = DOS;
	QFont f("Courier New", 10);
#else
	endOfLine = Unix;
	QFont f("Monospace", 9);
#endif
	setCurrentFont( f );
	panel->setFont( f );
	setTabSize( 8 );
	
	findWidget = new QsvTransparentWidget( this, 0.80 );
	ui_findWidget.setupUi( findWidget );
	ui_findWidget.searchText->setIcon( QPixmap(":/images/edit-undo.png") );
	ui_findWidget.frame->adjustSize();
	findWidget->adjustSize();
	findWidget->hide();

	replaceWidget = new QsvTransparentWidget( this, 0.80 );
	ui_replaceWidget.setupUi( replaceWidget );
	QApplication::processEvents();
	ui_replaceWidget.replaceOldText->setIcon( QPixmap(":/images/edit-undo.png") );
	ui_replaceWidget.replaceNewText->setIcon( QPixmap(":/images/edit-undo.png") );
	ui_replaceWidget.optionsFrame->hide();
	ui_replaceWidget.frame->adjustSize();
	replaceWidget->adjustSize();
	replaceWidget->hide();
	
	gotoLineWidget = new QsvTransparentWidget( this, 0.80 );
	ui_gotoLineWidget.setupUi( gotoLineWidget );
	ui_gotoLineWidget.frame->adjustSize();
	gotoLineWidget->adjustSize();
	gotoLineWidget->hide();

	fileMessage = new QsvTransparentWidget( this, 0.80 );
	ui_fileMessage.setupUi( fileMessage );
	connect( ui_fileMessage.label, SIGNAL(linkActivated(const QString&)), this, SLOT(on_fileMessage_clicked(QString)));
	fileMessage->hide();
	
	fileSystemWatcher = new QFileSystemWatcher(this);

	//connect( horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(adjustMarginWidgets()));
	//connect( verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(adjustMarginWidgets()));
	connect( this, SIGNAL(cursorPositionChanged()), this, SLOT(on_cursorPositionChanged()));
	connect( document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(on_textDocument_contentsChange(int,int,int)));
	connect( ui_findWidget.searchText, SIGNAL(textChanged(const QString)), this, SLOT(on_searchText_textChanged(const QString)));
	connect( ui_findWidget.searchText, SIGNAL(editingFinished()), this, SLOT(on_searchText_editingFinished()));
	connect( ui_findWidget.searchText, SIGNAL(returnPressed()), this, SLOT(showFindWidget()));

	connect( ui_findWidget.caseSensitive, SIGNAL(clicked()), this, SLOT(on_searchText_editingFinished()));
	connect( ui_findWidget.wholeWords, SIGNAL(clicked()), this, SLOT(on_searchText_editingFinished()));

	connect( ui_findWidget.prevButton, SIGNAL(clicked()), this, SLOT(findPrev()));
	connect( ui_findWidget.nextButton, SIGNAL(clicked()), this, SLOT(findNext()));
	connect( ui_findWidget.closeButton, SIGNAL(clicked()), this, SLOT(showFindWidget()));

	connect( ui_replaceWidget.moreButton, SIGNAL(clicked(bool)), this, SLOT(on_replaceWidget_expand(bool)));
	connect( ui_replaceWidget.replaceOldText, SIGNAL(textChanged(const QString)), this, SLOT(on_replaceOldText_textChanged(const QString)));
	connect( ui_replaceWidget.replaceOldText, SIGNAL(returnPressed()), this, SLOT(on_replaceOldText_returnPressed()));
	connect( ui_replaceWidget.replaceNewText, SIGNAL(returnPressed()), this, SLOT(on_replaceOldText_returnPressed()));
	connect( ui_replaceWidget.replaceButton, SIGNAL(clicked()), this, SLOT(on_replaceOldText_returnPressed()));
	connect( ui_replaceWidget.replaceAllButton, SIGNAL(clicked()), this, SLOT(on_replaceAll_clicked()));
	connect( ui_replaceWidget.closeButton, SIGNAL(clicked()), this, SLOT(showReplaceWidget()));

	connect( ui_gotoLineWidget.lineNumber, SIGNAL(editingFinished()), this, SLOT(on_lineNumber_editingFinished()));
	connect( ui_gotoLineWidget.lineNumber, SIGNAL(valueChanged(int)), this, SLOT(on_lineNumber_valueChanged(int)));
	connect( ui_gotoLineWidget.closeButton, SIGNAL(clicked()), this, SLOT(showGotoLineWidget()));

	connect( ui_fileMessage.closeButton, SIGNAL(clicked()), fileMessage, SLOT(hide()));
	connect( fileSystemWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(on_fileChanged(const QString&)));

	adjustMarginWidgets();
}

QsvEditor::~QsvEditor()
{
	// do we need to do something here...? 
}

void QsvEditor::setupActions()
{
	actionFind = new QAction( tr("&Find..."), this );
	actionFind->setObjectName("qsvEditor::actionFind");
	actionFind->setShortcut( QKeySequence("Ctrl+F") );
	connect( actionFind, SIGNAL(triggered()), this, SLOT(showFindWidget()) );

	actionReplace = new QAction( tr("&Replace..."), this );
	actionReplace->setObjectName("qsvEditor::actionReplace");
	actionReplace->setShortcut( QKeySequence("Ctrl+R") );
	connect( actionReplace, SIGNAL(triggered()), this, SLOT(showReplaceWidget()) );

	actionGotoLine = new QAction( tr("&Goto line..."), this );
	actionGotoLine->setObjectName("qsvEditor::actionGotoLine");
	actionGotoLine->setShortcut( QKeySequence("Ctrl+G") );
	connect( actionGotoLine, SIGNAL(triggered()), this, SLOT(showGotoLineWidget()) );

	actionFindNext = new QAction( tr("Find &next"), this );
	actionFindNext->setObjectName("qsvEditor::actionFindNext");
	actionFindNext->setShortcut( QKeySequence("F3") );
	connect( actionFindNext, SIGNAL(triggered()), this, SLOT(findNext()) );
	
	actionFindPrev = new QAction( tr("Find &previous"), this );
	actionFindPrev->setObjectName("qsvEditor::actionFindPrev");
	actionFindPrev->setShortcut( QKeySequence("Shift+F3") );
	connect( actionFindPrev, SIGNAL(triggered()), this, SLOT(findPrev()) );
	
	actionClearSearchHighlight = new QAction( tr("Clear search &highlight"), this );
	actionClearSearchHighlight->setObjectName("qsvEditor::actionClearSearchHighlight");
	//actionClearSearchHighlight->setShortcut( QKeySequence("Shift+F3") );
	connect( actionClearSearchHighlight, SIGNAL(triggered()), this, SLOT(clearSearchHighlight()) );
	
	actionCapitalize = new QAction( tr("Change to &capital letters"), this );
	actionCapitalize->setObjectName( "qsvEditor::actionCapitalize" );
	actionCapitalize->setShortcut( QKeySequence( Qt::CTRL | Qt::Key_U ) );
	connect( actionCapitalize, SIGNAL(triggered()), this, SLOT(transformBlockToUpper()) );

	actionLowerCase = new QAction( tr("Change to &lower letters"), this );
	actionLowerCase->setObjectName( "qsvEditor::actionLowerCase" );
	actionLowerCase->setShortcut( QKeySequence( Qt::CTRL | Qt::SHIFT | Qt::Key_U  ) );
	connect( actionLowerCase, SIGNAL(triggered()), this, SLOT(transformBlockToLower()) );

	actionChangeCase = new QAction( tr("Change ca&se"), this );
	actionChangeCase->setObjectName( "qsvEditor::actionChangeCase" );
	connect( actionChangeCase, SIGNAL(triggered()), this, SLOT(transformBlockCase()) );

	actionToggleBookmark = new QAction( tr("&Toggle line bookmark"), this );
	actionToggleBookmark->setObjectName( "qsvEditor::actionToggleBookmark" );
	actionToggleBookmark->setShortcut( QKeySequence( Qt::CTRL | Qt::Key_B  ) );
	connect( actionToggleBookmark, SIGNAL(triggered()), this, SLOT(toggleBookmark()) );

	actionPrevBookmark = new QAction( tr("&Previous bookmark"), this );
	actionPrevBookmark->setObjectName( "qsvEditor::actionPrevBookmark" );
	actionPrevBookmark->setShortcut( QKeySequence( Qt::CTRL | Qt::Key_PageUp ) );
	connect( actionPrevBookmark, SIGNAL(triggered()), this, SLOT(gotoPrevBookmark()) );

	actionNextBookmark = new QAction( tr("&Next bookmark"), this );
	actionNextBookmark->setObjectName( "qsvEditor::actionNextBookmark" );
	actionNextBookmark->setShortcut( QKeySequence( Qt::CTRL | Qt::Key_PageDown ) );
	connect( actionNextBookmark, SIGNAL(triggered()), this, SLOT(gotoNextBookmark()) );

	actionToggleBookmark = new QAction( tr("Toggle line &bookmark"), this );
	actionToggleBookmark->setObjectName( "qsvEditor::actionToggleBookmark" );
	actionToggleBookmark->setShortcut( QKeySequence( Qt::CTRL | Qt::Key_B  ) );
	connect( actionToggleBookmark, SIGNAL(triggered()), this, SLOT(toggleBookmark()) );

	actionTogglebreakpoint = new QAction( tr("Toggle b&reakpoint"), this );
	actionTogglebreakpoint->setObjectName( "qsvEditor::actionTogglebreakpoint" );
	actionTogglebreakpoint->setShortcut( QKeySequence("F9") );
	connect( actionTogglebreakpoint, SIGNAL(triggered()), this, SLOT(toggleBreakpoint()) );
	
	actionFindMatchingBracket = new QAction( tr("Goto matching bracket"), this );
	actionFindMatchingBracket->setObjectName( "qsvEditor::actionFindMatchingBracket" );
	QList<QKeySequence> l;
	l << QKeySequence( Qt::CTRL | Qt::Key_BracketRight );
	l << QKeySequence( Qt::CTRL | Qt::Key_BracketLeft );
	actionFindMatchingBracket->setShortcuts( l );
	connect( actionFindMatchingBracket, SIGNAL(triggered()), this, SLOT(findMatchingBracket()) );
}

QColor QsvEditor::getItemColor( ItemColors role )
{
	switch (role)
	{
		case LinesPanel:	return panel->m_panelColor; 
		case ModifiedColor:	return panel->m_modifiedColor;
		case CurrentLine:	return currentLineColor;
		case MatchBrackets:	return matchBracesColor;
		
		case NoText:		return searchNoText;
		case TextFound:		return searchFoundColor;
		case TextNoFound:	return searchNotFoundColor;
		
		case WhiteSpaceColor:	return whiteSpaceColor;
		case BookmarkLineColor:	return bookmarkLineColor;
		case BreakpointLineColor: return breakpointLineColor;
	}
	
	// just to keep gcc happy, will not get executed
	return QColor();
}

void QsvEditor::setItemColor( ItemColors role, QColor c )
{
	switch (role)
	{
		case QsvEditor::LinesPanel:
			panel->m_panelColor = c;
			panel->update();
			break;
		case QsvEditor::ModifiedColor:
			panel->m_modifiedColor = c;
			panel->update();
			break;
		case QsvEditor::CurrentLine:
			currentLineColor = c;
			break;
		case QsvEditor::MatchBrackets:
			matchBracesColor = c;
			break;
		case QsvEditor::NoText:
			searchNoText = c;
			break;
		case QsvEditor::TextFound:
			searchFoundColor = c;
			break;
		case QsvEditor::TextNoFound:
			searchNotFoundColor = c;
			break;
		case QsvEditor::WhiteSpaceColor:
			whiteSpaceColor = c;
			break;
		case QsvEditor::BookmarkLineColor:
			bookmarkLineColor = c;
		case QsvEditor::BreakpointLineColor: 
			breakpointLineColor = c;
	}
}

void	QsvEditor::setMargin( int width )
{
	printMarginWidth = width;
	showPrintingMargins = (width>0);
}

int	QsvEditor::getMargin()
{
	return printMarginWidth;
}

bool	QsvEditor::getInsertSpacesInsteadOfTabs()
{
	return insertSpacesInsteadOfTabs;
}

void	QsvEditor::setInsertSpacesInsteadOfTabs(bool newVal)
{
	insertSpacesInsteadOfTabs = newVal;
}

void	QsvEditor::setTabSize( int size )
{
	const QFontMetrics fm = QFontMetrics( currentFont() );
	int j = fm.width( " " ) * size;
	setTabStopWidth( j );
}

int	QsvEditor::getTabSize()
{
	const QFontMetrics fm = QFontMetrics( currentFont() );
	return tabStopWidth()/ fm.width( " " );
}

QsvSyntaxHighlighter* QsvEditor::getSyntaxHighlighter()
{
	return syntaxHighlighter;
}

void	QsvEditor::setSyntaxHighlighter( QsvSyntaxHighlighter *newSyntaxHighlighter )
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	pauseModificationsLookup();
	syntaxHighlighter = newSyntaxHighlighter;
	if (syntaxHighlighter)
		syntaxHighlighter->setDocument( this->document() );
	
	QApplication::processEvents();
	QApplication::restoreOverrideCursor();
	resumeModificationsLookup();
}

bool	QsvEditor::getDisplayCurrentLine()
{
	return highlightCurrentLine;
}

void	QsvEditor::setDisplayCurrentLine( bool b )
{
	highlightCurrentLine = b;
}

bool	QsvEditor::getDisplayWhiteSpaces()
{
	return showWhiteSpaces;
}

void	QsvEditor::setDisplayWhiteSpaces( bool b )
{
	showWhiteSpaces = b;
}

bool	QsvEditor::getDisplayMatchingBrackets()
{
	return showMatchingBraces;
}

void	QsvEditor::setDisplayMatchingBrackets( bool b )
{
	showMatchingBraces = b;
}

QString QsvEditor::getMatchingString()
{
	return matchingString;
}

void	QsvEditor::setMatchingString( QString s )
{
	matchingString = s;	
}

bool	QsvEditor::getUsingSmartHome()
{
	return usingSmartHome;
}

void	QsvEditor::setUsingSmartHome( bool newValue )
{
	usingSmartHome = newValue;
}

bool	QsvEditor::getUsingAutoBrackets()
{
	return usingAutoBrackets;
}

void	QsvEditor::setUsingAutoBrackets( bool newValue )
{
	usingAutoBrackets = newValue;
}

EndOfLineType	QsvEditor::getEndOfLine()
{
	return endOfLine;
}

void	QsvEditor::setEndOfLine( EndOfLineType newVal )
{
	endOfLine = newVal;
}

QTextCodec*	QsvEditor::getTextCodec()
{
	return textCodec;
}

void	QsvEditor::setTextCodec( QTextCodec* newVal)
{
	textCodec = newVal;
}

void	QsvEditor::setBookmark( BookmarkAction action, QTextBlock block  )
{
	QsvPrivateBlockData *data = getPrivateBlockData( block, true );
	if (!data)
		return;

	switch (action)
	{
		case Toggle:
			data->m_isBookmark = ! data->m_isBookmark;
			break;
		case Enable: 
			data->m_isBookmark = true;
			break;
		case Disable:
			data->m_isBookmark = false;
			break;
	}

	updateCurrentLine();
}

void	QsvEditor::setBreakpoint( BookmarkAction action, QTextBlock block )
{
	QsvPrivateBlockData *data = getPrivateBlockData( block, true );
	if (!data)
		return;

	switch (action)
	{
		case Toggle:
			data->m_isBreakpoint = ! data->m_isBreakpoint;
			break;
		case Enable: 
			data->m_isBreakpoint = true;
			break;
		case Disable:
			data->m_isBreakpoint = false;
			break;
	}
	updateCurrentLine();
}

QWidget* QsvEditor::getPanel()
{
	return panel;
}

// TODO cache state of panel->visible in the widget
void	QsvEditor::adjustMarginWidgets()
{
	if (panel->isVisible())
	{
		setViewportMargins( panel->width()-1, 0, 0, 0 );
		QRect viewportRect = viewport()->geometry();
		QRect lrect = QRect(viewportRect.topLeft(), viewportRect.bottomLeft());
		lrect.adjust( -panel->width(), 0, 0, 0 );
		panel->setGeometry(lrect);
	}
	else
	{
		setViewportMargins( 0, 0, 0, 0 );
	}
}

void	QsvEditor::displayBannerMessage( QString message )
{
	ui_fileMessage.label->setText( message );
	widgetToTop( fileMessage );
}

void	QsvEditor::hideBannerMessage()
{
	fileMessage->hide();
}

void	QsvEditor::clearEditor()
{
	hideBannerMessage();
	pauseFileSystemWatch();
	fileName.clear();
	clear();
	removeModifications();
	setReadOnly( false );
}

int	QsvEditor::loadFile( QString s )
{
	// clear older watches, and add a new one
	QStringList sl = fileSystemWatcher->directories();
	if (!sl.isEmpty())
		fileSystemWatcher->removePaths( sl );

	pauseModificationsLookup();	
	hideBannerMessage();
	this->setReadOnly( false );
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QApplication::processEvents();
	if (!s.isEmpty())
	{
		QFile file(s);
		QFileInfo fileInfo(file);
		
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			return -1;
		
		QTextCodec *c = textCodec;
		if (c == NULL)
			c = QTextCodec::codecForLocale();

		QTextStream textStream(&file);
		textStream.setCodec( c );
		setPlainText( textStream.readAll() );
		file.close();
	
		fileName = fileInfo.absoluteFilePath();
		fileSystemWatcher->addPath( fileName );
		if (!fileInfo.isWritable())
		{
			this->setReadOnly( true );
			displayBannerMessage( tr("The file is readonly. Click <a href=':forcerw' title='Click here to try and change the file attributes for write access'>here to force write access.</a>") );
		}
	}
	
	resumeModificationsLookup();
	removeModifications();

	QApplication::restoreOverrideCursor();
	return 0;
}

int	QsvEditor::saveFile( QString newFileName )
{
	pauseModificationsLookup();	
	hideBannerMessage();
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QApplication::processEvents();
	
	QFile file( newFileName );

	if (!file.open( QIODevice::WriteOnly ) )
		return false;
	
	QTextCodec *c = textCodec;
	if (c == NULL)
		c = QTextCodec::codecForLocale();
	
	QTextStream textStream(&file);
	textStream.setCodec( c );

	QTextBlock block = document()->begin();
	while(block.isValid())
	{
		if (endOfLine==KeepOldStyle)
			textStream << block.text();
		else
		{
			QString s = block.text(); 
			int i = s.length();
			
			if (!s.isEmpty()) if ((s[i-1] == '\n') || (s[i-1] == '\r'))
				s = s.left( i-1 );
			if (!s.isEmpty()) if ((s[i-1] == '\n') || (s[i-1] == '\r'))
				s = s.left( i-1 );
			textStream << s;
			switch (endOfLine)
			{
				case DOS:	textStream << "\r\n"; break; 
				case Unix: 	textStream << "\n"; break;
				case Mac:	textStream << "\r"; break;
				default:	return 0;		// just to keep gcc happy
			}
		}
		block = block.next();
	}
	file.close();

	fileName = newFileName;
	resumeModificationsLookup();
	removeModifications();
	QApplication::restoreOverrideCursor();
	return 0;
}

QString	QsvEditor::getFileName()
{
	return fileName;
}

int	QsvEditor::getIndentationSize( const QString s )
{
	int indentation = 0;
	int i = 0;
	int l = s.length();
	
	if (l == 0)
		return 0;
			
	QChar c = s.at(i);
	
	while( (i<l) && ((c == ' ') || (c == '\t')) )
	{
		if (c == '\t')
		{
			indentation ++;
			i ++;
		}
		else if (c == ' ')
		{
			int k=0;
			while( (i+k<l) && (s.at(i+k) == ' ' ))	
				k ++;
			i += k;
			indentation += k / getTabSize();
		}

		if (i<l)
			c = s.at(i);
	}
	
	return indentation;
}

void	QsvEditor::applyConfiguration( QsvEditorConfigData c )
{
	setUsingAutoBrackets( c.autoBrackets );
	setDisplayCurrentLine( c.markCurrentLine );
	getPanel()->setVisible( c.showLineNumbers );
	setDisplayWhiteSpaces( c.showWhiteSpaces );
	setDisplayMatchingBrackets( c.matchBrackets );
	setMatchingString( c.matchBracketsList );
	setUsingSmartHome( c.smartHome );
	if (c.showMargins)
		setMargin( c.marginsWidth );
	else
		setMargin( -1 );
	
	setInsertSpacesInsteadOfTabs( c.insertSpacesInsteadOfTabs );
	setTabSize( c.tabSize );
	getPanel()->setFont( c.currentFont );
	
	QTextCursor backup = textCursor();
	selectAll();
	setCurrentFont( c.currentFont );
	setTextCursor( backup );
	
	if (c.lineWrapping)
	{
		if (c.showMargins)
		{
#if QT_VERSION < 0x040400
			const QFontMetrics fm = QFontMetrics( currentFont() );
			const int newWrapWidth = fm.width( " " ) * c.marginsWidth;
			setLineWrapMode( QTextEditorControl::FixedPixelWidth );
			setLineWrapColumnOrWidth( newWrapWidth );
#endif
		}
		else
		{
			setLineWrapMode( QTextEditorControl::WidgetWidth );
		}
	}
	else
	{
		setLineWrapMode( QTextEditorControl::NoWrap );
	}
	
	if (c.currentColorScheme == NULL )
		qDebug("%s %d - Warning - no color scheme found!", __FILE__, __LINE__ );
	else
	{
		QPalette p( palette() );
		p.setColor( QPalette::Base, c.currentColorScheme->getColorDef("dsWidgetBackground").getBackground() );
		setPalette( p );
#if QT_VERSION < 0x040400
		setTextColor( c.currentColorScheme->getColorDef("dsNormal").getColor() );
#endif
		setItemColor( QsvEditor::LinesPanel, c.currentColorScheme->getColorDef("dsWidgetLinesPanel").getBackground() );
		setItemColor( QsvEditor::ModifiedColor, c.currentColorScheme->getColorDef("dsWidgetModifiedLine").getBackground() );
		setItemColor( QsvEditor::CurrentLine, c.currentColorScheme->getColorDef("dsWidgetCurLine").getBackground() );
		setItemColor( QsvEditor::MatchBrackets, c.currentColorScheme->getColorDef("dsCurrectBracket").getBackground() );
		setItemColor( QsvEditor::WhiteSpaceColor, c.currentColorScheme->getColorDef("dsWhiteSpace").getColor() );
		setItemColor( QsvEditor::BookmarkLineColor, c.currentColorScheme->getColorDef("dsWidgetBookmark").getBackground() );
		setItemColor( QsvEditor::BreakpointLineColor, c.currentColorScheme->getColorDef("dsWidgetActiveBreakpoint").getBackground() );

		pauseModificationsLookup();
		QsvSyntaxHighlighter *sh = getSyntaxHighlighter();
		if (sh)
		{
			sh->setColorsDef( c.currentColorScheme );
			sh->rehighlight();
		}
		/*else
			qDebug( "%s %d - Warning no syntax highlighter found!", __FILE__, __LINE__ ); */

		resumeModificationsLookup();
	}
	
	adjustMarginWidgets();
	update();
	viewport()->update();
}

QString	QsvEditor::updateIndentation( QString s, int indentation )
{
	if (s.isEmpty())
		return s;
		
	QString spaces;
	if (insertSpacesInsteadOfTabs)
	{
		int k = getTabSize();
		for(int i=0; i< k; i++ )
			spaces = spaces.insert(0,' ');
	}
	
	while ((s.at(0) == ' ') || (s.at(0) == '\t'))
	{
		s.remove(0,1);
		if (s.isEmpty())
			break;
	}
	
	while( indentation != 0 )
	{
		if (insertSpacesInsteadOfTabs)
			s = s.insert( 0, spaces );
		else
			s = s.insert( 0, QChar('\t') );
		indentation --;
	}
	
	return s;
}

void	QsvEditor::removeModifications()
{
	int i = 1;
	for ( QTextBlock block = document()->begin(); block.isValid(); block = block.next() )
	{
		QsvPrivateBlockData *data = dynamic_cast<QsvPrivateBlockData*>( block.userData() );
		if (!data)
			continue;
		data->m_isModified = false;
		i ++;
	}
	
	panel->update();
}

void	QsvEditor::pauseFileSystemWatch()
{
	disconnect( fileSystemWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(on_fileChanged(const QString&)));
	fileSystemWatcher->removePath( fileName );
}

void	QsvEditor::resumeFileSystemWatch()
{
	connect( fileSystemWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(on_fileChanged(const QString&)));
	fileSystemWatcher->addPath( fileName );
}

void	QsvEditor::pauseModificationsLookup()
{
	modificationsLookupEnabled = false;
}

void	QsvEditor::resumeModificationsLookup()
{
	modificationsLookupEnabled = true;
}

void	QsvEditor::showFindWidget()
{
	if (replaceWidget->isVisible())
		replaceWidget->hide();
	
	if (gotoLineWidget->isVisible())
		gotoLineWidget->hide();
	
	if (findWidget->isVisible())
	{
		QTimer::singleShot( floatingWidgetTimeout, findWidget, SLOT(hide()) );
		this->setFocus();
		return;
	}

	searchCursor = textCursor();
	widgetToBottom( findWidget );
	ui_findWidget.searchText->clear();
	ui_findWidget.searchText->setFocus();
}

void	QsvEditor::showReplaceWidget()
{
	if (findWidget->isVisible())
		findWidget->hide();
	
	if (gotoLineWidget->isVisible())
		gotoLineWidget->hide();
	
	if (replaceWidget->isVisible())
	{
		QTimer::singleShot( floatingWidgetTimeout, replaceWidget, SLOT(hide()) );
		this->setFocus();
		return;
	}
	
	searchCursor = textCursor();
	ui_replaceWidget.replaceOldText->clear();
	ui_replaceWidget.replaceOldText->setFocus();
	widgetToBottom( replaceWidget );
}

void	QsvEditor::showGotoLineWidget()
{
	if (findWidget->isVisible())
		findWidget->hide();
	
	if (replaceWidget->isVisible())
		replaceWidget->hide();

	if (gotoLineWidget->isVisible())
	{
		QTimer::singleShot( floatingWidgetTimeout, gotoLineWidget, SLOT(hide()));
		this->setFocus();
		return;
	}
	
	QTextCursor c = textCursor();
	ui_gotoLineWidget.lineNumber->setValue( c.blockNumber()+1 );
	
	c.movePosition(QTextCursor::End);
	ui_gotoLineWidget.lineNumber->setMaximum( c.blockNumber()+1 );
	ui_gotoLineWidget.linesCountLabel->setText( tr("%1 lines available").arg(c.blockNumber()+1) );
	ui_gotoLineWidget.lineNumber->setMinimum( 1 );
	ui_gotoLineWidget.lineNumber->setFocus();
	QLineEdit *lineEdit = ui_gotoLineWidget.lineNumber->findChild<QLineEdit*>();
	if (lineEdit)
		lineEdit->selectAll();
	widgetToBottom( gotoLineWidget );
}

void	QsvEditor::clearSearchHighlight()
{
	highlightString.clear();
	viewport()->update();
}

void	QsvEditor::findNext()
{
	issue_search( ui_findWidget.searchText->text(), textCursor(), getSearchFlags() );
}

void	QsvEditor::findPrev()
{
	issue_search( ui_findWidget.searchText->text(), textCursor(), getSearchFlags() | QTextDocument::FindBackward );
}

void	QsvEditor::toggleBreakpoint()
{
	setBreakpoint( Toggle, textCursor().block() );
}

void	QsvEditor::toggleBookmark()
{
	setBookmark( Toggle, textCursor().block() );
}

void	QsvEditor::gotoPrevBookmark()
{
	QTextCursor cursor = textCursor();
	QTextBlock block = cursor.block();
	QsvPrivateBlockData *data = getPrivateBlockData( block, false );
	
	while (block.isValid())
	{
		block = block.previous();
		data = getPrivateBlockData( block, false );
		if (data)
		{
			if (data->m_isBookmark)
			{
				cursor.setPosition( block.position() );
				setTextCursor(cursor);
				return;
			}
		}
	}
}

void	QsvEditor::gotoNextBookmark()
{
	QTextCursor cursor = textCursor();
	QTextBlock block = cursor.block();
	QsvPrivateBlockData *data = getPrivateBlockData( block, false );
	
	while (block.isValid())
	{
		block = block.next();
		data = getPrivateBlockData( block, false );
		if (data)
		{
			if (data->m_isBookmark)
			{
				cursor.setPosition( block.position() );
				setTextCursor(cursor);
				return;
			}
		}
	}
}

void	QsvEditor::transformBlockToUpper()
{
	QTextCursor cursor = getCurrentTextCursor();
	QString s_before = cursor.selectedText();
	QString s_after  = s_before.toUpper();
	
	if (s_before != s_after)
	{
		cursor.beginEditBlock();
		cursor.deleteChar();
		cursor.insertText( s_after );
		cursor.endEditBlock();
		setTextCursor( cursor );
	}
}

void	QsvEditor::transformBlockToLower()
{
	QTextCursor cursor = getCurrentTextCursor();
	QString s_before = cursor.selectedText();
	QString s_after  = s_before.toLower();
	
	if (s_before != s_after)
	{
		cursor.beginEditBlock();
		cursor.deleteChar();
		cursor.insertText( s_after );
		cursor.endEditBlock();
		setTextCursor( cursor );
	}
}

void	QsvEditor::transformBlockCase()
{
	QTextCursor cursor = getCurrentTextCursor();
	QString s_before = cursor.selectedText();
	QString s_after = s_before;
	uint s_len = s_before.length();
	
	for( uint i=0; i< s_len; i++ )
	{
		QChar c = s_after[i];
		if (c.isLower())
			c = c.toUpper();
		else if (c.isUpper())
			c = c.toLower();
		s_after[i] = c;
	}
		
	if (s_before != s_after)
	{
		cursor.beginEditBlock();
		cursor.deleteChar();
		cursor.insertText( s_after );
		cursor.endEditBlock();
		setTextCursor( cursor );
	}
}

void	QsvEditor::smartHome()
{
	QTextCursor c = textCursor();
	int blockLen = c.block().text().length();
	if (blockLen == 0 )
		return;

	int originalPosition = c.position();
	QTextCursor::MoveMode moveAnchor = QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)? 
		 QTextCursor::KeepAnchor:QTextCursor::MoveAnchor;
	
	c.movePosition(QTextCursor::StartOfLine, moveAnchor);
	int startOfLine = c.position();
	int i = 0;
	
	while ( c.block().text()[i].isSpace())
	{
		i ++;
		if (i==blockLen)
		{
			i = 0;
			break;
		}
	}
	
	if ((originalPosition == startOfLine) || (startOfLine + i != originalPosition ))
		c.setPosition( startOfLine + i, moveAnchor );
	setTextCursor( c );
}

void	QsvEditor::smartEnd()
{
	QTextCursor c = textCursor();
	int blockLen = c.block().text().length();
	if (blockLen == 0)
		return;

	int originalPosition = c.position();
	QTextCursor::MoveMode moveAnchor = QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)? 
		 QTextCursor::KeepAnchor:QTextCursor::MoveAnchor;
	c.movePosition(QTextCursor::StartOfLine,moveAnchor);
	int startOfLine = c.position();
	c.movePosition(QTextCursor::EndOfLine,moveAnchor);
	int i = blockLen;
	
	while (c.block().text()[i-1].isSpace())
	{
		i --;
		if (i==1)
		{
			i = blockLen;
			break;
		}
	}

	if ((originalPosition == startOfLine) || (startOfLine + i != originalPosition ))
		c.setPosition( startOfLine + i, moveAnchor );

	setTextCursor( c );
}

void	QsvEditor::findMatchingBracket()
{
	if (matchStart==-1)
		return;

	QTextCursor c = textCursor();
	if (c.position() == matchStart)
		c.setPosition(matchEnd);
	else if (c.position() == matchEnd)
		c.setPosition(matchStart);
	setTextCursor( c );
}

void	QsvEditor::updateCurrentLine()
{
	if (highlightCurrentLine)
		viewport()->update();
		
	panel->update();
}

void	QsvEditor::on_searchText_textChanged( const QString & text )
{
	if (text.isEmpty())
	{
		QPalette p = palette();
		p.setColor( QPalette::Base, QColor("#FFFFFF") ); // white
		ui_findWidget.searchText->setPalette( p );
		return;
	}
	
	issue_search( text, searchCursor, getSearchFlags() ); 
}

void	QsvEditor::on_searchText_editingFinished()
{
	//showFindWidget();
	highlightString = ui_findWidget.searchText->text();
	on_searchText_textChanged(highlightString);
	viewport()->update();
}

void	QsvEditor::on_replaceWidget_expand( bool checked )
{
	ui_replaceWidget.optionsFrame->setVisible( checked );
	ui_replaceWidget.frame->adjustSize();
	replaceWidget->adjustSize();
	widgetToBottom( replaceWidget );
}

void	QsvEditor::on_replaceOldText_textChanged( const QString & text )
{
	QPalette palette = this->palette();
	if (text.isEmpty())
	{
		palette.setColor( QPalette::Base, QColor("#FFFFFF") ); // white
		ui_replaceWidget.replaceOldText->setPalette( palette );
		return;
	}

	QTextCursor c = textCursor();
	//c.movePosition(QTextCursor::Start);
	c = document()->find( text, c, getReplaceFlags() );
	
	if (!c.isNull())
		palette.setColor( QPalette::Base, searchFoundColor );
	else
		palette.setColor( QPalette::Base, searchNotFoundColor );
	ui_replaceWidget.replaceOldText->setPalette( palette );
}

void	QsvEditor::on_replaceOldText_returnPressed()
{
	if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) ||
	    QApplication::keyboardModifiers().testFlag(Qt::AltModifier) ||
	    QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) )
	{
		on_replaceAll_clicked();
		showReplaceWidget();
		return;
	}
	
	QTextCursor c = textCursor();
	c = document()->find( ui_replaceWidget.replaceOldText->text(), c, getReplaceFlags() );
	if (c.isNull())
		return;
	
	c.beginEditBlock();
	c.deleteChar();
	c.insertText( ui_replaceWidget.replaceNewText->text() );
	c.endEditBlock();
	setTextCursor( c );
}

void	QsvEditor::on_replaceAll_clicked()
{
	// WHY NOT HIDING THE WIDGET?
	// it seems that if you hide the widget, when the replace all action 
	// is triggered by pressing control+enter on the replace widget
	// eventually an "enter event" is sent to the text eidtor.
	// the work around is to update the transparency of the widget, to let the user
	// see the text bellow the widget
	
	//showReplaceWidget();
	
	int replaceCount = 0;
	replaceWidget->setWidgetTransparency( 0.2 );
	QTextCursor c = textCursor();
	c = document()->find( ui_replaceWidget.replaceOldText->text(), c, getReplaceFlags() );
	
	while (!c.isNull())
	{
		setTextCursor( c );
		QMessageBox::StandardButton button = QMessageBox::question( this, tr("Replace all"), tr("Replace this text?"),
			QMessageBox::Yes | QMessageBox::Ignore | QMessageBox::Cancel );
		
		if (button == QMessageBox::Cancel)
		{
			break;
			
		}
		else if (button == QMessageBox::Yes)
		{
			c.beginEditBlock();
			c.deleteChar();
			c.insertText( ui_replaceWidget.replaceNewText->text() );
			c.endEditBlock();
			setTextCursor( c );
			replaceCount++;
		}
		
		c = document()->find( ui_replaceWidget.replaceOldText->text(), c, getReplaceFlags() );
	}
	replaceWidget->setWidgetTransparency( 0.8 );
	
	QMessageBox::information( 0, tr("Replace all"), tr("%1 replacement(s) made").arg(replaceCount) );
}

void	QsvEditor::on_lineNumber_editingFinished()
{
	// toggle the widget visibility only if visible
	if (gotoLineWidget->isVisible())
		showGotoLineWidget();
}

void	QsvEditor::on_lineNumber_valueChanged(int i)
{
	int requestedBlockNum = i - 1;
	QTextCursor c = textCursor();
	for( c.movePosition(QTextCursor::Start); requestedBlockNum && (!c.isNull()); --requestedBlockNum )
		c.movePosition( QTextCursor::NextBlock );
	
	if (c.isNull())
		return;
	
	setTextCursor( c );
}

void	QsvEditor::on_cursorPositionChanged()
{
	QTextCursor cursor = textCursor();
	int pos = cursor.position();
	if (pos == -1)
	{
		matchStart = matchEnd = -1;
		charStart = charEnd = 0;
		if (highlightCurrentLine)
			updateCurrentLine();
		return;
	}
		
	QTextBlock  block = cursor.block();
		int i = cursor.position();
	QChar currentChar = block.text()[i - block.position() ];
	matchStart = i;

	int j = matchingString.indexOf( currentChar );

	if (j == -1)
	{
		matchStart = matchEnd = -1;
		charStart = charEnd = 0;
		if (highlightCurrentLine)
			updateCurrentLine();
		return;
	}

	if ( matchingString[j] != matchingString[j+1] )
		if (j %2 == 0)
			findMatching( charStart = matchingString[j], charEnd = matchingString[j+1], true, block );
		else
			findMatching( charStart = matchingString[j], charEnd = matchingString[j-1], false, block );
	else
		findMatching( charStart = charEnd = matchingString[j], block );
		
	updateCurrentLine();
}

void	QsvEditor::on_textDocument_contentsChange( int position, int charsRemoved, int charsAdded )
{
	if (!modificationsLookupEnabled)
		return;
	
	if (charsAdded < 2)
	{
		QsvPrivateBlockData* data = getPrivateBlockData( textCursor().block(), true );
		if (!data)		return;
		if (data->m_isModified)	return;
		data->m_isModified = true;
	}
	else
	{
		int remaining = 0;
		QTextCursor cursor( document() );
		cursor.setPosition( position );
		int oldRemaining = -1;
		while ( (remaining+1 < charsAdded) && (oldRemaining != remaining) )
		{
			oldRemaining = remaining;
			QsvPrivateBlockData* data = getPrivateBlockData( cursor.block(), true );
			if (data) 
				data->m_isModified = true;	// should not happen, but can't be too safe
			cursor.movePosition( QTextCursor::NextBlock );
			remaining = cursor.position() - position;
		}
	}
	panel->update();
	
	Q_UNUSED( charsRemoved );
}

void	QsvEditor::on_fileChanged( const QString &fName )
{
	if (fName != fileName)
		return;
		
	QFileInfo f(fileName);
	QString message;
	
	if (f.exists())
		message = tr("File has been modified outside the editor. <a href=':reload' title='Clicking this links will revert all changes to this editor'>Click here to reload.</a>");
	else
		message = tr("File has been deleted outside the editor.");
	
	displayBannerMessage( message );
}

void	QsvEditor::on_fileMessage_clicked( QString s )
{
	if (s == ":reload")
	{
		loadFile( fileName );
		ui_fileMessage.label->setText( "" );
		fileMessage->hide();
	} else if (s == ":forcerw")
	{
		// TODO how to do it in a portable way?
		fileMessage->hide();
	}
}

void	QsvEditor::keyPressEvent( QKeyEvent *event )
{
	switch (event->key())
	{
		case Qt::Key_Escape:
			if (findWidget->isVisible())
				showFindWidget();
			else if (replaceWidget->isVisible())
				showReplaceWidget();
			else if (gotoLineWidget->isVisible())
				showGotoLineWidget();
			else
			{	// clear selection
				QTextCursor c = textCursor();
				if (c.hasSelection())
				{
					c.clearSelection();
					setTextCursor(c);
				}
			}
			break;
		
		case Qt::Key_Home:
			if (!usingSmartHome || QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) )
				break;
			smartHome();
			event->accept();
			return;
			
		case Qt::Key_End:
			if (!usingSmartHome || QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) )
				break;
			smartEnd();
			event->accept();
			return;
		
		case Qt::Key_Down:
			if (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
				break;
			verticalScrollBar()->triggerAction( QAbstractSlider::SliderSingleStepAdd );
			event->accept();
			break;
			
		case Qt::Key_Up:
			if (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
				break;
			verticalScrollBar()->triggerAction( QAbstractSlider::SliderSingleStepSub );
			event->accept();
			break;
			
		case Qt::Key_PageDown:
			if (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
				break;
			verticalScrollBar()->triggerAction( QAbstractSlider::SliderPageStepAdd);
			event->accept();
			break;

		case Qt::Key_PageUp:
			if (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
				break;
			verticalScrollBar()->triggerAction( QAbstractSlider::SliderPageStepSub );
			event->accept();
			break;
			
		case Qt::Key_Enter:
		case Qt::Key_Return:
			if (findWidget->isVisible() || replaceWidget->isVisible() || gotoLineWidget->isVisible())
			{
				event->ignore();
				return;
			}
			break;
			
		case Qt::Key_Tab:
			if (replaceWidget->isVisible())
			{	// since the replace widget has many subwidgets
				// tab should cycle between them
				event->ignore();
				return;
			}
			//if (tabIndents)
			{
				if (handleIndentEvent( !(event->modifiers() & Qt::ShiftModifier) ))
					// do not call original hanlder, if this was handled by that function
					return; 
			}
		case Qt::Key_Backtab:
			if (replaceWidget->isVisible())
			{
				event->ignore();
				return;
			}
			//if (tabIndents)
				if (handleIndentEvent(false))
					return; 
		
		default:
			if (usingAutoBrackets && handleKeyPressEvent(event))
				return;
	} // end case
	
	QTextEditorControl::keyPressEvent( event );
}

void	QsvEditor::resizeEvent ( QResizeEvent *event )
{
	QTextEditorControl::resizeEvent( event );
	adjustMarginWidgets();

	if (findWidget->isVisible())
	{
		findWidget->hide();
		showFindWidget();
	}
	if (replaceWidget->isVisible())
	{
		replaceWidget->hide();
		showReplaceWidget();
	}

	if (gotoLineWidget->isVisible())
	{
		gotoLineWidget->hide();
		showGotoLineWidget();
	}

	if (fileMessage->isVisible())
	{
		fileMessage->hide();
		widgetToTop( fileMessage );
	}
}

void	QsvEditor::timerEvent( QTimerEvent *event )
{
	// TODO
	Q_UNUSED( event );
}

void	QsvEditor::paintEvent(QPaintEvent *e)
{
	// if no special painting, no need to create the QPainter
	if (highlightCurrentLine || showWhiteSpaces || showMatchingBraces)
	{
		QPainter p( viewport() );
		printBackgrounds(p);
		if (showMatchingBraces)
			printMatchingBraces( p );
		p.end();
		
		QTextEditorControl::paintEvent(e);
	}
	else
		QTextEditorControl::paintEvent(e);
}

void	QsvEditor::printBackgrounds( QPainter &p )
{
	const int contentsY = verticalScrollBar()->value();
	const qreal pageBottom = contentsY + viewport()->height();
	const QFontMetrics fm = QFontMetrics( currentFont() );
	
	for ( QTextBlock block = document()->begin(); block.isValid(); block = block.next() )
	{
		QTextLayout* layout = block.layout();
		const QRectF boundingRect = layout->boundingRect();
		QPointF position = layout->position();
		
		if ( position.y() +boundingRect.height() < contentsY )
			continue;
		if ( position.y() > pageBottom )
			break;
			
		if (highlightCurrentLine)
			printCurrentLines( p, block );
		if (showWhiteSpaces)
			printWhiteSpaces( p, block, fm );
		if (!highlightString.isEmpty())
			printHighlightString( p, block, fm );
	}

	if (showPrintingMargins)
		printMargins( p );
}

void	QsvEditor::printWhiteSpaces( QPainter &p, const QTextBlock &block, const QFontMetrics &fm )
{
	const QString txt = block.text();
	const int len = txt.length();
	QChar printChar = 'X';
	int dx = 0 + fm.leading() + fm.width( printChar ) - fm.leftBearing( printChar );
	int dy = 0 + fm.height() - fm.descent();
	
	p.setFont( currentFont() );
	p.setPen( whiteSpaceColor );
	for ( int i=0; i<len; i++ )
	{
		if (txt[i] == ' ' )
			printChar = 0xB7; // ?
		else if (txt[i] == '\t' )
			printChar = 0xBB; // ?
		else 
			continue;
		
		// pixmaps are of size 8x8 pixels
		QTextCursor cursor = textCursor();
		cursor.setPosition( block.position() + i, QTextCursor::MoveAnchor);
		
		QRect r = cursorRect( cursor );
		int x = r.x() + dx;
		int y = r.y() + dy;
		p.drawText( x, y, printChar );
	}
}

void	QsvEditor::printCurrentLines( QPainter &p, const QTextBlock &block )
{
	QsvPrivateBlockData *data = dynamic_cast<QsvPrivateBlockData*>( block.userData() );
	QTextCursor cursor = textCursor();
	cursor.setPosition( block.position(), QTextCursor::MoveAnchor);
	QRect r = cursorRect( cursor );
	r.setX( 0 );
	r.setWidth( viewport()->width() );

	p.save();
	p.setOpacity( 0.8 );
	if (r.top() == cursorRect().top() )
		p.fillRect( r, currentLineColor );
	p.setOpacity( 0.2 );
	if (data)
	{
		if (data->m_isBookmark)
			p.fillRect( r, bookmarkLineColor );
		
		if (data->m_isBreakpoint)
			p.fillRect( r, breakpointLineColor );
		
		// TODO: print all other states in a generic way
	}
	p.restore();
}

#if 0
void	QsvEditor::printMatchingBraces( QPainter &p )
{
	if (matchStart == -1)
		return;

#if QT_VERSION > 0x040300
	QColor lighter = matchBracesColor.lighter();
#else
	// STUPID QT 4.2 on etch
	QColor lighter = matchBracesColor.light(150);
#endif
	QTextCursor cursor = textCursor();
	QRect r;
	int charWidth;
	
	p.setFont( currentFont() );
	p.setPen( matchBracesColor );
	
	charWidth = p.fontMetrics().width( charStart );
	cursor.setPosition(matchStart, QTextCursor::MoveAnchor);
	r = cursorRect( cursor );
	p.fillRect( r.x()+charWidth/4, r.y(), charWidth, r.height(), matchBracesColor );
	
	if (matchEnd==-1)
		return;
	charWidth = p.fontMetrics().width( charEnd );
	cursor.setPosition(matchEnd, QTextCursor::MoveAnchor);
	r = cursorRect( cursor );
	p.fillRect( r.x()+charWidth/4, r.y(), charWidth, r.height(), matchBracesColor );
}
#else
void	QsvEditor::printMatchingBraces( QPainter &p )
{
	QTextCursor cursor = textCursor();
// 		QFont f = document()->defaultFont();
	QFont f = currentFont();
	QColor matchBracesColor("#FFFF00");
	QRect r;
	int charWidth;
	
	p.setFont( f );
	p.setPen( matchBracesColor );
	
	if (matchStart==-1)
		return;
	charWidth = p.fontMetrics().width( charStart );
	cursor.setPosition(matchStart, QTextCursor::MoveAnchor);
	r = cursorRect( cursor );
	p.fillRect( r.x()+charWidth/4, r.y(), charWidth, r.height(), matchBracesColor );
// 		p.fillRect( r.x()+charWidth, r.y(), charWidth, r.height(), matchBracesColor );
		
	
	if (matchEnd==-1)
		return;
	charWidth = p.fontMetrics().width( charEnd );
	cursor.setPosition(matchEnd, QTextCursor::MoveAnchor);
	r = cursorRect( cursor );
	p.fillRect( r.x()+charWidth/4, r.y(), charWidth, r.height(), matchBracesColor );
// 		p.fillRect( r.x()+charWidth, r.y(), charWidth, r.height(), matchBracesColor );
}

#endif

void	QsvEditor::printHighlightString( QPainter &p, const QTextBlock &block, const QFontMetrics &fm )
{
	int highlightStringLen = fm.width( highlightString );
	 const QString t = block.text();
	QTextCursor cursor = textCursor();
	Qt::CaseSensitivity caseSensitive = getSearchFlags().testFlag(QTextDocument::FindCaseSensitively)?
		Qt::CaseSensitive : Qt::CaseInsensitive;
	bool searchWholeWords = getSearchFlags().testFlag(QTextDocument::FindWholeWords);
	QRect r;
	
	int k=0;
	int t_len = t.length();
	do
	{
		k = t.indexOf( highlightString, k, caseSensitive );
		if (k == -1)
			break;
			
		if (searchWholeWords && (!isFullWord( t, highlightString, k )))
		{
			k = k + highlightString.length();
			continue;
		}

		cursor.setPosition(block.position()+k+1, QTextCursor::MoveAnchor);
		r = cursorRect( cursor );
		p.setOpacity( 0.7 );
		p.fillRect(r.x()-1, r.y(), highlightStringLen, r.height(), Qt::yellow );
		p.setOpacity( 1 );
		k = k + highlightString.length();
	} while(k< t_len);
}

void	QsvEditor::printMargins( QPainter &p )
{
	int lineLocation;

	p.setFont( currentFont() );
	p.setPen( whiteSpaceColor );
	lineLocation = p.fontMetrics().width( " " ) * printMarginWidth + 0;
	p.drawLine( lineLocation, 0, lineLocation, height() );
}

void	QsvEditor::widgetToBottom( QWidget *w )
{
	w->adjustSize();
	QRect r1 = viewport()->geometry();
	QRect r2 = w->geometry();

	w->adjustSize();
	int i = r2.height();
	r2.setX( r1.left() + 5 );
	r2.setY( r1.height() - i - 5 );
	r2.setWidth( r1.width() - 10 );
	r2.setHeight( i );
	
	w->setGeometry(r2);
	w->show();
}

void	QsvEditor::widgetToTop( QWidget *w )
{
	QRect r1 = viewport()->geometry();
	QRect r2 = w->geometry();

	w->adjustSize();
	int i = r2.height();
	r2.setX( r1.left() + 5 );
	r2.setY( 5 );
	r2.setWidth( r1.width() - 10 );
	r2.setHeight( i );
	
	w->setGeometry(r2);
	w->show();
}

bool	QsvEditor::handleKeyPressEvent( QKeyEvent *event )
{
	QTextCursor cursor = textCursor();
	int i,j;
	QString s;
	
	// handle automatic deletcion of mathcing brackets
	if ( (event->key() == Qt::Key_Delete) || (event->key() == Qt::Key_Backspace) )
	{
		if (cursor.hasSelection())
			return false;
		
		i = cursor.position() - cursor.block().position();
		QChar c1 = cursor.block().text()[ i ];
		j = matchingString.indexOf( c1 );
		if (j == -1)
			return false;
		
		if (i == 0)
			return false;
		
		if (matchingString[j+1] == matchingString[j])
			j++;
		
		QChar c2 = cursor.block().text()[ i-1 ];
		if (c2 != matchingString[j-1])
			return false;
		cursor.deletePreviousChar();
		cursor.deleteChar();
		matchStart = matchEnd = -1;
		//matchChar = 0;
		charStart = charEnd = 0;

		goto FUNCTION_END;
	}
	
	// handle only normal key presses
	s = event->text();
	if (s.isEmpty())
		return false;
	
	// don't handle if not in the matching list
	j = matchingString.indexOf( s[0] );
	if ((j == -1) || (j%2 == 1))
		return false;
	
	i = cursor.position();
	
	// handle automatic insert of matching brackets
	if (!cursor.hasSelection())
	{
		cursor.insertText( QString(matchingString[j]) );
		cursor.insertText( QString(matchingString[j+1]) );
	}
	else
	{
		QString s = cursor.selectedText();
		cursor.beginEditBlock();
		cursor.deleteChar();
		s = matchingString[j] + s + matchingString[j+1];
		cursor.insertText(s);
		cursor.endEditBlock();
	}
	cursor.setPosition( i + 1 );
	setTextCursor(cursor);
	
FUNCTION_END:
	event->accept();
	return true;
}

bool	QsvEditor::handleIndentEvent( bool forward )
{
	QTextCursor cursor1 = textCursor();
	bool reSelect = true;

	if (!cursor1.hasSelection())
	{
		reSelect = false;
		cursor1.select( QTextCursor::LineUnderCursor );
	}
	
	QTextCursor cursor2 = cursor1;
	cursor1.setPosition( cursor1.selectionStart() );
	cursor2.setPosition( cursor2.selectionEnd() );
	int endBlock = cursor2.blockNumber();
	
	QString spaces;
	if (insertSpacesInsteadOfTabs)
	{
		int k = getTabSize();
		for(int i=0; i< k; i++ )
			spaces = spaces.insert(0,' ');
			
	}
	
	int baseIndentation = getIndentationSize( cursor1.block().text() );
	
	if (forward)
		baseIndentation++;
	else if (baseIndentation!=0) 
		baseIndentation--;

	cursor1.beginEditBlock();
	int origPos = cursor1.position();
	do
	{
		int pos = cursor1.position();
		QString s = cursor1.block().text();
		cursor1.select( QTextCursor::LineUnderCursor );
		
		if (forward)
			cursor1.insertText( insertSpacesInsteadOfTabs? spaces + s : '\t' + s );
		else
			cursor1.insertText( updateIndentation(s,baseIndentation) );

		cursor1.setPosition( pos );
		if (!cursor1.movePosition(QTextCursor::NextBlock))
			break;
	} while( cursor1.blockNumber() < endBlock );
	
	if (reSelect)
		cursor1.setPosition( origPos, QTextCursor::KeepAnchor );
	else
		cursor1.setPosition( origPos, QTextCursor::MoveAnchor );
	cursor1.endEditBlock();
	setTextCursor( cursor1 );
	return true;
}

bool	QsvEditor::issue_search( const QString &text, QTextCursor newCursor, QFlags<QTextDocument::FindFlag> findOptions  )
{
	QTextCursor c = document()->find( text, newCursor, findOptions );
	bool found = ! c.isNull();
	
	if (!found)
	{
		//lets try again, from the start
		c.movePosition(findOptions.testFlag(QTextDocument::FindBackward)? QTextCursor::End : QTextCursor::Start);
		c = document()->find( ui_findWidget.searchText->text(), c, findOptions );
		found = ! c.isNull();
	}
	
	if (found)
	{
		QPalette ok = this->palette();
		ok.setColor( QPalette::Base, searchFoundColor );
		ui_findWidget.searchText->setPalette( ok );
		setTextCursor( c );
	}
	else
	{
		QPalette bad = this->palette();
		bad.setColor( QPalette::Base, searchNotFoundColor );
		ui_findWidget.searchText->setPalette( bad );	
		setTextCursor( searchCursor );
	}

	return found;
}

void	QsvEditor::findMatching( QChar c1, QChar c2, bool forward, QTextBlock &block )
{
	int i = matchStart;
	int n = 1;
	QChar c;
	QString blockString = block.text();
	
	do
	{
		if (forward)
		{
			i ++;
			if ((i - block.position()) == block.length())
			{
				block = block.next();
				blockString = block.text();
			}
		}
		else
		{
			i --;
			if ((i - block.position()) == -1)
			{
				block = block.previous();
				blockString = block.text();
			}
		}
		if (block.isValid())
			c = blockString[i - block.position()];
		else
			break;

		if (c == c1)
			n++;
		else if (c == c2)
			n--;
	} while (n!=0);
	
	if (n == 0)
		matchEnd = i;
	else
		matchEnd = -1;
}

void	QsvEditor::findMatching( QChar c, QTextBlock &block )
{
	int n = 0;
	QString blockString = block.text();
	int blockLen = block.length();
	
	// try forward
	while (n < blockLen) 
	{
		if ( (n != matchStart - block.position()) && (blockString[n] == c))
		{
			matchEnd = block.position() + n;
			return;
		}
		n++;
	}
}

QsvPrivateBlockData* QsvEditor::getPrivateBlockData( QTextBlock block, bool createIfNotExisting )
{
	QTextBlockUserData *d1 = block.userData();
	QsvPrivateBlockData *data = dynamic_cast<QsvPrivateBlockData*>( d1 );
	
	// a user data has been defined, and it's not our structure
	if (d1 && !data)
		return NULL;
	
	if (!data && createIfNotExisting)
	{
		data = new QsvPrivateBlockData;
		block.setUserData( data );
	}
	return data;
}

QTextCursor	QsvEditor::getCurrentTextCursor()
{
	QTextCursor cursor = textCursor();
	if (!cursor.hasSelection())
		cursor.select(QTextCursor::WordUnderCursor);
	return cursor;
}


QFlags<QTextDocument::FindFlag> QsvEditor::getSearchFlags()
{
	QFlags<QTextDocument::FindFlag> f;
	
	if (ui_findWidget.caseSensitive->isChecked())
		f = f | QTextDocument::FindCaseSensitively;
	
	if (ui_findWidget.wholeWords->isChecked())
		f = f | QTextDocument::FindWholeWords;
	
	return f;
}

QFlags<QTextDocument::FindFlag> QsvEditor::getReplaceFlags()
{
	QFlags<QTextDocument::FindFlag> f;
	
	if (ui_replaceWidget.caseSensitive->isChecked())
		f = f | QTextDocument::FindCaseSensitively;
	
	if (ui_replaceWidget.wholeWords->isChecked())
		f = f | QTextDocument::FindWholeWords;

	if (ui_replaceWidget.findBackwards->isChecked())
		f = f | QTextDocument::FindBackward;
	
	return f;
}
