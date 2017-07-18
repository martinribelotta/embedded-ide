#include "dialogconfigworkspace.h"
#include "mainwindow.h"
#include "appconfig.h"

#include <QApplication>
#include <QtDebug>
#include <QSslSocket>
#include <QFile>
#include <QTranslator>
#include <QSystemTrayIcon>

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <configdialog.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#ifdef Q_OS_LINUX
#include <stdlib.h>
#define HARD_CONF_PATH "../share/embedded-ide/embedded-ide.hardconf"
#endif

#ifdef Q_OS_WIN
#define HARD_CONF_PATH "./embedded-ide.hardconf"
#endif

#ifdef Q_OS_MAC
#define HARD_CONF_PATH "../share/embedded-ide/embedded-ide.hardconf"
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
    // FIXME If QT_STYLE_OVERRIDE is set, Qt plugin platform cannot work correctly
    ::unsetenv("QT_STYLE_OVERRIDE");
#endif

    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("none");
    QCoreApplication::setOrganizationDomain("none.unknown.com");
    QCoreApplication::setApplicationName("embedded IDE");
    a.setWindowIcon(QIcon(":/images/embedded-ide.png"));

    qDebug() << "Support ssl " << QSslSocket::supportsSsl();

    QTranslator tr;
    qDebug() << "load translations"
             << QLocale::system().name()
             << tr.load(QLocale::system().name(), ":/i18n");
    a.installTranslator(&tr);

    QFile hardConfFile(QDir(QApplication::applicationDirPath()).absoluteFilePath(HARD_CONF_PATH));
    if (hardConfFile.open(QFile::ReadOnly)) {
        AppConfig::mutableInstance().load();
        auto hardConf = QJsonDocument::fromJson(hardConfFile.readAll()).object();
        QStringList additionalPaths;
        for (auto p: hardConf.value("additionalPaths").toArray())
            additionalPaths.append(p.toString().replace("{{APP_PATH}}", QCoreApplication::applicationDirPath()));
        AppConfig::mutableInstance().setBuildAdditionalPaths(additionalPaths);
        AppConfig::mutableInstance().setBuilTemplateUrl(hardConf.value("templateUrl").toString());
        AppConfig::mutableInstance().save();
    }

    AppConfig::mutableInstance().load();
    AppConfig::mutableInstance().adjustPath();

    QDir projectDir = AppConfig::mutableInstance().builDefaultProjectPath();
    QDir wSpace(QDir::cleanPath(projectDir.absoluteFilePath("..")));
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
                                  a.tr("Error creating directory %1")
                                  .arg(wSpace.absolutePath()));
            return 0;
        }
    }

    MainWindow w;
    w.show();
    a.setStyleSheet([]() -> QString {
                        QFile f(":/style.css");
                        return f.open(QFile::ReadOnly)? f.readAll() : QString();
                    }());

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
      QMessageBox::critical(
          nullptr, QObject::tr("Desktop support"),
          QCoreApplication::applicationName() +
              QObject::tr(" can not detect any system tray on this "
                          "desktop, notify this to developers."));
    }

    return a.exec();
}
