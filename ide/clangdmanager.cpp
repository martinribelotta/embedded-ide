#include "clangdmanager.h"

#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QTimer>

#include <QtDebug>

class ClangdManager::Priv_t {
public:
    QProcess *clangd;
    QString buffer;
    int remaining;
};

namespace rpc {

static int getId() {
    static QMutex mutexGetId;
    static int id = 0;
    QMutexLocker lock(&mutexGetId);
    return id++;
}

static QByteArray create(const QJsonObject& v) {
    auto data = QString(QJsonDocument(v).toJson()).replace("\n", "\r\n");
    return QString("Content-Length: %1\r\n\r\n%2").arg(data.length()).arg(data).toLatin1();
}

static QJsonObject message(const QString& method, const QJsonObject& v) {
    QJsonObject o;
    o.insert("id", getId());
    o.insert("jsonrpc", "2.0");
    o.insert("method", method);
    o.insert("params", v);
    return o;
}

static QByteArray initialize(qint64 processId, const QString& uri)
{
    QJsonObject o;
    o.insert("processId", processId);
    o.insert("rootUri", uri);
    o.insert("clientCapabilitiens", QJsonObject());
    return create(message("initialize", o));
}

}

ClangdManager::ClangdManager(QObject *parent) :
    QObject(parent),
    priv(new Priv_t)
{
    priv->clangd = new QProcess(this);
    priv->clangd->setProgram("clangd");
}

ClangdManager::~ClangdManager()
{
    //priv->clangd->terminate();
    //priv->clangd->waitForFinished();
    //disconnect();
    delete priv;
}

ClangdManager *ClangdManager::instance()
{
    static ClangdManager *self = nullptr;
    if (!self)
        self = new ClangdManager(QCoreApplication::instance());
    return self;
}

void ClangdManager::start(const QString &workspace)
{
    connect(priv->clangd, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            [this](QProcess::ProcessError err) {
        // Deffer to process event for deletion condition
        QTimer::singleShot(0,[this, err]() {
            qDebug() << err << priv->clangd->errorString();
        });
    });
    connect(priv->clangd, &QProcess::started, [this, workspace]() {
        auto data = rpc::initialize(QCoreApplication::applicationPid(), workspace);
        qDebug() << data;
        priv->buffer.clear();
        priv->remaining = 0;
        priv->clangd->write(data);
    });
    connect(priv->clangd, &QProcess::readyReadStandardOutput, [this]() {
        QByteArray data = priv->clangd->readAllStandardOutput();
        if (priv->remaining == 0) {
            priv->buffer.append(data);
            QRegularExpression startRe(R"(Content-Length: (\d+)\r\n\r\n)");
            QRegularExpressionMatch m = startRe.match(priv->buffer);
            if (m.hasMatch()) {
                int count = m.captured(1).toInt();
                priv->buffer = data.mid(m.capturedStart() + m.capturedLength());
                priv->remaining = count - priv->buffer.length();
            }
        } else {
            priv->buffer.append(data);
            priv->remaining -= data.length();
        }
        if (priv->remaining <= 0) {
            //processResponse(QJsonDocument::fromJson(priv->buffer.toLatin1()));
        }
    });
    priv->clangd->setWorkingDirectory(workspace);
    priv->clangd->start();
}

void ClangdManager::addTarget(const QString &source, const QString &target, const QString &cmd)
{

}

void ClangdManager::processResponse(const QJsonObject &o)
{

}
