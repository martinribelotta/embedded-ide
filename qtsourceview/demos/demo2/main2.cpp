#include <QApplication>
#include "mainwindow2.h"

int main(int argc, char *argv[])
{
        QApplication a(argc, argv);
        MainWindow2 w;
        w.show();
        a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
        return a.exec();
}
