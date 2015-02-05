#include <QApplication>
#include <QAction>
#include "demo-qate2.h"

Demo2MainWindow::Demo2MainWindow(QWidget *parent) : QMainWindow(parent), pdummyContext(&dummyContext)
{
	createMainGUI();
	highlightReady = false;
	mimes       = new Qate::MimeDatabase();
	highlighter = new TextEditor::Internal::Highlighter;
	Qate::DefaultColors::ApplyToHighlighter(highlighter);
	hl_manager = Qate::HighlightDefinitionManager::instance();
	connect(hl_manager,SIGNAL(mimeTypesRegistered()), this, SLOT(onDefinitionsMetaDataReady()));
	hl_manager->setMimeDatabase(mimes);
	hl_manager->registerMimeTypes();
}

void Demo2MainWindow::createMainGUI()
{
	QToolBar *b = addToolBar("main");

    b->setMovable(false);
    b->addAction(tr("&New") , this, SLOT(onNew()))
     ->setShortcut(QKeySequence("Ctrl+N"));
	b->addAction(tr("&Open"), this, SLOT(onOpen()))
     ->setShortcut(QKeySequence("Ctrl+O"));
	b->addAction(tr("&Save"), this, SLOT(onSave()))
     ->setShortcut(QKeySequence("Ctrl+S"));
	b->addSeparator();
	b->addAction(tr("&Quit"), this, SLOT(onQuit()));

	textEditor = new QPlainTextEdit;
	textEditor->setFont(QFont("Courier",10));
	textEditor->setFrameStyle(QFrame::NoFrame);
	setCentralWidget(textEditor);
//	loadTextFile("demo-qate2.h");
}

void Demo2MainWindow::onDefinitionsMetaDataReady()
{
	/*
	const char* FILE = "C++";
	highlight_definition = hl_manager->definition(hl_manager->definitionIdByName(FILE));
	if (highlight_definition.isNull()) {
		qDebug("No definition found for %s", FILE);
		return;
	}
	highlighter->setDefaultContext(highlight_definition->initialContext());
	highlighter->setDocument(textEditor->document());
	*/
	highlightReady = false;
	highlightReady = true;
	loadTextFile("demo-qagte2.h");
}

void Demo2MainWindow::onNew()
{
	qDebug("New");
}

void Demo2MainWindow::onOpen()
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

bool Demo2MainWindow::onSave()
{
	return false;
}

void Demo2MainWindow::onQuit()
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

void Demo2MainWindow::loadTextFile(QString fileName)
{
	QFile f(fileName);

	if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	QString s = f.readAll();
	textEditor->clear();
	textEditor->setLineWrapMode(QPlainTextEdit::NoWrap);

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
				highlighter = new TextEditor::Internal::Highlighter;
				Qate::DefaultColors::ApplyToHighlighter(highlighter);
				qDebug("Error loading %s", qPrintable(definitionId));
			}
		} else {
			delete highlighter;
			highlighter = new TextEditor::Internal::Highlighter;
			Qate::DefaultColors::ApplyToHighlighter(highlighter);
			qDebug("No definition found for %s", qPrintable(fileName));
		}
		highlighter->setDocument(textEditor->document());
	}

	textEditor->setPlainText(s);
}

QString Demo2MainWindow::findDefinitionId(const Qate::MimeType &mimeType, bool considerParents) const
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

Qate::MimeType Demo2MainWindow::getMimeByExt(const QString &fileName)
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
	QMainWindow *mainWindow = new Demo2MainWindow;
	mainWindow->show();
	return app.exec();
}
