#include <QTextEdit>
#include <QApplication>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QDir>

#include "qsvsh/qsvcolordef.h"
#include "qsvsh/qsvcolordeffactory.h"
#include "qsvsh/qsvlangdef.h"
#include "qsvsh/qsvsyntaxhighlighter.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QTextEdit *textEdit = new QTextEdit;
	textEdit->show();
	QFile file("demos/demo1/main1.cpp");
	if (file.open(QFile::ReadOnly | QFile::Text)) {
		QTextStream in(&file);
		textEdit->setPlainText(in.readAll());
	}

	// load a default color set
	QsvColorDefFactory *defColors = new QsvColorDefFactory( "data/colors/kate.xml" );

	// load a default language definition
	QsvLangDef *langCpp   = new QsvLangDef( "data/langs/cpp.lang" );

	// new syntax highlighter, with the default colors and language
	QsvSyntaxHighlighter *highlight = new QsvSyntaxHighlighter( textEdit, defColors, langCpp );

	a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
	return a.exec();
	
	Q_UNUSED(highlight);
}
