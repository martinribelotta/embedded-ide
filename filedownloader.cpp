#include "filedownloader.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHash>
#include <QFile>

#include <QtDebug>

struct DownloadEntry {
    QUrl url;
    QString localPath;

    DownloadEntry(const QUrl& _url, const QString& _localPath) :
        url(_url), localPath(_localPath) {}
};

class FileDownloaderPrivate {
public:
    FileDownloaderPrivate(FileDownloader * parent) :
        q_ptr(parent)
    {
        Q_Q(FileDownloader);
        net = new QNetworkAccessManager(q);
    }

    bool startDownload(const DownloadEntry& entry)
    {
        QNetworkReply *reply = net->get(QNetworkRequest(entry.url));
        if (!reply) {
            qDebug() << "FATAL not reply";
            return false;
        }
        QObject::connect(reply, &QNetworkReply::downloadProgress,
                         [entry, this] (quint64 received, quint64 bytesTotal)
        {
            Q_Q(FileDownloader);
            int percent = static_cast<int>(received*100/bytesTotal);
            emit q->downloadProgress(entry.url, percent);
        });
        QObject::connect(reply, &QNetworkReply::finished,
                         [entry, reply, this] ()
        {
            Q_Q(FileDownloader);
            if (entry.localPath.isEmpty()) {
                emit q->downloadDataFinished(entry.url, reply->readAll());
            } else {
                QFile localFile(entry.localPath);
                if (!localFile.open(QFile::WriteOnly)) {
                    emit q->downloadError(QObject::tr("Error downloading %1 to %2: %3")
                                          .arg(entry.url.toString())
                                          .arg(entry.localPath)
                                          .arg(localFile.errorString()));
                } else {
                    localFile.write(reply->readAll());
                    localFile.close();
                    qDebug() << "File " << localFile.fileName() << " writed";
                    emit q->downloadFinished(entry.url, entry.localPath);
                }
            }
            reply->deleteLater();
            processNext();
        });
        QObject::connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                [this, entry, reply] (QNetworkReply::NetworkError)
        {
            Q_Q(FileDownloader);
            emit q->downloadError(QObject::tr("Network error downloading %1: %2")
                                  .arg(reply->errorString())
                                  .arg(entry.url.toString()));
        });

        return true;
    }

    bool processNext()
    {
        Q_Q(FileDownloader);
        if (downloadQueue.isEmpty()) {
            emit q->allDownloadsFinished();
            return false;
        }
        startDownload(downloadQueue.takeFirst());
        return true;
    }

    FileDownloader * const q_ptr;
    Q_DECLARE_PUBLIC(FileDownloader)

    QNetworkAccessManager *net;
    QList<DownloadEntry> downloadQueue;
};

FileDownloader::FileDownloader(QObject *parent) :
    QObject(parent), d_ptr(new FileDownloaderPrivate(this))
{
}

FileDownloader::~FileDownloader()
{
    delete d_ptr;
}

void FileDownloader::startDownload(const QUrl &url, const QString &path)
{
    Q_D(FileDownloader);
    DownloadEntry entry(url, path);
    d->startDownload(entry);
}
void FileDownloader::enqueueDownload(const QUrl &url, const QString &path)
{
    Q_D(FileDownloader);
    DownloadEntry entry(url, path);
    d->downloadQueue.append(entry);
}

void FileDownloader::processEnqueued()
{
    Q_D(FileDownloader);
    d->processNext();
}
