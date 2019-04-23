/*
 * This file is part of Embedded-IDE
 * 
 * Copyright 2019 Martin Ribelotta <martinribelotta@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
#include <QTranslator>

#include <QtDebug>

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("Embedded IDE");
    QCoreApplication::setOrganizationName("none");
    QCoreApplication::setOrganizationDomain("unknown.tk");

    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon(":/images/embedded-ide.svg"));
    QTranslator tr;
    for(const auto& p: AppConfig::langPaths()) {
        if (tr.load(QLocale::system().name(), p)) {
            if (app.installTranslator(&tr)) {
                qDebug() << "load translations" << QLocale::system().name();
                break;
            }
        }
    }

    auto checkConfig = [&app, &tr](AppConfig *config) {
        app.setStyleSheet( config->useDarkStyle()?
            AppConfig::readEntireTextFile(":/qdarkstyle/style.qss"): ""
        );
        auto selectedLang = config->language();
        if (!selectedLang.isEmpty()) {
            for(const auto& p: AppConfig::langPaths()) {
                if (tr.load(selectedLang, p)) {
                    if (app.installTranslator(&tr)) {
                        qDebug() << "load translations" << QLocale::system().name();
                        break;
                    }
                }
            }
        }
    };
    QObject::connect(&AppConfig::instance(), &AppConfig::configChanged, [&checkConfig](AppConfig *config)
    {
        checkConfig(config);
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
    checkConfig(&AppConfig::instance());


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
