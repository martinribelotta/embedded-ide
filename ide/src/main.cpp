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
#include <QCommandLineParser>
#include <QTimer>
#include <QProcess>

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

    QCommandLineParser opt;
    opt.addHelpOption();
    opt.addVersionOption();
    opt.addPositionalArgument("filename", "Makefile filename");
#define _(str) QCoreApplication::translate(str, "main")
    opt.addOptions({
                       { { "e", "exec" }, "Execute stript or file", "execname" }
                   });
#undef _
    opt.process(a);

    if (opt.isSet("exec")) {
        QString execname = opt.value("exec");
        qDebug() << "executing" << execname;
        QProcess::startDetached(execname);
        return 0;
    }

    AppConfig::mutableInstance().addFilterTextVariable(
                "appExecPath", QCoreApplication::applicationDirPath);
    AppConfig::mutableInstance().addFilterTextVariable(
                "appExecName", QCoreApplication::applicationFilePath);

    qDebug() << "Support ssl " << QSslSocket::supportsSsl() << "\n"
             << "SSL Version: " << QSslSocket::sslLibraryVersionString() << "\n"
             << "SSL Build: " << QSslSocket::sslLibraryBuildVersionString();

    QTranslator tr;
    qDebug() << "load translations"
             << QLocale::system().name()
             << tr.load(QLocale::system().name(), ":/i18n");
    a.installTranslator(&tr);

    AppConfig::mutableInstance().load();
    AppConfig::mutableInstance().adjustPath();
    QFile hardConfFile(QDir(QApplication::applicationDirPath()).absoluteFilePath(HARD_CONF_PATH));
    if (hardConfFile.open(QFile::ReadOnly)) {
        AppConfig::mutableInstance().load();
        auto hardConf = QJsonDocument::fromJson(hardConfFile.readAll()).object();
        QStringList additionalPaths;
        for (auto p: hardConf.value("additionalPaths").toArray())
            additionalPaths.append(QDir::cleanPath(AppConfig::mutableInstance()
                                        .filterTextWithVariables(p.toString())));
        AppConfig::mutableInstance().adjustPath(additionalPaths);
        AppConfig::mutableInstance().setBuildTemplateUrl(hardConf.value("templateUrl").toString());
        AppConfig::mutableInstance().save();
        a.setProperty("hardConf", QVariant(hardConf));
    }

    AppConfig::mutableInstance().load();

    QDir projectDir = AppConfig::mutableInstance().buildDefaultProjectPath();
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

    if (!opt.positionalArguments().isEmpty()) {
        QString path = opt.positionalArguments().first();
        QTimer::singleShot(0, [path, &w]() { w.openProject(path); });
    }

    return a.exec();
}
