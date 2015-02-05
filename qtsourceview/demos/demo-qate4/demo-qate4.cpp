#include <QApplication>
#include <QToolBar>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QAction>

#include "qsvtextedit.h"
#include "qsvtextoperationswidget.h"
#include "demo-qate4.h"

Demo4MainWindow::Demo4MainWindow(QWidget *parent) : QMainWindow(parent)
{
	highlightReady = false;
	mimes       = new Qate::MimeDatabase();
	hl_manager = Qate::HighlightDefinitionManager::instance();
	connect(hl_manager,SIGNAL(mimeTypesRegistered()), this, SLOT(onDefinitionsMetaDataReady()));
	hl_manager->setMimeDatabase(mimes);
	hl_manager->registerMimeTypes();
	createMainGUI();
	textEditor->displayBannerMessage(tr("Click open if you dare"));
}

void Demo4MainWindow::createMainGUI()
{
	QToolBar *b = addToolBar("main");

	textEditor = new QsvTextEdit;
	textEditor->setFont(QFont("Courier",10));
	textEditor->setFrameStyle(QFrame::NoFrame);
	textEditor->setMatchBracket(true);
	textEditor->setMatchBracketList("[]{}()\"\"''");

	highlighter = new QateHighlighter;
	Qate::DefaultColors::ApplyToHighlighter(highlighter);
	highlighter->setDocument(textEditor->document());
	textEditor->setHighlighter(highlighter);
	setCentralWidget(textEditor);
	QsvTextOperationsWidget *textOpetations = new QsvTextOperationsWidget(textEditor);

	b->setMovable(false);
	b->addAction(tr("&New") , this, SLOT(onNew()))
	 ->setShortcut(QKeySequence("Ctrl+N"));
	b->addAction(tr("&Open"), this, SLOT(onOpen()))
	 ->setShortcut(QKeySequence("Ctrl+O"));
	b->addAction(tr("&Save"), this, SLOT(onSave()))
	 ->setShortcut(QKeySequence("Ctrl+S"));
	b->addSeparator();
	b->addAction(tr("&Find"), textOpetations, SLOT(showSearch()))
	 ->setShortcut(QKeySequence("Ctrl+F"));
	b->addAction(tr("&Replace"),textOpetations, SLOT(showReplace()))
	 ->setShortcut(QKeySequence("Ctrl+R"));
	b->addAction( tr("Find &next"), textOpetations, SLOT(searchNext()))
	 ->setShortcut(QKeySequence("F3"));
	b->addAction( tr("Find &prev"), textOpetations, SLOT(searchPrev()))
	 ->setShortcut(QKeySequence("Shift+F3"));
	b->addSeparator();
	b->addAction(tr("&Quit"), this, SLOT(onQuit()));

	loadTextFile("demos/demo-qate4.h");
}

void Demo4MainWindow::onDefinitionsMetaDataReady()
{
	const char* FILE = "C++";
	highlight_definition = hl_manager->definition(hl_manager->definitionIdByName(FILE));
	if (highlight_definition.isNull()) {
		qDebug("No definition found for %s", FILE);
		return;
	}
	highlighter->setDefaultContext(highlight_definition->initialContext());
	highlighter->setDocument(textEditor->document());
	highlightReady = true;
}

void Demo4MainWindow::onNew()
{
	qDebug("New");
}

void Demo4MainWindow::onOpen()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open file"), "", tr(
		"Source files (*.h * hpp *.c *.C *.cpp *.cxx *.pas);;"
		"Scripts (*.pl *.py *.sh);;"
		"All files (*)"
	));

	if (fileName.isEmpty())
		return;
	loadTextFile(fileName);
}

bool Demo4MainWindow::onSave()
{
	return false;
}

void Demo4MainWindow::onQuit()
{
	if (textEditor->document()->isModified()) {
		QMessageBox::StandardButton ret = QMessageBox::warning( this, tr("Qate - demo2"),
			tr("The document has been modified.\n"
			"Do you want to save your changes before closing?"),
			QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
			QMessageBox::Save
		);

		switch (ret) {
			case QMessageBox::Save:
				if (!onSave()) return;
				break;
			case QMessageBox::Discard:
				break;
			case QMessageBox::Cancel:
				return;
		}

	}
	QApplication::quit();
}

void Demo4MainWindow::loadTextFile(QString fileName)
{
	QFile f(fileName);

	if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
		textEditor->displayBannerMessage(tr("Could not open file: %1").arg(fileName));
		return;
	}
	QString s = f.readAll();
	textEditor->clear();
	textEditor->setLineWrapMode(QPlainTextEdit::NoWrap);
	textEditor->setHighlighter(NULL);
	if (highlightReady) {
		Qate::MimeType m = mimes->findByFile(fileName);

		if (m.isNull())
			m = getMimeByExt(fileName);

		QString definitionId = hl_manager->definitionIdByMimeType(m.type());
		if (definitionId.isEmpty())
			definitionId = findDefinitionId(m,true);
		if (!definitionId.isEmpty()) {
			qDebug("Using %s", qPrintable(definitionId));
			highlight_definition = hl_manager->definition(hl_manager->definitionIdByMimeType(m.type()));
			if (!highlight_definition.isNull()) {
				highlighter->setDefaultContext(highlight_definition->initialContext());
			} else {
				delete highlighter;
				highlighter = new QateHighlighter;
				Qate::DefaultColors::ApplyToHighlighter(highlighter);
				qDebug("Error loading %s", qPrintable(definitionId));
			}
		} else {
			delete highlighter;
			highlighter = new QateHighlighter;
			Qate::DefaultColors::ApplyToHighlighter(highlighter);
			qDebug("No definition found for %s", qPrintable(fileName));
		}
		highlighter->setDocument(textEditor->document());
	}

	textEditor->setHighlighter(highlighter);
	textEditor->removeModifications();
	textEditor->setPlainText(s);
}

QString Demo4MainWindow::findDefinitionId(const Qate::MimeType &mimeType, bool considerParents) const
{
	QString definitionId = hl_manager->definitionIdByAnyMimeType(mimeType.aliases());
	if (definitionId.isEmpty() && considerParents) {
		definitionId = hl_manager->definitionIdByAnyMimeType(mimeType.subClassesOf());
		if (definitionId.isEmpty()) {
			foreach (const QString &parent, mimeType.subClassesOf()) {
				const Qate::MimeType &parentMimeType =  mimes->findByType(parent);
				definitionId = findDefinitionId(parentMimeType, considerParents);
			}
		}
	}
	return definitionId;
}

Qate::MimeType Demo4MainWindow::getMimeByExt(const QString &fileName)
{
	QFileInfo fi(fileName);
	QString extension = QString("*.%1").arg(fi.suffix());
	foreach(Qate::MimeType mime, mimes->mimeTypes() ) {
		foreach(Qate::MimeGlobPattern pattern, mime.globPatterns() ) {
			if (extension == pattern.regExp().pattern())
				return mime;
		}
	}
	return Qate::MimeType();
}

int main( int argc, char* argv[] )
{
	QApplication app( argc, argv );
	QMainWindow *mainWindow = new Demo4MainWindow;
	mainWindow->show();
	return app.exec();
}
