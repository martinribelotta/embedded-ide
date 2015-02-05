#ifndef QSVTEXTOPERATIONSWIDGET_H
#define QSVTEXTOPERATIONSWIDGET_H

// this is done to shut up warnings inside Qt Creator
// if you remove it, the whole project gets marked with warnings
// as it thinks QObject is not defined. WTF.
class QObject;
class QString;
class QTextCursor;

#include <QObject>
#include <QString>
#include <QTextCursor>
#include <QTextDocument>
#include <QColor>
#include <QTimer>

class QWidget;
class QLineEdit;

namespace Ui{
	class searchForm;
	class replaceForm;
}

class QsvTextEdit;

class QsvTextOperationsWidget : public QObject
{
	Q_OBJECT
	friend class QsvTextEdit;
public:
	QsvTextOperationsWidget( QWidget *parent );
	void initSearchWidget();
	void initReplaceWidget();
	void initGotoLineWidget();

	QFlags<QTextDocument::FindFlag> getSearchFlags();
	QFlags<QTextDocument::FindFlag> getReplaceFlags();

	virtual QTextCursor getTextCursor();
	virtual void setTextCursor(QTextCursor c);
	virtual QTextDocument* getTextDocument();

public slots:
	void showSearch();
	void showReplace();
	void showGotoLine();
	void showBottomWidget(QWidget* w=NULL);
	void on_searchText_modified(QString s);
	void on_replaceText_modified(QString s);
	void on_replaceOldText_returnPressed();
	void on_replaceAll_clicked();
	void searchNext();
	void searchPrevious();
	void searchPrev(){ searchPrevious(); }
	void adjustBottomWidget();
	
	void updateSearchInput();
	void updateReplaceInput();
	

protected:
	bool eventFilter(QObject *obj, QEvent *event);
	bool issue_search( const QString &text, QTextCursor newCursor, QFlags<QTextDocument::FindFlag> findOptions, QLineEdit *l, bool moveCursor );

	QTextCursor m_searchCursor;
	QTextDocument *m_document;
	QTimer m_replaceTimer;
	QTimer m_searchTimer;
	
public:
	QWidget *m_search;
	QWidget *m_replace;
	QWidget *m_gotoLine;
	QColor searchFoundColor;
	QColor searchNotFoundColor;
	
	Ui::searchForm *searchFormUi;
	Ui::replaceForm *replaceFormUi;
};

#endif // QSVTEXTOPERATIONSWIDGET_H
