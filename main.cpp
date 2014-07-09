#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QIcon::setThemeName("adfadsfdsf");
    QIcon::setThemeSearchPaths(QStringList());
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
