#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>

class FileDownloaderPrivate;

class FileDownloader : public QObject
{
    Q_OBJECT

public:
    explicit FileDownloader(QObject *parent = 0);
    virtual ~FileDownloader();

signals:
    void downloadProgress(const QUrl& url, int percent);
    void downloadFinished(const QUrl& url, const QString& path);
    void downloadDataFinished(const QUrl& url, const QByteArray& data);
    void downloadError(const QString& error);
    void allDownloadsFinished();

public slots:
    void startDownload(const QUrl& url, const QString& path = QString());
    void enqueueDownload(const QUrl& url, const QString& path = QString());
    void processEnqueued();

protected:
    FileDownloaderPrivate * const d_ptr;

private:
    Q_DECLARE_PRIVATE(FileDownloader)
};

#endif // FILEDOWNLOADER_H
