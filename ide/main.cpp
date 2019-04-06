#include "appconfig.h"
#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QFont>
#include <QFontDatabase>
#include <QIcon>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QProcess>
#include <QTimer>

#include <QtDebug>

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("Embedded IDE");
    QCoreApplication::setOrganizationName("none");
    QCoreApplication::setOrganizationDomain("unknown.tk");


    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon(":/images/embedded-ide.svg"));

    auto checkDarkStyle = [&app](AppConfig *config) {
        app.setStyleSheet( config->useDarkStyle()?
            AppConfig::readEntireTextFile(":/qdarkstyle/style.qss"): ""
        );
    };
    QObject::connect(&AppConfig::instance(), &AppConfig::configChanged, [&checkDarkStyle](AppConfig *config)
    {
        checkDarkStyle(config);
        switch (config->networkProxyType()) {
        case AppConfig::NetworkProxyType::None:
            QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
            break;
        case AppConfig::NetworkProxyType::System:
            QNetworkProxyFactory::setUseSystemConfiguration(true);
            break;
        case AppConfig::NetworkProxyType::Custom:
            QNetworkProxy proxy(
                        QNetworkProxy::HttpProxy, config->networkProxyHost(),
                        static_cast<quint16>(config->networkProxyPort().toInt()));
            if (config->networkProxyUseCredentials()) {
                proxy.setUser(config->networkProxyUsername());
                proxy.setPassword(config->networkProxyPassword());
            }
            QNetworkProxy::setApplicationProxy(proxy);
            break;
        }
    });
    AppConfig::instance().load();
    checkDarkStyle(&AppConfig::instance());


    QCommandLineParser opt;
    opt.addHelpOption();
    opt.addVersionOption();
    opt.addPositionalArgument("filename", "Makefile filename");
    opt.addOptions({
                       { { "e", "exec" }, "Execute stript or file", "execname" },
                       { { "d", "debug"}, "Enable debug" },
                       { { "s", "stacktrace" }, "Add stack trace to debug" }
                   });
    opt.process(app);

    if (opt.isSet("exec")) {
        QString execname = opt.value("exec");
        QProcess::startDetached(execname);
        return 0;
    }

    if (opt.isSet("debug")) {
        QString debugString = "[%{type}] %{appname} (%{file}:%{line}) - %{message}";
        if (opt.isSet("stacktrace"))
            debugString += "\n\t%{backtrace separator=\"\n\t\"}";
        qSetMessagePattern(debugString);
    } else
        qSetMessagePattern("");

    MainWindow w;
    w.show();

    if (!opt.positionalArguments().isEmpty()) {
        QString path = opt.positionalArguments().first();
        QTimer::singleShot(0, [path, &w]() { w.openProject(path); });
    }

    return QApplication::exec();
}
