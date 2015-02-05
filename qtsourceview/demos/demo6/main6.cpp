#include <QApplication>
#include <QAction>
#include <QToolBar>
#include <QTimer>
#include <QMainWindow>
#include <QFileDialog>
#include <QSyntaxHighlighter>
#include <QToolButton>
#include <QPushButton>
#include <QDebug>
#include <QCommonStyle>

#include "context.h"
#include "highlighter.h"
#include "highlightdefinition.h"
#include "qate/highlightdefinitionmanager.h"
#include "qate/defaultcolors.h"

#include "qsvtextedit.h"
#include "qsvsyntaxhighlighterbase.h"
#include "qsvtextoperationswidget.h"

class MyHighlighter: public TextEditor::Internal::Highlighter, public QsvSyntaxHighlighterBase
{
public:
	MyHighlighter(QTextDocument *parent): TextEditor::Internal::Highlighter(parent)
	{
		setMatchBracketList("{}()[]''\"\"");
	}

	void highlightBlock(const QString &text)
	{
		QsvSyntaxHighlighterBase::highlightBlock(text);
		TextEditor::Internal::Highlighter::highlightBlock(text);
	}

	virtual void toggleBookmark(QTextBlock &block)
	{
	}

	virtual void removeModification(QTextBlock &block)
	{
	}

	virtual void setBlockModified(QTextBlock &block, bool on)
	{
	}

	virtual bool isBlockModified(QTextBlock &block)
	{
	}

	virtual bool isBlockBookmarked(QTextBlock &block)
	{
	}

	virtual QsvBlockData::LineFlags getBlockFlags(QTextBlock &block)
	{
	}
};


class MainWindow : QMainWindow
{
	Q_OBJECT
	QsvTextEdit *editor;
	QsvSyntaxHighlighterBase * syntax;
	QsvTextOperationsWidget  * textOpetations;

	Qate::MimeDatabase                                        *mimes;
	Qate::HighlightDefinitionManager                          *hl_manager;
	TextEditor::Internal::Highlighter                         *highlight;
	MyHighlighter                                             *highlightEx;
	QSharedPointer<TextEditor::Internal::HighlightDefinition>  highlight_definition;

public:
	MainWindow( const QString &file )
	{
		editor     = new QsvTextEdit(this, NULL);
		highlightEx  = new MyHighlighter(editor->document());
		editor->setHighlighter(highlightEx);
		editor->setFrameStyle(QFrame::NoFrame);
		editor->displayBannerMessage(tr("Click \"Open\" if you dare"), 3);

		mimes      = new Qate::MimeDatabase();
//		highlight  = new TextEditor::Internal::Highlighter(editor->document());
		highlight = highlightEx;
		hl_manager = Qate::HighlightDefinitionManager::instance();
		hl_manager->setMimeDatabase(mimes);
		hl_manager->registerMimeTypes();
		Qate::DefaultColors::ApplyToHighlighter(highlight);
		connect(hl_manager,SIGNAL(mimeTypesRegistered()),this,SLOT(setCPPHighlight()));

		textOpetations   = new QsvTextOperationsWidget(editor);
		textOpetations->initSearchWidget();
		textOpetations->initReplaceWidget();
		setCentralWidget(editor);
		setup_GUI();
		showMaximized();

		if (!file.isEmpty())
			loadFile(file);
		else {
			setWindowTitle("QtSourceView/qate demo6 - qedit");
			QFile f(":/qedit/readme.txt");
			if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
				return;
			editor->setPlainText(f.readAll());
			editor->removeModifications();
		}
#if 0
		e->setMarkCurrentLine(false);
		// tests for defaults
		e->setShowLineNumbers(true);
		e->setShowMargins(true);
		e->setTabSize(8);
		e->setTabIndents(true);
		e->setInsertSpacesInsteadOfTabs(true);
		e->setShowWhiteSpace(true);
#endif
	}
	
public slots:
	void loadFile( QString filename ="" )
	{
		const QString dir;
		if (filename.isEmpty()) {
			filename = QFileDialog::getOpenFileName(this,tr("Load file"),dir);
			if (filename.isEmpty())
				return;
		}
//		TODO
//		if (e->isModified){
//			e->save();
//		}
		
		editor->clear();
//		langDefinition = QsvLangDefFactory::getInstanse()->getHighlight(filename);
//		highlight->setHighlight(langDefinition);
		editor->loadFile(filename);
		editor->removeModifications();
		setWindowTitle(filename);
	}

	void setCPPHighlight()
	{
		highlight_definition = hl_manager->definition( hl_manager->definitionIdByName("C++") );
		if (highlight_definition.isNull()) {
			editor->displayBannerMessage("No C++ highlight definition is found. If you are using QtCreator's defintion, please download also C++");
			return;
		}
		highlight->setDefaultContext(highlight_definition->initialContext());
	}

	void setup_GUI()
	{
		QPushButton *p;
		editor->findChild<QWidget*>("banner")
		      ->findChild<QToolButton*>("closeButton")
		      ->setIcon( style()->standardIcon(QStyle::SP_DockWidgetCloseButton) );

		textOpetations->m_search->findChild<QAbstractButton*>("closeButton")->setIcon( style()->standardIcon(QStyle::SP_DockWidgetCloseButton) );
		p = textOpetations->m_search->findChild<QPushButton*>("previousButton");
		p->setIcon( style()->standardIcon(QStyle::SP_ArrowUp) );
		p->setText("");
		p->setFlat(true);
		p->setAutoRepeat(true);
		p = textOpetations->m_search->findChild<QPushButton*>("nextButton");
		p->setIcon( style()->standardIcon(QStyle::SP_ArrowDown) );
		p->setText("");
		p->setFlat(true);
		p->setAutoRepeat(true);

		textOpetations->m_replace->findChild<QAbstractButton*>("closeButton")->setIcon( style()->standardIcon(QStyle::SP_DockWidgetCloseButton) );
		p = textOpetations->m_search->findChild<QPushButton*>("previousButton");

		QToolBar *b = addToolBar( "" );
		b->addAction( tr("&New"), editor, SLOT(newDocument()))              ->setShortcut(QKeySequence("Ctrl+N"));
		b->addAction( tr("&Open"), this, SLOT(loadFile()))                  ->setShortcut(QKeySequence("Ctrl+O"));
		b->addAction( tr("&Save"), editor, SLOT(saveFile()))                ->setShortcut(QKeySequence("Ctrl+S"));
		b->addAction( tr("&Find"), textOpetations, SLOT(showSearch()))      ->setShortcut(QKeySequence("Ctrl+F"));
		b->addAction( tr("&Replace"), textOpetations, SLOT(showReplace()))  ->setShortcut(QKeySequence("Ctrl+R"));
		b->addAction( tr("Find &next"), textOpetations, SLOT(searchNext())) ->setShortcut(QKeySequence("F3"));
		b->setMovable(false);
		b->addAction( tr("Find &prev"), textOpetations, SLOT(searchPrev())) ->setShortcut(QKeySequence("Shift+F3"));
		b->setMovable(false);
	}

private:
};

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w( a.arguments().count()>=2 ? a.arguments().at(1) : QString() );
	a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
	return a.exec();
}

#include "main6.moc"
