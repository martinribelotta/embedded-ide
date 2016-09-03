#include "mainwindow.h"
#include <QApplication>
#include <QtDebug>
#include <QSslSocket>

extern void adjustPath();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug() << "Support ssl " << QSslSocket::supportsSsl();
    QCoreApplication::setOrganizationName("none");
    QCoreApplication::setOrganizationDomain("none.unknown.com");
    QCoreApplication::setApplicationName("embedded IDE");
    MainWindow w;
    adjustPath();
    w.show();

    return a.exec();
}
