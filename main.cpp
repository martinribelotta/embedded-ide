#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    //QIcon::setThemeName("adfadsfdsf");
    //QIcon::setThemeSearchPaths(QStringList());
    QApplication a(argc, argv);
    a.setStyle("QtCurve");
    QCoreApplication::setOrganizationName("none");
    QCoreApplication::setOrganizationDomain("none.unknown.com");
    QCoreApplication::setApplicationName("embedded IDE");
    MainWindow w;
    w.show();

    return a.exec();
}
