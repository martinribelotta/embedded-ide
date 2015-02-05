#ifndef QSVTEXTEDIT_H
#define QSVTEXTEDIT_H

#include <QPlainTextEdit>
#include <QTextCharFormat>
#include <QColor>
#include <QList>
#include <QTimer>

class QsvSyntaxHighlighterBase;
class QsvEditorPanel;
class QFileSystemWatcher;
class QLabel;

// Unused yet
enum EndOfLineType {
	DOS, Unix, Mac, KeepOldStyle
};

struct QsvEditorConfigData {
	bool			markCurrentLine;
	bool			smartHome;
	bool			matchBrackets;
	QString			matchBracketsList;
	QFont			currentFont;
	bool			lineWrapping;
	bool			modificationsLookupEnabled;
	bool			autoBrackets;
	bool			showLineNumbers;
	bool			showMargins;
	uint			marginsWidth;
	int			tabSize;
	bool			insertSpacesInsteadOfTabs;
	bool			tabIndents;
	bool			showWhiteSpaces;
	
//	EndOfLineType		endOfLine;
//	QsvColorDefFactory	*currentColorScheme;
};

namespace Ui{
	class BannerMessage;
};

class QsvTextEdit : public QPlainTextEdit
{
	Q_OBJECT
public:
	QsvTextEdit( QWidget *parent = 0, QsvSyntaxHighlighterBase *s = 0 );
	void setupActions();

	void setMarkCurrentLine( bool on );
	bool getmarkCurrentLine() const;
	void setStartHome( bool on );
	bool getStartHome() const;
	void setMatchBracketList( const QString &m );
	const QString getMatchBracketList() const;
	void setMatchBracket( bool on );
	bool getMatchBracket() const;
	void setLineWrapping( bool on );
	bool getLineWrapping() const;
	void setModificationsLookupEnabled( bool on );
	bool getModificationsLookupEnabled() const;
	void setShowLineNumbers( bool on );
	bool getShowLineNumbers() const;
	void setShowMargins( bool on );
	bool getShowMargins() const;
	void setMarginsWidth( uint i );
	uint getMarginsWidth() const;
	void setTabSize( int size );
	int  getTabSize() const;
	void setInsertSpacesInsteadOfTabs( bool on );
	bool getInsertSpacesInsteadOfTabs() const;
	void setTabIndents( bool on );
	bool getTabIndents() const;
	void setShowWhiteSpace( bool on );
	bool getShowWhiteSpace() const;
	void setDefaultConfig();
	void setHighlighter(QsvSyntaxHighlighterBase *s);
	QsvSyntaxHighlighterBase* getHighlighter() const;
	static void setDefaultConfig( QsvEditorConfigData *config );
    QString getFileName(){ return m_fileName; }
	
	QTextCursor getCurrentTextCursor();
	void paintPanel(QPaintEvent *e);
	
	int loadFile(const QString &fileName);
	int saveFile(const QString &fileName);
	void displayBannerMessage(QString message,int time=15);
	void hideBannerMessage();
	
public slots:
	void newDocument();
	int saveFile();
	int saveFileAs();
	void smartHome();
	void smartEnd();
	void transformBlockToUpper();
	void transformBlockToLower();
	void transformBlockCase();
	void gotoMatchingBracket();
	void toggleBookmark();
	void gotoNextBookmark();
	void gotoPrevBookmark();
	void updateExtraSelections();
	
	void removeModifications();
	void on_cursor_positionChanged();
	void on_textDocument_contentsChange( int position, int charsRemoved, int charsAdded );
	void on_hideTimer_timeout();
	void adjustBottomAndTopWidget();
	void on_fileChanged(const QString &filename);
	void on_fileMessage_clicked(const QString &s);
	
signals:
	void widgetResized();

public:
	QAction *actionCapitalize;
	QAction *actionLowerCase;
	QAction *actionChangeCase;
	QAction *actionFindMatchingBracket;
	QAction *actionToggleBookmark;
	QAction *actionNextBookmark;
	QAction *actionPrevBookmark;
	
	void showUpperWidget(QWidget* w);
	void showBottomWidget(QWidget* w);
protected:
	void paintEvent(QPaintEvent *e);
	void resizeEvent(QResizeEvent *e);
	void keyPressEvent(QKeyEvent *e);
	bool handleKeyPressEvent(QKeyEvent *e);
	virtual void updateMargins();

	bool handleIndentEvent( bool forward );
	int getIndentationSize( const QString s );
	QString updateIndentation( QString s, int indentation );
	int findMatchingChar( QChar c1, QChar c2, bool forward, QTextBlock &block, int from );
	QTextEdit::ExtraSelection getSelectionForBlock( QTextCursor &cursor, QTextCharFormat &format );
	void resetExtraSelections();

	QsvSyntaxHighlighterBase *m_highlighter;
	QsvEditorPanel *m_panel;
	QWidget *m_banner;
	Ui::BannerMessage *ui_banner;
	
	QWidget         *m_topWidget;
	QWidget         *m_bottomWidget;

	QList<QTextEdit::ExtraSelection> m_selections;
	QTextCharFormat m_matchesFormat;
	QColor          m_currentLineBackground;
	QColor          m_bookmarkColor;
	QColor          m_panelColor;
	QColor          m_modifiedColor;
	QPixmap         m_bookMarkImage;
	int             m_timerHideout;
	QString         m_fileName;
	QFileSystemWatcher *m_fileSystemWatcher;
	
	QTimer m_selectionTimer;

	QsvEditorConfigData m_config;
};

#endif // QSVTEXTEDIT_H
