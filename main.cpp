#include "mainwindow.h"
#include <QApplication>
#include <QtDebug>
#include <QSslSocket>
#include <QFile>
#include <QTranslator>

extern void adjustPath();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("none");
    QCoreApplication::setOrganizationDomain("none.unknown.com");
    QCoreApplication::setApplicationName("embedded IDE");

    a.setWindowIcon(QIcon(":/images/embedded-ide.png"));
    a.setStyleSheet([]() -> QString {
                        QFile f(":/style.css");
                        return f.open(QFile::ReadOnly)? f.readAll() : QString();
                    }());
    QTranslator tr;
    qDebug() << "load translations"
             << QLocale::system().name()
             << tr.load(QLocale::system().name(), ":/i18n");
    a.installTranslator(&tr);

    qDebug() << "Support ssl " << QSslSocket::supportsSsl();
    adjustPath();

    MainWindow w;
    w.show();

    return a.exec();
}
