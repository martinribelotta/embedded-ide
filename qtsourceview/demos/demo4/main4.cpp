#include "qsvtextoperationswidget.h"
#include "qsvdefaulthighlighter.h"
#include "qsvtextedit.h"
#include "mainwindow4.h"

MainWindow4::MainWindow4( const QString &file )
{
	DefaultHighlighter *s = new DefaultHighlighter;
	QsvTextEdit *e = new QsvTextEdit(this, s);
	QsvTextOperationsWidget *textOpetations = new QsvTextOperationsWidget(e);

	QToolBar *b = addToolBar( "" );
	b->setMovable(false);
	b->addAction( tr("&New"), e, SLOT(newDocument()))
	  ->setShortcut(QKeySequence("Ctrl+N"));
	b->addAction( tr("&Open"), this, SLOT(loadFile()))
	  ->setShortcut(QKeySequence("Ctrl+O"));
	b->addAction( tr("&Save"), e, SLOT(saveFile()))
	  ->setShortcut(QKeySequence("Ctrl+S"));
	b->addAction( tr("&Find"), textOpetations, SLOT(showSearch()))
	  ->setShortcut(QKeySequence("Ctrl+F"));
	b->addAction( tr("&Replace"), textOpetations, SLOT(showReplace()))
	  ->setShortcut(QKeySequence("Ctrl+R"));
	b->addAction( tr("Find &next"), textOpetations, SLOT(searchNext()))
	  ->setShortcut(QKeySequence("F3"));
	b->addAction( tr("Find &prev"), textOpetations, SLOT(searchPrev()))
	  ->setShortcut(QKeySequence("Shift+F3"));

	setCentralWidget(e);
	showMaximized();
	e->displayBannerMessage(tr("Click \"Open\" if you dare"));

	if (!file.isEmpty())
		loadFile(file);
	else {
		setWindowTitle("QtSourceView demo4");
		QFile f(":/qedit/readme.txt");
		if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
			return;
		e->setPlainText(f.readAll());
		e->removeModifications();
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

void MainWindow4::loadFile( QString filename )
{
	const QString dir;
	if (filename.isEmpty()) {
		filename = QFileDialog::getOpenFileName(this,tr("Load file"),dir);
		if (filename.isEmpty())
			return;
	}
	QsvTextEdit *e = findChild<QsvTextEdit*>();
	if (!e)
		return;

//		TODO
//		if (e->isModified){
//			e->save();
//		}

	e->loadFile(filename);
	e->removeModifications();
	setWindowTitle(filename);
}

int main( int argc, char* argv[] )
{
	QApplication app( argc, argv );
	MainWindow4 window("");
	window.show();
	return app.exec();
}
