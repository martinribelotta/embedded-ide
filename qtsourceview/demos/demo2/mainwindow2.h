#ifndef __MAINWINDOW2_H__
#define __MAINWINDOW2_H__

#include <QMainWindow>
#include "ui_mainwindow2.h"

class QsvColorDefFactory;
class QsvLangDef;
class QsvSyntaxHighlighter;
class QStringList;

class MainWindow2: public QMainWindow, private Ui::MainWindow2
{
	Q_OBJECT
public:	
	MainWindow2( QMainWindow *parent=0 ); 
	
public slots:
	void fillComboBoxes();
	
	void on_action_New_triggered();
	void on_action_Open_triggered();
	void on_action_About_triggered();
	void on_actionAbout_Qt_triggered();
	void on_actionE_xit_triggered();
	void on_comboBox_colors_currentIndexChanged( const QString & text );
	void on_comboBox_syntax_currentIndexChanged( const QString & text );
	
	void update_syntax_color();
	
private:
	QsvColorDefFactory	*defColors;
	QsvLangDef		*defLang;
	QsvSyntaxHighlighter	*highlight;
	QStringList		colorFiles, syntaxFiles;
	
	bool disable_combo_updates;
};

#endif //__MAINWINDOW2_H__
