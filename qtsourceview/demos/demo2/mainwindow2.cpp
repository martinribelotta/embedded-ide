#include <QTimer>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QPalette>
#include <QSyntaxHighlighter>
#include <QFile>

#include "qsvsh/qsvcolordef.h"
#include "qsvsh/qsvcolordeffactory.h"
#include "qsvsh/qsvlangdef.h"
#include "qsvsh/qsvsyntaxhighlighter.h"
#include "mainwindow2.h"

//	QString dataPath  = QApplication::applicationDirPath();
QString dataPath;

MainWindow2::MainWindow2( QMainWindow *parent )
:QMainWindow( parent )
{
	defColors = NULL;
	defLang  = NULL;
	highlight  = NULL;
		
//	dataPath  = QApplication::applicationDirPath() + "/../../";	
	dataPath  = QApplication::applicationDirPath();
	setupUi( this );
        statusBar()->showMessage(tr("Welcome, the default syntax is C++"), 10000);
	QTimer::singleShot( 0, this, SLOT(fillComboBoxes()));
}

//comboBox_colors
void MainWindow2::fillComboBoxes()
{
	disable_combo_updates = true;
	QString directory = dataPath + "/data/langs/";
	QDir dir;
	
	dir = QDir( dataPath + "/data/langs/", "*.lang");
	syntaxFiles = dir.entryList(QDir::Files);
	comboBox_syntax->addItems( syntaxFiles );
		
	dir = QDir( dataPath + "/data/colors/", "*.xml");
	colorFiles = dir.entryList(QDir::Files);	
	comboBox_colors->addItems( colorFiles );

	disable_combo_updates = false;
	// now set the default language to c++, and use kate color definitions
//	comboBox_syntax->setCurrentIndex( comboBox_syntax->findText("test_.lang" ) );
	comboBox_syntax->setCurrentIndex( comboBox_syntax->findText("cpp.lang" ) );
	comboBox_colors->setCurrentIndex( comboBox_colors->findText("kate.xml" ) );
}

void MainWindow2::on_action_New_triggered()
{
	textEdit->clear();
}

void MainWindow2::on_action_Open_triggered()
{        
	QString fileName = QFileDialog::getOpenFileName( this, "Open file", "", "*" );

        if (fileName.isEmpty() )
                return;

        QFile file(fileName);
        if (!file.open(QFile::ReadOnly | QFile::Text))
        {
                QMessageBox::warning(this, tr("Application"),
                       tr("Cannot read file %1:\n%2.")
                       .arg(fileName)
                       .arg(file.errorString()));
                return;
        }

        QTextStream in(&file);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        textEdit->setPlainText(in.readAll());
        QApplication::restoreOverrideCursor();

        statusBar()->showMessage(tr("File loaded"), 2000);        
}

void MainWindow2::on_action_About_triggered()
{
	QMessageBox::information(this, "Demo 2",
    "First demo of the qtsourceview library.\n"
    "Diego Iastrubni <diegoiast@gmail.com> 2006, lincensed under the terms of the LGPL.");
}

void MainWindow2::on_actionAbout_Qt_triggered()
{
	QMessageBox::aboutQt( this, "Demo2" );
}

void MainWindow2::on_actionE_xit_triggered()
{
	this->close();
}


void MainWindow2::on_comboBox_colors_currentIndexChanged( const QString & text )
{
	if (disable_combo_updates)
		return;
			
#if 0
	delete defColors;
	defColors = new QsvColorDefFactory( dataPath + "/data/colors/" + text );
	highlight->setColorsDef( defColors );
#else
	QTimer::singleShot( 0, this, SLOT(update_syntax_color()) );
#endif	
	
	statusBar()->showMessage(tr("New color: %1").arg(text), 3000 );
}

void MainWindow2::on_comboBox_syntax_currentIndexChanged( const QString & text )
{
	if (disable_combo_updates)
		return;
		
#if 0
	delete defLang;
	defLang = new QsvLangDef( dataPath + "/data/langs/" + text );
	highlight->setHighlight( defLang );
#else
	QTimer::singleShot( 0, this, SLOT(update_syntax_color()) );
#endif

	statusBar()->showMessage(tr("New syntax: %1").arg(text), 3000 );
}

void MainWindow2::update_syntax_color()
{
	delete highlight;
	delete defLang;
	delete defColors;	
	
	defColors = new QsvColorDefFactory( dataPath + "/data/colors/" + comboBox_colors->currentText() );	

	// set the background of the texteditor, to the same as the syntax		
	// also set the default color of the syntax
	// this must be done before setting the new syntax highlighter
	QPalette p( palette() );
	QsvColorDef dsNormal = defColors->getColorDef("dsNormal");
	
	if (dsNormal.getBackground().isValid())
	    p.setColor( QPalette::Base, dsNormal.getBackground() );
	p.setColor( QPalette::Text, dsNormal.getColor() );
	textEdit->setPalette( p );
//	textEdit->setTextColor( dsNormal.getColor() );
	
        defLang   = new QsvLangDef( dataPath + "/data/langs/" + comboBox_syntax->currentText() );
	highlight = new QsvSyntaxHighlighter( textEdit, defColors, defLang );
}
