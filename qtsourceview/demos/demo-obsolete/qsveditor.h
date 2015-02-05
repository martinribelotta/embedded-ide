#ifndef __QSV_EDITOR_H__
#define __QSV_EDITOR_H__

/*
#if QT_VERSION >= 0x040400
#	define	QTextEditorControl	QPlainTextEdit
#	warning	"Using QPlainTextEdit as the text editor control"
#else
#	define	QTextEditorControl	QTextEdit
#endif
*/

#define	QTextEditorControl	QTextEdit
#include <QTextEdit>

#include "ui_findwidget.h"
#include "ui_replacewidget.h"
#include "ui_gotolinewidget.h"
#include "ui_filemessage.h"

class QsvSyntaxHighlighter;
class QFileSystemWatcher;
class QTextCursor;
class QTextCodec;

#include <QTextDocument>
//enum QTextDocument::FindFlag;

class QsvEditorPanel;
class QsvPrivateBlockData;
class QsvTransparentWidget;
class QsvColorDefFactory;

bool isFullWord( QString s1, QString s2, int location );

enum EndOfLineType {
	DOS, Unix, Mac, KeepOldStyle
};


struct QsvEditorConfigData
{
	bool			autoBrackets;
	bool			markCurrentLine;
	bool			showLineNumbers;
	bool			showWhiteSpaces;
	bool			showMargins;
	bool			matchBrackets;
	bool			lineWrapping;
	bool			smartHome;
	bool			insertSpacesInsteadOfTabs;
	int			tabSize;
	int			marginsWidth;
	QString			matchBracketsList;
	QFont			currentFont;
	EndOfLineType		endOfLine;
	QsvColorDefFactory	*currentColorScheme;
};

class QsvEditor: public QTextEditorControl
{
	Q_OBJECT
public:
	enum ItemColors {
		 LinesPanel, CurrentLine, MatchBrackets, NoText, TextFound, TextNoFound, WhiteSpaceColor, 
		 BookmarkLineColor, BreakpointLineColor, ModifiedColor
	};
	
	enum BookmarkAction {
		Toggle, Enable, Disable
	};
	
	QsvEditor( QWidget *p=NULL );
	virtual		~QsvEditor();
	void		setupActions();

	QColor		getItemColor( ItemColors role );
	void		setItemColor( ItemColors role, QColor );
	int		getMargin();
	void		setMargin( int width );
	int		getTabSize();
	void		setTabSize( int size );
	bool		getInsertSpacesInsteadOfTabs();
	void		setInsertSpacesInsteadOfTabs(bool);
	QsvSyntaxHighlighter*	getSyntaxHighlighter();
	void		setSyntaxHighlighter( QsvSyntaxHighlighter *newSyntaxHighlighter );
	QTextCursor	getCurrentTextCursor();
	bool		getDisplayCurrentLine();
	void		setDisplayCurrentLine( bool );
	bool		getDisplayWhiteSpaces();
	void		setDisplayWhiteSpaces( bool );
	bool		getDisplayMatchingBrackets();
	void		setDisplayMatchingBrackets( bool );
	QString		getMatchingString();
	void		setMatchingString( QString );
	bool		getUsingSmartHome();
	void		setUsingSmartHome( bool );
	bool		getUsingAutoBrackets();
	void		setUsingAutoBrackets( bool );
	EndOfLineType	getEndOfLine();
	void		setEndOfLine( EndOfLineType );
	QTextCodec*	getTextCodec();
	void		setTextCodec( QTextCodec* );
		
	void		setBookmark( BookmarkAction action, QTextBlock block );
	void		setBreakpoint( BookmarkAction action, QTextBlock block );
	QWidget*	getPanel();
	void		displayBannerMessage( QString );
	void		hideBannerMessage();
	void		clearEditor();
	int		loadFile( QString );
	int		saveFile( QString );
	QString		getFileName();
	void		removeModifications();
	int		getIndentationSize( const QString s );
	QString		updateIndentation( QString s, int indentation );
	
public slots:
	void		applyConfiguration( QsvEditorConfigData c );
	void		pauseFileSystemWatch();
	void		resumeFileSystemWatch();
	void		pauseModificationsLookup();
	void		resumeModificationsLookup();
	void		adjustMarginWidgets();
	void		showFindWidget();
	void		showReplaceWidget();
	void		showGotoLineWidget();
	void		clearSearchHighlight();
	void		findNext();
	void		findPrev();
	void		toggleBookmark();
	void		toggleBreakpoint();
	void		gotoPrevBookmark();
	void		gotoNextBookmark();
	void		transformBlockToUpper();
	void		transformBlockToLower();
	void		transformBlockCase();
	void		smartHome();
	void		smartEnd();
	void		findMatchingBracket();
	
	void		updateCurrentLine();
	void		on_searchText_textChanged( const QString & text );
	void		on_searchText_editingFinished();
	void		on_replaceWidget_expand( bool checked );
	void		on_replaceOldText_textChanged( const QString & text );
	void		on_replaceOldText_returnPressed();
	void		on_replaceAll_clicked();
	void		on_lineNumber_editingFinished();
	void		on_lineNumber_valueChanged(int i);
	void		on_cursorPositionChanged();
	void		on_textDocument_contentsChange( int position, int charsRemoved, int charsAdded );
	void		on_fileChanged( const QString &fName );
	void		on_fileMessage_clicked( QString s );
	
protected:
	void		keyPressEvent ( QKeyEvent *event );
	void		resizeEvent ( QResizeEvent *event );
	void		timerEvent( QTimerEvent *event );

	void		paintEvent(QPaintEvent *e);
	void		printBackgrounds( QPainter &p );
	void		printWhiteSpaces( QPainter &p, const QTextBlock &block, const QFontMetrics &fm );
	void		printCurrentLines( QPainter &p, const QTextBlock &block );
	void		printMatchingBraces( QPainter &p );
	void		printHighlightString( QPainter &p, const QTextBlock &block, const QFontMetrics &fm );
	void		printMargins( QPainter &p );
	
	void		widgetToBottom( QWidget *w );
	void		widgetToTop( QWidget *w );
	bool		handleKeyPressEvent( QKeyEvent *event );
	bool		handleIndentEvent( bool forward );
	bool		issue_search( const QString &text, QTextCursor newCursor, QFlags<QTextDocument::FindFlag> findOptions );
	virtual void	findMatching( QChar c1, QChar c2, bool forward, QTextBlock &block );
	virtual void	findMatching( QChar c, QTextBlock &block );
	QsvPrivateBlockData*	getPrivateBlockData( QTextBlock block, bool createIfNotExisting=false );
	QFlags<QTextDocument::FindFlag> getSearchFlags();
	QFlags<QTextDocument::FindFlag> getReplaceFlags();

public:
	QAction	*actionFind;
	QAction	*actionFindNext;
	QAction	*actionFindPrev;
	QAction	*actionClearSearchHighlight;
	QAction	*actionReplace;
	QAction	*actionGotoLine;
	
	QAction	*actionNextBookmark;
	QAction	*actionPrevBookmark;
	QAction	*actionCapitalize;
	QAction	*actionLowerCase;
	QAction	*actionChangeCase;
	QAction	*actionToggleBookmark;
	QAction	*actionTogglebreakpoint;
	QAction	*actionFindMatchingBracket;

private:
	QColor	currentLineColor;
	QColor	bookmarkLineColor;
	QColor	breakpointLineColor;
	QColor	matchBracesColor;
	QColor	searchFoundColor;
	QColor	searchNotFoundColor;
	QColor	searchNoText;
	QColor	whiteSpaceColor;
	
	bool	highlightCurrentLine;
	bool	showWhiteSpaces;
	bool	showMatchingBraces;
	bool	showPrintingMargins;
	bool	usingSmartHome;
	bool	usingAutoBrackets;
	bool	modificationsLookupEnabled;
	bool	insertSpacesInsteadOfTabs;
	int	printMarginWidth;
	QString	matchingString;
	EndOfLineType	endOfLine;
	QTextCodec	*textCodec;
	
	bool	ignoreFileSystemWatch;
	int	matchStart;
	int	matchEnd;
	QChar	charStart;
	QChar	charEnd;
	QString	highlightString;
	QString	fileName;
	QFileSystemWatcher	*fileSystemWatcher;
	
	QsvSyntaxHighlighter	*syntaxHighlighter;	
	QTextCursor		searchCursor;
	QsvEditorPanel		*panel;
	
	QsvTransparentWidget	*findWidget;
	QsvTransparentWidget	*replaceWidget;
	QsvTransparentWidget	*gotoLineWidget;
	QsvTransparentWidget	*fileMessage;
	Ui::FindWidget		ui_findWidget;
	Ui::ReplaceWidget	ui_replaceWidget;
	Ui::GotoLineWidget	ui_gotoLineWidget;
	Ui::FileMessage		ui_fileMessage;
};

#endif // __QSV_EDITOR_H__
