#include <QApplication>
#include "mainwindow3.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow3 w;
	w.show();
	a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
	return a.exec();
}
