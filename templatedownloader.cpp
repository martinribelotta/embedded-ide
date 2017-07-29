#include "templatedownloader.h"

#include <vector>

#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMainWindow>
#include <QMessageBox>
#include <QProgressDialog>
#include <QProgressDialog>
#include <QString>
#include <QString>
#include <QUrl>
#include <QUuid>

#include "appconfig.h"
#include "filedownloader.h"
#include "templatesdownloadselector.h"

TemplateDownloader::TemplateDownloader() : QObject(), tmpls_{} {}

void TemplateDownloader::requestPendantDownloads() {
    tmpls_.clear();
    downloadMetadata();
}

void TemplateDownloader::download() {
    if (!tmpls_.empty()) {
        TemplatesDownloadSelector(tmpls_).exec();
    }
}

void TemplateDownloader::downloadMetadata() {
    AppConfig& config = AppConfig::mutableInstance();
    QUrl templateUrl = config.buildTemplateUrl();
    if (!templateUrl.isValid()) templateUrl = QUrl(config.buildTemplateUrl());
    if (!templateUrl.isValid()) {
        if (!isSilent())
            QMessageBox::critical(
                    nullptr, QMainWindow::tr("Error"),
                    QMainWindow::tr("No valid URL: %1").arg(templateUrl.toString()));
        qDebug() << "No valid URL:" << templateUrl.toString();
        emit finished(false);
        return;
    }
    auto  downloader = new FileDownloader(this);
    connect(downloader, &FileDownloader::downloadError,
            [downloader, this](const QString& msg) {
        if (!isSilent())
            QMessageBox::critical(nullptr, QMainWindow::tr("Network error"), msg);
        qDebug() << "Network error" << msg;
        downloader->deleteLater();
        emit finished(false);
    });
    connect(downloader, &FileDownloader::downloadDataFinished,
            [this](const QUrl&, const QByteArray& data) {
        QJsonDocument contents = QJsonDocument::fromJson(data);
        if (!contents.isNull() && contents.isArray()) {
            for (auto entry : contents.array()) {
                QJsonObject oEntry{entry.toObject()};
                QString name{oEntry.value("name").toString()};
                if (QStringList({"template", "jtemplate"}).contains(QFileInfo(name).suffix())) {
                    Template tmpl(name, oEntry.value("download_url").toString(),
                                  oEntry.value("git_url").toString());
                    if (tmpl.change() != Template::ChangeType::None) {
                        tmpls_.emplace_back(tmpl);
                    }
                }
            }
            if (!tmpls_.empty()) {
                emit newUpdatesAvailables();
            }
        }
        emit finished(true);
    });
    downloader->startDownload(templateUrl);
}
