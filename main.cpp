#include "mainwindow.h"
#include <QApplication>

extern void adjustPath();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("none");
    QCoreApplication::setOrganizationDomain("none.unknown.com");
    QCoreApplication::setApplicationName("embedded IDE");
    MainWindow w;
    adjustPath();
    w.show();

    return a.exec();
}
