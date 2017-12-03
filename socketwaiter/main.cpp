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
