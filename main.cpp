#include "dialogconfigworkspace.h"
#include "mainwindow.h"
#include "appconfig.h"

#include <QApplication>
#include <QtDebug>
#include <QSslSocket>
#include <QFile>
#include <QTranslator>

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <configdialog.h>

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
    AppConfig::mutable_instance().adjustPath();

    QDir projectDir = AppConfig::mutable_instance().builDefaultProjectPath();
    QDir wSpace(projectDir.absoluteFilePath(".."));
    if (!wSpace.exists()) {
        DialogConfigWorkspace d;
        if (d.exec() != QDialog::Accepted) {
            return 0;
        }
        wSpace.setPath(d.path());
    }
    if (!wSpace.exists()) {
        auto root = QDir::root();
        if (!root.mkpath(wSpace.absolutePath())) {
            QMessageBox::critical(nullptr,
                                  a.tr("Error"),
                                  a.tr("Error creating directory %1 because %2")
                                  .arg(wSpace.absolutePath()));
            return 0;
        }
    }


    MainWindow w;
    w.show();

    return a.exec();
}
