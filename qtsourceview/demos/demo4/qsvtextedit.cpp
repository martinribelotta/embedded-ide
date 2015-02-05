#include <QApplication>
#include <QScrollBar>
#include <QAbstractSlider>
#include <QList>
#include <QPainter>
#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QTextStream>
#include <QDebug>
#include <QLabel>
#include <QFileSystemWatcher>
#include <QFileDialog>

#include "qsvtextedit.h"
#include "qsvsyntaxhighlighterbase.h"
#include "ui_bannermessage.h"

// private class
class QsvEditorPanel : public QWidget
{
public:
	QsvEditorPanel(QsvTextEdit *editor)
	 :QWidget(editor)
	{
		setFixedWidth(50);
	}

private:
	void paintEvent(QPaintEvent*e)
	{
		((QsvTextEdit*)parent())->paintPanel(e);
	}
friend class QsvTextEdit;
};


QsvTextEdit::QsvTextEdit( QWidget *parent, QsvSyntaxHighlighterBase *s ):
	QPlainTextEdit(parent)
{
	m_highlighter = s;
	if (m_highlighter)
		m_highlighter->setTextDocument( document() );
	m_panel = new QsvEditorPanel(this);
	m_banner = new QWidget(this);
	m_banner->setFont(QApplication::font());
	m_banner->hide();
	m_banner->setObjectName("banner");
	ui_banner = new Ui::BannerMessage;
	ui_banner->setupUi(m_banner);
	m_topWidget = NULL;
	m_bottomWidget = NULL;
	if (ui_banner->frame->style()->inherits("QWindowsStyle"))
		ui_banner->frame->setFrameStyle(QFrame::StyledPanel);
	m_selectionTimer.setSingleShot(true);
	m_selectionTimer.setInterval(200);
	
	m_fileName.clear();
	m_fileSystemWatcher = new QFileSystemWatcher(this);
	
	connect(&m_selectionTimer,SIGNAL(timeout()),this,SLOT(updateExtraSelections()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(on_cursor_positionChanged()));
	connect(this, SIGNAL(widgetResized()),this,SLOT(adjustBottomAndTopWidget()));
	connect(ui_banner->label,SIGNAL(linkActivated(QString)),this,SLOT(on_fileMessage_clicked(QString)));
	connect(document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(on_textDocument_contentsChange(int,int,int)));
	connect(verticalScrollBar(), SIGNAL(valueChanged(int)), m_panel, SLOT(update()));
	connect(m_fileSystemWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(on_fileChanged(const QString&)));
	
	QFont f;
	f.setBold(true);
	m_matchesFormat = QTextCharFormat();
	m_matchesFormat.setBackground( QBrush(QColor(0xff,0xff,0x00,0xff) ));
	m_matchesFormat.setForeground( QBrush(QColor(0x00,0x80,0x00,0xff) ));
	m_matchesFormat.setFont(f);
	m_currentLineBackground   = QColor(0xc0,0xff,0xc0,0x80);
	m_bookmarkColor           = QColor(0xa0,0xa0,0xff,0x80);
	m_modifiedColor           = QColor(0x00,0xff,0x00,0xff);
	m_panelColor              = QColor(0xff,0xff,0xd0,0xff);
	
	actionCapitalize          = NULL;
	actionLowerCase           = NULL;
	actionChangeCase          = NULL;
	actionFindMatchingBracket = NULL;
	actionToggleBookmark      = NULL;
	actionNextBookmark        = NULL;
	actionPrevBookmark        = NULL;

	setDefaultConfig();
	setupActions();
	updateMargins();
	setMatchBracketList(m_config.matchBracketsList);
	setFont(m_config.currentFont);
	setLineWrapMode(QPlainTextEdit::NoWrap);
	setFrameStyle(QPlainTextEdit::NoFrame);
}

void	QsvTextEdit::setupActions()
{
	if (actionCapitalize) delete actionCapitalize;
	actionCapitalize = new QAction( tr("Change to &capital letters"), this );
	actionCapitalize->setObjectName("qsvEditor::actionCapitalize");
	actionCapitalize->setShortcut( QKeySequence( Qt::CTRL | Qt::Key_U ) );
	connect( actionCapitalize, SIGNAL(triggered()), this, SLOT(transformBlockToUpper()) );
	addAction(actionCapitalize);

	if (actionLowerCase) delete actionLowerCase;
	actionLowerCase = new QAction( tr("Change to &lower letters"), this );
	actionLowerCase->setObjectName( "qsvEditor::actionLowerCase" );
	actionLowerCase->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U));
	connect( actionLowerCase, SIGNAL(triggered()), this, SLOT(transformBlockToLower()) );
	addAction(actionLowerCase);

	if (actionChangeCase) delete actionChangeCase;
	actionChangeCase = new QAction( tr("Change ca&se"), this );
	actionChangeCase->setObjectName( "qsvEditor::actionChangeCase" );
	connect( actionChangeCase, SIGNAL(triggered()), this, SLOT(transformBlockCase()) );
	addAction(actionChangeCase);
	
	if (actionFindMatchingBracket) delete actionFindMatchingBracket;
	actionFindMatchingBracket = new QAction(tr("Find matching bracket"), this);
	actionFindMatchingBracket->setObjectName("qsvEditor::ctionFindMatchingBracket");
	actionFindMatchingBracket->setShortcuts( 
		QList<QKeySequence>() 
		 << QKeySequence(Qt::CTRL | Qt::Key_6)
		 << QKeySequence(Qt::CTRL | Qt::Key_BracketLeft)
		 << QKeySequence(Qt::CTRL | Qt::Key_BracketRight)
	);
	connect( actionFindMatchingBracket, SIGNAL(triggered()), this, SLOT(gotoMatchingBracket()) );
	addAction(actionFindMatchingBracket);
	
	if (actionToggleBookmark) delete actionToggleBookmark;
	actionToggleBookmark = new QAction( tr("Change ca&se"), this );
	actionToggleBookmark->setObjectName("qsvEditor::actionToggleBookmark");
	actionToggleBookmark->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
	connect(actionToggleBookmark, SIGNAL(triggered()), this, SLOT(toggleBookmark()));
	addAction(actionToggleBookmark);
	
	if (actionNextBookmark) delete actionNextBookmark;
	actionNextBookmark = new QAction( tr("Goto Next bookmark"), this );
	actionNextBookmark->setObjectName("qsvEditor::actionNextBookmark");
	actionNextBookmark->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageDown));
	connect(actionNextBookmark, SIGNAL(triggered()), this, SLOT(gotoNextBookmark()));
	addAction(actionNextBookmark);
	
	if (actionPrevBookmark) delete actionPrevBookmark;
	actionPrevBookmark = new QAction( tr("Goto Previous bookmark"), this );
	actionPrevBookmark->setObjectName("qsvEditor::actionPrevBookmark");
	actionPrevBookmark->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageUp));
	connect(actionPrevBookmark, SIGNAL(triggered()), this, SLOT(gotoPrevBookmark()));
	addAction(actionPrevBookmark);
}

void	QsvTextEdit::setMarkCurrentLine( bool on )
{
	m_config.markCurrentLine = on;
	on_cursor_positionChanged();
}

bool	QsvTextEdit::getmarkCurrentLine() const
{
	return m_config.markCurrentLine;
}

void	QsvTextEdit::setStartHome( bool on )
{
	m_config.smartHome = on;
}

bool	QsvTextEdit::getStartHome() const
{
	return m_config.smartHome;
}

void	QsvTextEdit::setMatchBracketList( const QString &m )
{
	m_config.matchBracketsList = m;
	if (m_highlighter)
		m_highlighter->setMatchBracketList( m );
}

const QString QsvTextEdit::getMatchBracketList() const
{
	return m_config.matchBracketsList;
}

void	QsvTextEdit::setMatchBracket( bool on )
{
	m_config.matchBrackets = on;
	on_cursor_positionChanged();
}

bool	QsvTextEdit::getMatchBracket() const
{
	return m_config.matchBrackets;
}

void	QsvTextEdit::setLineWrapping( bool on )
{
	m_config.lineWrapping = on;
	setLineWrapMode(on?QPlainTextEdit::NoWrap:QPlainTextEdit::WidgetWidth);
}

bool	QsvTextEdit::getLineWrapping() const
{
	return m_config.lineWrapping;
}

void	QsvTextEdit::setModificationsLookupEnabled( bool on )
{
	m_config.modificationsLookupEnabled = on;
	if (m_panel)
		m_panel->update();
}

bool	QsvTextEdit::getModificationsLookupEnabled() const
{
	return m_config.modificationsLookupEnabled;
}

void	QsvTextEdit::setShowLineNumbers( bool on )
{
	m_config.showLineNumbers = on;
	m_panel->setVisible(on);
	updateMargins();
}

bool	QsvTextEdit::getShowLineNumbers() const
{
	return m_config.showLineNumbers;
}

// helper function, in Pascal it would have been an internal
// procedure inside on_cursor_moved();
// TODO move it to the class, as a helper method by adding extra selections
// to the class
inline void appendExtraSelection( QList<QTextEdit::ExtraSelection> &selections,
	int position, QPlainTextEdit *self, QTextCharFormat matchesFormat )
{
	if (position==-1)
		return;
	QTextEdit::ExtraSelection selection;
	QTextCursor cursor = self->textCursor();
	selection.format = matchesFormat;
	cursor.setPosition(position);
	cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
	selection.cursor = cursor;
	selections.append(selection);
}

void QsvTextEdit::on_cursor_positionChanged()
{
	// update the extra selections only 5 times per second
	if (m_selectionTimer.isActive())
		m_selectionTimer.stop();
	m_selectionTimer.start();
	
	// but, do mark the current line all times:
	QList<QTextEdit::ExtraSelection> selections;
	selections = m_selections;
	if (m_config.markCurrentLine) {
		QTextCursor cursor = textCursor();
		QTextCharFormat format;
		format.setBackground(m_currentLineBackground);
		selections.append(getSelectionForBlock(cursor,format));
	}
	setExtraSelections(selections);
	
	if (m_panel)
		m_panel->update();
}

void	QsvTextEdit::newDocument()
{
	loadFile("");
}

int	QsvTextEdit::loadFile(const QString &fileName)
{
	// clear older watches, and add a new one
	QStringList sl = m_fileSystemWatcher->directories();
	if (!sl.isEmpty())
		m_fileSystemWatcher->removePaths(sl);

	bool modificationsEnabledState = getModificationsLookupEnabled();
	setModificationsLookupEnabled(false);
	hideBannerMessage();
	this->setReadOnly(false);
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QApplication::processEvents();
	if (!fileName.isEmpty()) {
		QFile file(fileName);
		QFileInfo fileInfo(file);
		
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			return -1;
		
//		QTextCodec *c = m_config.textCodec;
		QTextCodec *c = NULL;
		if (c == NULL)
			c = QTextCodec::codecForLocale();

		QTextStream textStream(&file);
		textStream.setCodec( c );
		setPlainText( textStream.readAll() );
		file.close();
	
		m_fileName = fileInfo.absoluteFilePath();
		m_fileSystemWatcher->addPath(m_fileName);
		if (!fileInfo.isWritable()) {
			this->setReadOnly( true);
			displayBannerMessage( tr("The file is readonly. Click <a href=':forcerw' title='Click here to try and change the file attributes for write access'>here to force write access.</a>") );
		}
	} else {
		m_fileName.clear();
		clear();
	}
	
	setModificationsLookupEnabled(modificationsEnabledState);
	removeModifications();

	QApplication::restoreOverrideCursor();
	return 0;
}

int	QsvTextEdit::saveFile(const QString &fileName)
{
	QStringList sl = m_fileSystemWatcher->directories();
	if (!sl.isEmpty())
		m_fileSystemWatcher->removePaths(sl);
	bool modificationsEnabledState = getModificationsLookupEnabled();
	setModificationsLookupEnabled(false);
	hideBannerMessage();
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QApplication::processEvents();

	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly))
		return false;

	QTextCodec *c = NULL;
	//	QTextCodec *c = textCodec;
	if (c == NULL)
		c = QTextCodec::codecForLocale();

	QTextStream textStream(&file);
	textStream.setCodec(c);

	QTextBlock block = document()->begin();
	while(block.isValid()) {
//		if (endOfLine==KeepOldStyle){
			textStream << block.text();
			// TODO WTF? - which type the file originally had?
			textStream << "\n";
/*		} else {
			QString s = block.text();
			int i = s.length();

			if (!s.isEmpty()) if ((s[i-1] == '\n') || (s[i-1] == '\r'))
				s = s.left( i-1 );
			if (!s.isEmpty()) if ((s[i-1] == '\n') || (s[i-1] == '\r'))
				s = s.left( i-1 );
			textStream << s;
			switch (endOfLine) {
				case DOS:	textStream << "\r\n"; break;
				case Unix: 	textStream << "\n"; break;
				case Mac:	textStream << "\r"; break;
				default:	return 0; // just to keep gcc happy
			}
		}*/
		block = block.next();
	}
	file.close();

	m_fileName = fileName;
	removeModifications();
	setModificationsLookupEnabled(modificationsEnabledState);
//	m_fileSystemWatcher->addPath(m_fileName);
	
	QApplication::restoreOverrideCursor();
	return 0;
}

void	QsvTextEdit::displayBannerMessage(QString message,int time)
{
	showUpperWidget(m_banner);
	ui_banner->label->setText(message);
	m_timerHideout = time;
	QTimer::singleShot(1000,this,SLOT(on_hideTimer_timeout()));
}

void	QsvTextEdit::hideBannerMessage()
{
	m_timerHideout = 0;
	ui_banner->label->clear();
	m_banner->hide();
	
	// sometimes the top widget is displayed, lets workaround this
	// TODO: find WTF this is happening
	if (m_topWidget == m_banner)
		m_topWidget = NULL;
}

int	QsvTextEdit::saveFile()
{
	if (m_fileName.isEmpty()){
		return saveFileAs();
	} else
		return saveFile(m_fileName);
}

int	QsvTextEdit::saveFileAs()
{
	const QString lastDirectory;
	QString s = QFileDialog::getSaveFileName(this,tr("Save file"),lastDirectory);
	if (s.isEmpty())
		return false;
	return saveFile(s);
}

void	QsvTextEdit::smartHome()
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
	while ( c.block().text()[i].isSpace()) {
		i ++;
		if (i==blockLen) {
			i = 0;
			break;
		}
	}
	if ((originalPosition == startOfLine) || (startOfLine + i != originalPosition ))
		c.setPosition( startOfLine + i, moveAnchor );
	setTextCursor( c );
}

void	QsvTextEdit::smartEnd()
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
	while (c.block().text()[i-1].isSpace())	{
		i --;
		if (i==1)		{
			i = blockLen;
			break;
		}
	}
	if ((originalPosition == startOfLine) || (startOfLine + i != originalPosition ))
		c.setPosition( startOfLine + i, moveAnchor );

	setTextCursor( c );
}

void	QsvTextEdit::transformBlockToUpper()
{
	QTextCursor cursor = getCurrentTextCursor();
	QString s_before   = cursor.selectedText();
	QString s_after    = s_before.toUpper();
	
	if (s_before != s_after) {
		cursor.beginEditBlock();
		cursor.deleteChar();
		cursor.insertText( s_after );
		cursor.endEditBlock();
		setTextCursor( cursor );
	}
}

void	QsvTextEdit::transformBlockToLower()
{
	QTextCursor cursor = getCurrentTextCursor();
	QString s_before   = cursor.selectedText();
	QString s_after    = s_before.toLower();
	
	if (s_before != s_after) {
		cursor.beginEditBlock();
		cursor.deleteChar();
		cursor.insertText( s_after );
		cursor.endEditBlock();
		setTextCursor( cursor );
	}
}

void	QsvTextEdit::transformBlockCase()
{
	QTextCursor cursor = getCurrentTextCursor();
	QString s_before   = cursor.selectedText();
	QString s_after    = s_before;
	uint s_len = s_before.length();
	
	for( uint i=0; i< s_len; i++ ) {
		QChar c = s_after[i];
		if (c.isLower())
			c = c.toUpper();
		else if (c.isUpper())
			c = c.toLower();
		s_after[i] = c;
	}
		
	if (s_before != s_after) {
		cursor.beginEditBlock();
		cursor.deleteChar();
		cursor.insertText( s_after );
		cursor.endEditBlock();
		setTextCursor( cursor );
	}
}

void	QsvTextEdit::gotoMatchingBracket()
{
	/*
	  WARNING: code duplication between this method and on_cursor_positionChanged();
	  this needs to be refactored
	 */
	
	QTextCursor cursor = textCursor();
	QTextBlock  block     = cursor.block();
	int blockPosition     = block.position();
	int cursorPosition    = cursor.position();
	int relativePosition  = cursorPosition - blockPosition;
	QChar currentChar     = block.text()[relativePosition];
	
	// lets find it's partner
	// in theory, no errors shuold not happen, but one can never be too sure
	int j = m_config.matchBracketsList.indexOf(currentChar);
	if (j==-1)
		return;

	if (m_config.matchBracketsList[j] != m_config.matchBracketsList[j+1])
		if (j %2 == 0)
			j = findMatchingChar( m_config.matchBracketsList[j], m_config.matchBracketsList[j+1], true , block, cursorPosition );
		else
			j = findMatchingChar( m_config.matchBracketsList[j], m_config.matchBracketsList[j-1], false, block, cursorPosition  );
	else
		j = findMatchingChar( m_config.matchBracketsList[j], m_config.matchBracketsList[j+1], true , block, cursorPosition );
	
	cursor.setPosition(j);
	setTextCursor(cursor);
}

void 	QsvTextEdit::toggleBookmark()
{
	if (!m_highlighter) {
		displayBannerMessage(tr("DEBUG: Please use a proper QsvSyntaxHighlighter for having bookmarks"));
		return;
	}
	QTextCursor cursor = textCursor();
	QTextBlock  block  = cursor.block();
	m_highlighter->toggleBookmark(block);
	resetExtraSelections();
}

void	QsvTextEdit::updateExtraSelections()
{
	QList<QTextEdit::ExtraSelection> selections;
	QTextCursor cursor = textCursor();
	QTextBlock  block;
	int blockPosition;
	int cursorPosition;
	int relativePosition;
	QChar currentChar;
	Qate::BlockData *data;

	selections = m_selections;
	if (m_config.markCurrentLine) {
		QTextCharFormat format;
		format.setBackground(m_currentLineBackground);
		selections.append(getSelectionForBlock(cursor,format));
	}
	
	/*
	  WARNING: code duplication between this method and gotoMatchingBracket()
	  this needs to be refactored
	 */
	
	if (!m_config.matchBrackets)
		goto NO_MATCHES;
	block             = cursor.block();
	blockPosition     = block.position();
	cursorPosition    = cursor.position();
	relativePosition  = cursorPosition - blockPosition;
	currentChar       = block.text()[relativePosition];
	data              = dynamic_cast<Qate::BlockData*>(block.userData());
	if (data==NULL)
		return;

	for ( int k=0; k<data->matches.length(); k++) {
		Qate::MatchData m = data->matches.at(k);
		if (m.position != relativePosition)
			continue;

		appendExtraSelection(selections, cursorPosition, this, m_matchesFormat);

		// lets find it's partner
		// in theory, no errors shuold not happen, but one can never be too sure
		int j = m_config.matchBracketsList.indexOf(currentChar);
		if (j==-1)
			continue;

		if (m_config.matchBracketsList[j] != m_config.matchBracketsList[j+1])
			if (j %2 == 0)
				j = findMatchingChar( m_config.matchBracketsList[j], m_config.matchBracketsList[j+1], true , block, cursorPosition );
			else
				j = findMatchingChar( m_config.matchBracketsList[j], m_config.matchBracketsList[j-1], false, block, cursorPosition  );
		else
			j = findMatchingChar( m_config.matchBracketsList[j], m_config.matchBracketsList[j+1], true , block, cursorPosition );
		appendExtraSelection(selections, j, this,m_matchesFormat);
	}
NO_MATCHES:
	setExtraSelections(selections);
}

void	QsvTextEdit::removeModifications()
{
	int i = 1;
    if (m_highlighter!=NULL)
	for ( QTextBlock block = document()->begin(); block.isValid(); block = block.next() ) {
		m_highlighter->removeModification(block);
		i ++;
	}
	m_panel->update();
}

void	QsvTextEdit::on_textDocument_contentsChange( int position, int charsRemoved, int charsAdded )
{
	if (!m_config.modificationsLookupEnabled)
		return;

	QTextCursor cursor( document() );
	QTextBlock  block;
	if (charsAdded < 2) {
		block = this->textCursor().block();
        if (m_highlighter)
		m_highlighter->setBlockModified(block, true);
	}
	else {
		int remaining = 0;
		cursor.setPosition( position );
		
		int oldRemaining = -1;
		while ( (remaining+1 < charsAdded) && (oldRemaining != remaining) ){
			oldRemaining = remaining;
			block = cursor.block();
            if (m_highlighter)
			m_highlighter->setBlockModified(block, true);
			cursor.movePosition( QTextCursor::NextBlock );
			remaining = cursor.position() - position;
		}
	}
	m_panel->update();
	Q_UNUSED( charsRemoved );
}

void	QsvTextEdit::on_hideTimer_timeout()
{
	if (m_timerHideout != 0 ){
		QString s;
		s.setNum(m_timerHideout);
		m_timerHideout--;
		ui_banner->timer->setText(s);
		QTimer::singleShot(1000, this, SLOT(on_hideTimer_timeout()));
	}
	else {
		ui_banner->timer->clear();
		m_banner->hide();
	}
}

void	QsvTextEdit::adjustBottomAndTopWidget()
{
	if (m_topWidget){
		QWidget *parent = viewport();
		QRect r = parent->rect();
		m_topWidget->adjustSize();
		r.adjust(10, 0, -10, 0);
		r.setHeight(m_topWidget->height());
		r.moveTop(10);
                r.moveLeft(parent->pos().x()+10);
		m_topWidget->setGeometry(r);
		m_topWidget->show();
	}	
	if (m_bottomWidget){
		QWidget *parent = viewport();
		QRect r = parent->rect();
		m_bottomWidget->adjustSize();
		r.adjust(10, 0, -10, 0);
		r.setHeight(m_bottomWidget->height());
		r.moveBottom(parent->rect().height()-10);
                r.moveLeft(parent->pos().x()+10);
		m_bottomWidget->setGeometry(r);
		m_bottomWidget->show();
	}
}

void	QsvTextEdit::on_fileChanged(const QString &filename)
{
	if (m_fileName != filename)
		return;
	
	QFileInfo f(filename);
	QString message;
	if (f.exists())
		message = QString("%1 <a href=':reload' title='%2'>%3</a>")
		  .arg(tr("File has been modified outside the editor"))
		  .arg(tr("Clicking this links will revert all changes to this editor"))
		  .arg(tr("Click here to reload"));
	else
		message = tr("File has been deleted outside the editor.");
	displayBannerMessage(message);
}

void	QsvTextEdit::on_fileMessage_clicked(const QString &s)
{
	if (s == ":reload") {
		loadFile(m_fileName);
		hideBannerMessage();
	} else if (s == ":forcerw") {
		// TODO how to do it in a portable way?
		hideBannerMessage();
		this->setReadOnly(false);
	}
}


void	QsvTextEdit::paintEvent(QPaintEvent *e)
{
	if (m_config.showMargins) {
		uint position = fontMetrics().width(' ') * m_config.marginsWidth;
		QPainter p(viewport());
		QPen     pen = p.pen();
		QColor   c = pen.color();
		c.setAlpha(64);
		pen.setColor(c);
		p.setPen(pen);
		p.drawLine(position,0,position,viewport()->height());
	}
	QPlainTextEdit::paintEvent(e);
}

void	QsvTextEdit::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);

	// this get connected in QsvTextOperationsWidget
	emit(widgetResized());
	updateMargins();
}

void	QsvTextEdit::keyPressEvent(QKeyEvent *e)
{
	QTextCursor c = textCursor();
	switch (e->key()) {
		case Qt::Key_Escape:
			if (c.hasSelection()) {
				c.clearSelection();
				setTextCursor(c);
				e->accept();
			}
			break;
		case Qt::Key_Down:
		case Qt::Key_Up:
			if (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
				break;
			verticalScrollBar()->triggerAction( 
				e->key() == Qt::Key_Down?
				QAbstractSlider::SliderSingleStepAdd:
				QAbstractSlider::SliderSingleStepSub
			);
			e->accept();
			break;
		case Qt::Key_PageDown:
		case Qt::Key_PageUp:
			if (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
				break;
			verticalScrollBar()->triggerAction(
				e->key() == Qt::Key_PageDown?
				QAbstractSlider::SliderPageStepAdd:
				QAbstractSlider::SliderPageStepSub
			);
			e->accept();
			break;
		
		case Qt::Key_Tab:
			if (m_config.tabIndents) {
				if (handleIndentEvent( !(e->modifiers() & Qt::ShiftModifier) ))
					// do not call original hanlder, if this was handled by that function
					return;
			}
			// TODO else insert_spaced_as_needed()
			break;
		case Qt::Key_Backtab:
			if (m_config.tabIndents)
				if (handleIndentEvent(false))
					return;

		case Qt::Key_Home:
		case Qt::Key_End:
			if (!m_config.smartHome || QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) )
				break;
			if (e->key() == Qt::Key_Home)
				smartHome();
			else
				smartEnd();
			e->accept();
			return;
		
		default:
			if (m_config.autoBrackets && handleKeyPressEvent(e))
				return;
	} // end case
	QPlainTextEdit::keyPressEvent(e);
}

bool	QsvTextEdit::handleKeyPressEvent(QKeyEvent *e)
{
	QTextCursor cursor = textCursor();
	int i,j;
	QString s;
	
	// handle automatic deletcion of mathcing brackets
	if ((e->key() == Qt::Key_Delete) || (e->key() == Qt::Key_Backspace)) {
		if (cursor.hasSelection())
			return false;
		i = cursor.position() - cursor.block().position();
		QChar c1 = cursor.block().text()[ i ];
		j =  m_config.matchBracketsList.indexOf( c1 );
		if (j == -1)
			return false;
		if (i == 0)
			return false;
		if (m_config.matchBracketsList[j+1] == m_config.matchBracketsList[j])
			j++;
		QChar c2 = cursor.block().text()[ i-1 ];
		if (c2 != m_config.matchBracketsList[j-1])
			return false;
		cursor.deletePreviousChar();
		cursor.deleteChar();
		goto FUNCTION_END;
	}
	
	// handle only normal key presses
	s = e->text();
	if (s.isEmpty())
		return false;
	
	// don't handle if not in the matching list
	j = m_config.matchBracketsList.indexOf(s[0]);
	if ((j == -1) || (j%2 == 1))
		return false;
	
	// handle automatic insert of matching brackets
	i = cursor.position();
	if (!cursor.hasSelection()) {
		cursor.insertText( QString(m_config.matchBracketsList[j]) );
		cursor.insertText( QString(m_config.matchBracketsList[j+1]) );
	} else {
		QString s = cursor.selectedText();
		cursor.beginEditBlock();
		cursor.deleteChar();
		s = m_config.matchBracketsList[j] + s + m_config.matchBracketsList[j+1];
		cursor.insertText(s);
		cursor.endEditBlock();
	}
	cursor.setPosition(i+1);
	setTextCursor(cursor);
	
FUNCTION_END:
	e->accept();
	return true;
}

void	QsvTextEdit::updateMargins()
{
	if (!m_panel || !m_config.showLineNumbers) {
		setViewportMargins( 0, 0, 0, 0 );
		return;
	}
	
	QRect viewportRect = viewport()->geometry();
	QRect lrect = QRect(viewportRect.topLeft(), viewportRect.bottomLeft());
	int panelWidth = m_panel->width();
	lrect.adjust( -panelWidth, 0, 0, 0 );
	
	setViewportMargins( panelWidth-1, 0, 0, 0 );
	m_panel->setGeometry(lrect);
}

void	QsvTextEdit::showUpperWidget(QWidget* w)
{
	// TODO handle dangling pointers
	m_topWidget = w;
	
	adjustBottomAndTopWidget();
}

void	QsvTextEdit::showBottomWidget(QWidget* w)
{
	// TODO handle dangling pointers
	m_bottomWidget = w;
	
	adjustBottomAndTopWidget();
}

bool	QsvTextEdit::handleIndentEvent( bool forward )
{
	QTextCursor cursor1 = textCursor();
	bool reSelect = true;

	if (!cursor1.hasSelection()) {
		reSelect = false;
		cursor1.select( QTextCursor::LineUnderCursor );
	}
	
	QTextCursor cursor2 = cursor1;
	cursor1.setPosition( cursor1.selectionStart() );
	cursor2.setPosition( cursor2.selectionEnd() );
	int endBlock = cursor2.blockNumber();
	
	QString spaces;
	if (m_config.insertSpacesInsteadOfTabs) {
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
	do {
		int pos = cursor1.position();
		QString s = cursor1.block().text();
		cursor1.select( QTextCursor::LineUnderCursor );
		if (forward)
			cursor1.insertText( m_config.insertSpacesInsteadOfTabs? spaces + s : '\t' + s );
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

int	QsvTextEdit::getIndentationSize( const QString s )
{
	int indentation = 0;
	int i = 0;
	int l = s.length();
	if (l == 0)
		return 0;
	QChar c = s.at(i);
	while( (i<l) && ((c == ' ') || (c == '\t')) ) {
		if (c == '\t') {
			indentation ++;
			i ++;
		} else if (c == ' ') {
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

QString	QsvTextEdit::updateIndentation( QString s, int indentation )
{
	if (s.isEmpty())
		return s;
	QString spaces;
	if (m_config.insertSpacesInsteadOfTabs) {
		int k = getTabSize();
		for (int i=0; i< k; i++ )
			spaces = spaces.insert(0,' ');
	}
	while ((s.at(0) == ' ') || (s.at(0) == '\t')) {
		s.remove(0,1);
		if (s.isEmpty())
			break;
	}
	while( indentation != 0 ) {
		if (m_config.insertSpacesInsteadOfTabs)
			s = s.insert(0,spaces);
		else
			s = s.insert(0,QChar('\t'));
		indentation --;
	}
	return s;
}

int	QsvTextEdit::findMatchingChar( QChar c1, QChar c2, bool forward, QTextBlock &block, int from )
{
	int i = 1;
	while (block.isValid())
	{
		QList<Qate::MatchData> matches = m_highlighter->getMatches(block);;
		for( int k=0; k<matches.length(); k++)
		{
			int j = forward? k : matches.length() - k - 1;
			Qate::MatchData m = matches.at(j);
			int globalPosition = m.position + block.position();
			if (forward) {
				if (globalPosition <= from) continue;
			}	else if (globalPosition >= from) continue;

			if (m.matchedChar == c1)
				i ++;
			if (m.matchedChar == c2)
				i --;
			// we found the braket
			if (i==0)
			{
				return globalPosition;
			}
		}
		if (forward)
			block = block.next();
		else
			block = block.previous();
	}
	return -1;
}

QTextEdit::ExtraSelection QsvTextEdit::getSelectionForBlock( QTextCursor &cursor, QTextCharFormat &format )
{
	QTextEdit::ExtraSelection selection;
	selection.format = format;
	selection.format.setProperty(QTextFormat::FullWidthSelection, true);
	selection.cursor = cursor;
	selection.cursor.clearSelection();
	
	return selection;
}


void	QsvTextEdit::resetExtraSelections()
{
	if (m_highlighter==NULL) {
		displayBannerMessage(tr("DEBUG: Please use a proper QsvSyntaxHighlighter"));
		return;
	}
	
	QTextBlock  block = document()->firstBlock();
	int i = 0;
	
	m_selections.clear();
	while (block.isValid()){
		if (m_highlighter->getBlockFlags(block).testFlag(Qate::BlockData::Bookmark)) {
			QTextCursor cursor = textCursor();
			QTextCharFormat format;
			format.setBackground(m_bookmarkColor);
			cursor.setPosition(block.position());
			cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
			m_selections.append(getSelectionForBlock(cursor,format));
		}
		block = block.next();
		i++;
	}
	
	updateExtraSelections();
}

void	QsvTextEdit::setShowMargins( bool on )
{
	if (m_config.showMargins == on)
		return;
	m_config.showMargins = on;
	viewport()->update();
}

bool	QsvTextEdit::getShowMargins() const
{
	return m_config.showMargins;
}

void	QsvTextEdit::setMarginsWidth( uint i )
{
	if (m_config.marginsWidth == i)
		return;
	m_config.marginsWidth = i;
	if (m_config.showMargins)
		viewport()->update();
}

uint	QsvTextEdit::getMarginsWidth() const
{
	return m_config.marginsWidth;
}

void	QsvTextEdit::setTabSize( int size )
{
	m_config.tabSize = size;
	setTabStopWidth(fontMetrics().width(" ")*size);
}

int	QsvTextEdit::getTabSize() const
{
	return m_config.tabSize;
}

void QsvTextEdit::setInsertSpacesInsteadOfTabs( bool on )
{
	m_config.insertSpacesInsteadOfTabs = on;
}

bool QsvTextEdit::getInsertSpacesInsteadOfTabs() const
{
	return m_config.insertSpacesInsteadOfTabs;
}

void QsvTextEdit::setTabIndents( bool on )
{
	m_config.tabIndents = on;
}

bool QsvTextEdit::getTabIndents() const
{
	return m_config.tabIndents;
}

void QsvTextEdit::setDefaultConfig()
{
	setDefaultConfig(&m_config);
}

void QsvTextEdit::setHighlighter(QsvSyntaxHighlighterBase *s)
{
	if (m_highlighter)
		m_highlighter->setTextDocument(NULL);
	m_highlighter = s;
	if (m_highlighter) {
		m_highlighter->setTextDocument(document());
		m_highlighter->setMatchBracketList(m_config.matchBracketsList);
	}
}

QsvSyntaxHighlighterBase* QsvTextEdit::getHighlighter() const
{
	return m_highlighter;
}

void QsvTextEdit::setShowWhiteSpace( bool on )
{
	m_config.showWhiteSpaces = on;
	
	QTextOption option =  document()->defaultTextOption();
	if (on)
		option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
	else
		option.setFlags(option.flags() & ~QTextOption::ShowTabsAndSpaces);
	option.setFlags(option.flags() | QTextOption::AddSpaceForLineAndParagraphSeparators);
	document()->setDefaultTextOption(option);
}

bool QsvTextEdit::getShowWhiteSpace() const
{
	return m_config.showWhiteSpaces;
}


void QsvTextEdit::setDefaultConfig( QsvEditorConfigData *config ) // static
{
	config->currentFont        = QFont("Courier new", 10);
	config->showLineNumbers    = true;
	config->smartHome          = true;
	config->markCurrentLine    = true;
	config->matchBrackets      = true;
	config->lineWrapping       = false;
	config->matchBracketsList  = "()[]{}\"\"''";
	config->modificationsLookupEnabled = true;
	config->autoBrackets       = true;
	config->showMargins        = true;
	config->marginsWidth       = 80;
	config->tabSize            = 8;
	config->insertSpacesInsteadOfTabs = false;
	config->tabIndents         = false;
	config->showWhiteSpaces    = false;
}

void	QsvTextEdit::gotoNextBookmark()
{
	QTextCursor c = textCursor();
	QTextBlock  b = c.block().next();
	
	while(b.isValid()){
		// found it
		if (m_highlighter->isBlockBookmarked(b)) {
			c.setPosition(b.position());
			setTextCursor(c);
			return;
		}
		b = b.next();
	}
}

void	QsvTextEdit::gotoPrevBookmark()
{
	QTextCursor c = textCursor();
	QTextBlock  b = c.block().previous();
	
	while(b.isValid()){
		// found it
		if (m_highlighter->isBlockBookmarked(b)) {
			c.setPosition(b.position());
			setTextCursor(c);
			return;
		}
		b = b.previous();
	}
}

QTextCursor	QsvTextEdit::getCurrentTextCursor()
{
	QTextCursor cursor = textCursor();
	if (!cursor.hasSelection())
		cursor.select(QTextCursor::WordUnderCursor);
	return cursor;
}

void	QsvTextEdit::paintPanel(QPaintEvent*e)
{
	if (!m_panel)
		return;

	QPainter p(m_panel);
	QTextBlock current = textCursor().block();
	QTextBlock block = firstVisibleBlock();
	QString s;
	int y = blockBoundingRect(block).translated(contentOffset()).top();
	int l = block.blockNumber();
	int h = fontMetrics().height();
	int w = m_panel->width();

	p.setFont(font());
	p.fillRect( e->rect(), m_panelColor );
	while (block.isValid() && block.isVisible()){
		s = s.number(l);
		if (block.isVisible() &&  block == current) {
			QFont f = p.font();
			f.setBold(true);
			p.setFont(f);
			p.drawText( 0, y, w-5, h, Qt::AlignRight, s );
			f.setBold(false);
			p.setFont( f );
		}
		else
			p.drawText( 0, y, w-5, h, Qt::AlignRight, s );

//		if (m_highlighter->isBlockBookmarked(block))
//			p.drawPixmap( 2, y, m_panel->m_bookMarkImage );
		if (m_highlighter && m_highlighter->isBlockModified(block))
			p.fillRect( w-3, y, 2, h, m_modifiedColor );
		y += blockBoundingRect(block).height();
		block = block.next();
		l ++;
	}
}
