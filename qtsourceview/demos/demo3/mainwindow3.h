#ifndef __MAINWINDOW2_H__
#define __MAINWINDOW2_H__

#include <QMainWindow>
#include "ui_mainwindow3.h"

class QsvColorDefFactory;
class QsvLangDef;
class QsvSyntaxHighlighter;

class MainWindow3: public QMainWindow, private Ui::MainWindow3
{
	Q_OBJECT
public:	
	MainWindow3( QMainWindow *parent=0 ); 
	
public slots:
	void on_action_New_triggered();
	void on_action_Open_triggered();
	void on_action_About_triggered();
	void on_actionAbout_Qt_triggered();
	void on_actionE_xit_triggered();
	
private:
	QsvColorDefFactory	*defColors;
	QsvLangDef		*langDefinition;
	QsvSyntaxHighlighter	*highlight;
};

#endif //__MAINWINDOW1_H__
