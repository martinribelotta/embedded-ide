#include "mainwindow.h"
#include <QApplication>
#include <QtDebug>
#include <QSslSocket>
#include <QFile>

extern void adjustPath();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyleSheet([]() -> QString {
                        QFile f(":/style.css");
                        return f.open(QFile::ReadOnly)? f.readAll() : QString();
                    }());
    qDebug() << "Support ssl " << QSslSocket::supportsSsl();
    QCoreApplication::setOrganizationName("none");
    QCoreApplication::setOrganizationDomain("none.unknown.com");
    QCoreApplication::setApplicationName("embedded IDE");
    MainWindow w;
    a.setWindowIcon(QIcon(":/images/embedded-ide.png"));
    adjustPath();
    w.show();

    return a.exec();
}
