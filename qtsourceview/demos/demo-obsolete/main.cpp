#include <QApplication>
#include <QMainWindow>
#include "mainwindowimpl.h"

int main (int argc, char **argv)
{
	QApplication app(argc, argv);
	MainWindowImpl w;
	w.show();
	
	return app.exec();
}
