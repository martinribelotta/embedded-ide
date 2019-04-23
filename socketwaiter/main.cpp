/*
 * This file is part of shocketwaiter, utility of Embedded-IDE
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
#include <QtCore>
#include <QtNetwork>
#include <QtDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    if (app.arguments().count() < 2) {
        qDebug() << "usage" << app.arguments().at(0) << " host:port";
        return -1;
    }

    QRegularExpression re(R"((\S+)?\:(\d+))");
    auto m = re.match(app.arguments().at(1));
    if (!m.hasMatch()) {
        qDebug() << "cannot parse" << app.arguments().at(1);
        return -1;
    }

    QString host = m.captured(1);
    int port = m.captured(2).toInt();
    if (host.isEmpty()) host= "localhost";

    QTcpSocket sock;
    auto connf = [&sock, host, port]() {
        qDebug() << "try to connect to" << host << port;
        QTimer::singleShot(1000, [&sock, host, port]() { sock.connectToHost(host, port); });
    };

    QObject::connect(&sock, &QTcpSocket::connected, [](){
        qDebug() << "Connected";
        QCoreApplication::instance()->exit(0);
    });
    QObject::connect(&sock, static_cast<void(QTcpSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), connf);
    QTimer::singleShot(0, connf);
    return app.exec();
}
