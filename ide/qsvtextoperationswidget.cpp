#include "qsvtextoperationswidget.h"
#include "ui_searchform.h"
#include "ui_replaceform.h"

#include <QLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QTextDocument>
#include <QMessageBox>

#include <QDebug>

QsvTextOperationsWidget::QsvTextOperationsWidget( QWidget *parent )
	: QObject(parent)
{
	setObjectName("QsvTextOperationWidget");
	m_gotoLine    = NULL;
	m_search      = NULL;
	m_replace     = NULL;
	m_document    = NULL;
	searchFormUi  = NULL;
	replaceFormUi = NULL;
	searchFoundColor	= QColor( "#DDDDFF" ); //QColor::fromRgb( 220, 220, 255)
	searchNotFoundColor	= QColor( "#FFAAAA" ); //QColor::fromRgb( 255, 102, 102) "#FF6666"
	
	m_replaceTimer.setInterval(100);
	m_replaceTimer.setSingleShot(true);
	connect(&m_replaceTimer,SIGNAL(timeout()),this,SLOT(updateReplaceInput()));

	// this one is slower, to let the user think about his action
	// this is a modifying command, unlike a passive search
	m_searchTimer.setInterval(250);
	m_searchTimer.setSingleShot(true);
	connect(&m_searchTimer,SIGNAL(timeout()),this,SLOT(updateSearchInput()));
	
	// TODO clean this up, this is too ugly to be in a constructor
	QTextEdit *t = qobject_cast<QTextEdit*>(parent);
	if (t) {
		m_document = t->document();
	}
	else {
		QPlainTextEdit *pt = qobject_cast<QPlainTextEdit*>(parent);
		if (pt) {
			m_document = pt->document();
		}
	}
	connect(parent,SIGNAL(widgetResized()),this,SLOT(adjustBottomWidget()));
	parent->installEventFilter(this);
}

void QsvTextOperationsWidget::initSearchWidget()
{
	m_search = new QWidget( (QWidget*) parent() );
	m_search->setObjectName("m_search");
	searchFormUi = new Ui::searchForm();
	searchFormUi->setupUi(m_search);
	searchFormUi->searchText->setFont( m_search->parentWidget()->font() );
        if (searchFormUi->frame->style()->inherits("QWindowsStyle"))
		searchFormUi->frame->setFrameStyle(QFrame::StyledPanel);
	// otherwise it inherits the default font from the editor - fixed
	m_search->setFont(QApplication::font());
	m_search->adjustSize();
	m_search->hide();

	connect(searchFormUi->searchText,SIGNAL(textChanged(QString)),this,SLOT(on_searchText_modified(QString)));
	connect(searchFormUi->nextButton,SIGNAL(clicked()),this,SLOT(searchNext()));
	connect(searchFormUi->previousButton,SIGNAL(clicked()),this,SLOT(searchPrev()));
	connect(searchFormUi->closeButton,SIGNAL(clicked()),this, SLOT(showSearch()));
}

void QsvTextOperationsWidget::initReplaceWidget()
{
	m_replace = new QWidget( (QWidget*) parent() );
	m_replace->setObjectName("m_replace");
	replaceFormUi = new Ui::replaceForm();
	replaceFormUi->setupUi(m_replace);
	replaceFormUi->optionsGroupBox->hide();
	replaceFormUi->findText->setFont( m_replace->parentWidget()->font() );
	replaceFormUi->replaceText->setFont( m_replace->parentWidget()->font() );
        if (replaceFormUi->frame->style()->inherits("QWindowsStyle"))
		replaceFormUi->frame->setFrameStyle(QFrame::StyledPanel);
	// otherwise it inherits the default font from the editor - fixed
	m_replace->setFont(QApplication::font());
	m_replace->adjustSize();
	m_replace->hide();

	connect(replaceFormUi->moreButton,SIGNAL(clicked()),this,SLOT(adjustBottomWidget()));
	connect(replaceFormUi->findText,SIGNAL(textChanged(QString)),this,SLOT(on_replaceText_modified(QString)));
	connect(replaceFormUi->replaceButton,SIGNAL(clicked()),this,SLOT(on_replaceOldText_returnPressed()));
	connect(replaceFormUi->closeButton,SIGNAL(clicked()),this, SLOT(showReplace()));
}

void QsvTextOperationsWidget::initGotoLineWidget()
{
	m_gotoLine = new QWidget( (QWidget*) parent() );
	m_gotoLine->setObjectName("m_gotoLine");
	m_gotoLine->adjustSize();
	m_gotoLine->hide();
}

void	QsvTextOperationsWidget::searchNext()
{
	if (!searchFormUi)
		return;
	issue_search( searchFormUi->searchText->text(), 
		getTextCursor(), 
		getSearchFlags() & ~QTextDocument::FindBackward,
		searchFormUi->searchText,
		true
	);
}

void	QsvTextOperationsWidget::searchPrevious()
{
	if (!searchFormUi)
	return;
	issue_search( searchFormUi->searchText->text(),
		getTextCursor(), 
		getSearchFlags() | QTextDocument::FindBackward, 
		searchFormUi->searchText,
		true
	);
}

void	QsvTextOperationsWidget::adjustBottomWidget()
{
	showBottomWidget(NULL);
}

void	 QsvTextOperationsWidget::updateSearchInput()
{
	if (!searchFormUi)
		return;
	issue_search(searchFormUi->searchText->text(),
		m_searchCursor,
		getSearchFlags(),
		searchFormUi->searchText,
		true
	);
}

void	 QsvTextOperationsWidget::updateReplaceInput()
{
	if (!replaceFormUi)
		return;
	issue_search(replaceFormUi->findText->text(),
		m_searchCursor,
		getReplaceFlags(),
		replaceFormUi->findText,
		true
	);
}

bool	 QsvTextOperationsWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (obj != parent())
		return false;
	if (event->type() != QEvent::KeyPress)
		return false;

	QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
	switch (keyEvent->key()){
		case Qt::Key_Escape:
			if (m_search && m_search->isVisible()){
				showSearch();
				return true;
			} else if (m_replace && m_replace->isVisible()){
				showReplace();
				return true;
			}/* else if (m_gotoLine && m_gotoLine->isVisible()) {
				showGotoLine();
				return true;
			}*/
			break;
			
		case Qt::Key_Enter:
		case Qt::Key_Return:
			if (m_search && m_search->isVisible()){
				if (keyEvent->modifiers().testFlag(Qt::ControlModifier) ||
				    keyEvent->modifiers().testFlag(Qt::AltModifier) ||
				    keyEvent->modifiers().testFlag(Qt::ShiftModifier) )
					searchPrev();
				else
					searchNext();
				return true;
			} else if (m_replace && m_replace->isVisible()){
				if (keyEvent->modifiers().testFlag(Qt::ControlModifier) ||
				    keyEvent->modifiers().testFlag(Qt::AltModifier) ||
				    keyEvent->modifiers().testFlag(Qt::ShiftModifier) )
					on_replaceAll_clicked();
				else
					on_replaceOldText_returnPressed();
				return true;
			}
			
			// TODO replace, goto line
			break;

		case Qt::Key_Tab:
		case Qt::Key_Backtab:
			if (m_replace && m_replace->isVisible()){
				/*
				// TODO - no cycle yet.
				if (Qt::Key_Tab == keyEvent->key())
					m_replace->focusWidget()->nextInFocusChain()->setFocus();
				else
					m_replace->focusWidget()->previousInFocusChain()->setFocus();
				*/
				// Instead - cycle between those two input lines. IMHO good enough
				if (replaceFormUi->replaceText->hasFocus()){
					replaceFormUi->findText->setFocus();
					replaceFormUi->findText->selectAll();
				}
				else{
					replaceFormUi->replaceText->setFocus();
					replaceFormUi->replaceText->selectAll();
				}
				return true;
			}
			break;
	}

	return false;
}

QFlags<QTextDocument::FindFlag> QsvTextOperationsWidget::getSearchFlags()
{
	QFlags<QTextDocument::FindFlag> f;

	// one can never be too safe
	if (!searchFormUi){
		qDebug("%s:%d - searchFormUi not available, memory problems?", __FILE__, __LINE__ );
		return f;
	}

	if (searchFormUi->caseSensitiveCheckBox->isChecked())
		f = f | QTextDocument::FindCaseSensitively;
	if (searchFormUi->wholeWorldsCheckbox->isChecked())
		f = f | QTextDocument::FindWholeWords;
	return f;
}

QFlags<QTextDocument::FindFlag> QsvTextOperationsWidget::getReplaceFlags()
{
	QFlags<QTextDocument::FindFlag> f;
	if (!replaceFormUi){
		qDebug("%s:%d - replaceFormUi not available, memory problems?", __FILE__, __LINE__ );
		return f;
	}
	if (replaceFormUi->caseCheckBox->isChecked())
		f = f | QTextDocument::FindCaseSensitively;
	if (replaceFormUi->wholeWordsCheckBox->isChecked())
		f = f | QTextDocument::FindWholeWords;
	return f;
}

QTextCursor	QsvTextOperationsWidget::getTextCursor()
{
	QTextCursor searchCursor;
	QTextEdit *t = qobject_cast<QTextEdit*>(parent());
	if (t) {
		searchCursor = t->textCursor();
	} else {
		QPlainTextEdit *pt = qobject_cast<QPlainTextEdit*>(parent());
		if (pt) {
			searchCursor = pt->textCursor();
		}
	}
	return searchCursor;
}

void	QsvTextOperationsWidget::setTextCursor(QTextCursor c)
{
	QTextEdit *t = qobject_cast<QTextEdit*>(parent());
	if (t) {
		t->setTextCursor(c);
	} else {
		QPlainTextEdit *pt = qobject_cast<QPlainTextEdit*>(parent());
		if (pt) {
			pt->setTextCursor(c);
		}
	}
}

QTextDocument* QsvTextOperationsWidget::getTextDocument()
{
	QTextEdit *t = qobject_cast<QTextEdit*>(parent());
	if (t) {
		return t->document();
	} else {
		QPlainTextEdit *pt = qobject_cast<QPlainTextEdit*>(parent());
		if (pt) {
			return pt->document();
		}
	}
	return NULL;
}

void QsvTextOperationsWidget::showSearch()
{
	if (!m_search)
		initSearchWidget();
	if (m_replace && m_replace->isVisible())
		m_replace->hide();

	QWidget *parent = qobject_cast<QWidget*>(this->parent());
	if (m_search->isVisible()) {
		m_search->hide();
		if (parent)
			parent->setFocus();
		return;
	}

	m_searchCursor = getTextCursor();
	searchFormUi->searchText->setFocus();
	searchFormUi->searchText->selectAll();
	showBottomWidget(m_search);
}

void	QsvTextOperationsWidget::on_replaceOldText_returnPressed()
{
	if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) ||
	    QApplication::keyboardModifiers().testFlag(Qt::AltModifier) ||
	    QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) ) {
		on_replaceAll_clicked();
		showReplace();
		return;
	}

	QTextCursor c = m_searchCursor;
	QTextDocument *doc = getTextDocument();
	if (!doc){
		qDebug("%s:%d - no document found, using a wrong class? wrong parent?", __FILE__,__LINE__);
		return;
	}
	c = doc->find( replaceFormUi->findText->text(), c, getReplaceFlags() );
	if (c.isNull())
		return;

	int start = c.selectionStart();
	int end   = c.selectionEnd();
	c.beginEditBlock();
	c.deleteChar();
	c.insertText( replaceFormUi->replaceText->text() );
	c.setPosition(start,QTextCursor::KeepAnchor);
	c.setPosition(end  ,QTextCursor::MoveAnchor);
	c.endEditBlock();
	setTextCursor( c );

	// is there any other apperance of this text?
	m_searchCursor = c;
	updateReplaceInput();
}

void	QsvTextOperationsWidget::on_replaceAll_clicked()
{
	// WHY NOT HIDING THE WIDGET?
	// it seems that if you hide the widget, when the replace all action
	// is triggered by pressing control+enter on the replace widget
	// eventually an "enter event" is sent to the text eidtor.
	// the work around is to update the transparency of the widget, to let the user
	// see the text bellow the widget

	//showReplaceWidget();
	m_replace->hide();

	int replaceCount = 0;
	//        replaceWidget->setWidgetTransparency( 0.2 );
	QTextCursor c = getTextCursor();
	c = getTextDocument()->find( replaceFormUi->replaceText->text(), c, getReplaceFlags() );

	while (!c.isNull())
	{
		setTextCursor( c );
		QMessageBox::StandardButton button = QMessageBox::question( qobject_cast<QWidget*>(parent()), tr("Replace all"), tr("Replace this text?"),
			QMessageBox::Yes | QMessageBox::Ignore | QMessageBox::Cancel );

		if (button == QMessageBox::Cancel)
		{
			break;

		}
		else if (button == QMessageBox::Yes)
		{
			c.beginEditBlock();
			c.deleteChar();
			c.insertText( replaceFormUi->replaceText->text() );
			c.endEditBlock();
			setTextCursor( c );
			replaceCount++;
		}

		c = getTextDocument()->find( replaceFormUi->replaceText->text(), c, getReplaceFlags() );
	}
	// replaceWidget->setWidgetTransparency( 0.8 );
	m_replace->show();

	QMessageBox::information( 0, tr("Replace all"), tr("%1 replacement(s) made").arg(replaceCount) );
}

void	QsvTextOperationsWidget::showReplace()
{
	if (!m_replace)
		initReplaceWidget();
	if (m_search && m_search->isVisible())
		m_search->hide();

	QWidget *parent = qobject_cast<QWidget*>(this->parent());
	if (m_replace->isVisible()) {
		m_replace->hide();
		if (parent)
			parent->setFocus();
		return;
	}

	if (m_searchCursor.isNull())
		m_searchCursor = getTextCursor();
	replaceFormUi->findText->setFocus();
	replaceFormUi->findText->selectAll();
	showBottomWidget(m_replace);
}

void QsvTextOperationsWidget::showGotoLine()
{
	if (!m_gotoLine)
	initGotoLineWidget();

	QWidget *parent = qobject_cast<QWidget*>(this->parent());
	if (m_gotoLine->isVisible()) {
		m_gotoLine->hide();
		if (parent)
			parent->setFocus();
		return;
	}

	showBottomWidget(m_gotoLine);
}

void	QsvTextOperationsWidget::showBottomWidget(QWidget* w)
{
	if (w == NULL) {
		if (m_replace && m_replace->isVisible())
			w = m_replace;
		else if (m_search && m_search->isVisible())
			w = m_search;
		else if (m_gotoLine && m_gotoLine->isVisible())
			w = m_gotoLine;
	}
	if (!w)
		return;

	QRect r;
	QWidget *parent = qobject_cast<QWidget*>(this->parent());

	// I must admit this line looks ugly, but I am open to suggestions
	if (parent->inherits("QAbstractScrollArea"))
		parent = ((QAbstractScrollArea*) (parent))->viewport();

	r = parent->rect();
	w->adjustSize();
	r.adjust(10, 0, -10, 0);
	r.setHeight(w->height());
	r.moveBottom(parent->rect().height()-10);

	r.moveLeft(parent->pos().x() + 10);
	w->setGeometry(r);
	w->show();
}

void QsvTextOperationsWidget::on_searchText_modified(QString s)
{
	if (m_searchTimer.isActive())
		m_searchTimer.stop();
	m_searchTimer.start();
	Q_UNUSED(s);

	//this will triggered by the timer
	//updateSearchInput();
}

void	QsvTextOperationsWidget::on_replaceText_modified(QString s)
{
	if (m_replaceTimer.isActive())
		m_replaceTimer.stop();
	m_replaceTimer.start();
	Q_UNUSED(s);

	//this will be triggered by the timer
	//updateReplaceInput();
}

bool	QsvTextOperationsWidget::issue_search( const QString &text, QTextCursor newCursor, QFlags<QTextDocument::FindFlag> findOptions, QLineEdit *l, bool moveCursor )
{
	QTextCursor c = m_document->find( text, newCursor, findOptions );
	bool found = ! c.isNull();

	//lets try again, from the start
	if (!found) {
		c.movePosition(findOptions.testFlag(QTextDocument::FindBackward)? QTextCursor::End : QTextCursor::Start);
		c = m_document->find(text, c, findOptions);
		found = ! c.isNull();
	}

	QPalette p = l->palette();
	if (found) {
		p.setColor(QPalette::Base, searchFoundColor);
	} else {
		if (!text.isEmpty())
			p.setColor(QPalette::Base, searchNotFoundColor);
		else
			p.setColor(QPalette::Base,
				l->style()->standardPalette().base().color()
			);
		c =  m_searchCursor;
	}
	l->setPalette(p);

	if (moveCursor){
		int start = c.selectionStart();
		int end   = c.selectionEnd();
		c.setPosition(end  ,QTextCursor::MoveAnchor);
		c.setPosition(start,QTextCursor::KeepAnchor);
		setTextCursor(c);
	}
	return found;
}
