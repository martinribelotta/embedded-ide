#include "templateitemwidget.h"
#include "ui_templateitemwidget.h"

#include "appconfig.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSaveFile>
#include <QTemporaryFile>

TemplateItem::TemplateItem(const QUrl &u, const QByteArray &h) : _url(u), _hash(h)
{
    _localFile.setFile(QDir(AppConfig::instance().templatesPath()).filePath(u.fileName()));
}

TemplateItem::TemplateItem(const QFileInfo &local) : _localFile(local)
{
    _url.setHost(AppConfig::instance().templatesUrl());
    _url.setPath(_localFile.fileName());
}

TemplateItemWidget::TemplateItemWidget(const TemplateItem &item, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TemplateItemWidget),
    _item(item)
{
    ui->setupUi(this);
    setTemplate(item);
    connect(ui->currentDownload, &QToolButton::clicked, [this]() { emit downloadStart(_item); });
    connect(ui->deleteFile, &QToolButton::clicked, [this]() {
        QFile f(_item.file().absoluteFilePath());
        if (f.remove())
            emit downloadMessage(tr("Remove File %1 ok").arg(f.fileName()));
        else
            emit downloadError(tr("Error %1 removing %2").arg(f.errorString()).arg(f.fileName()));
    });
}

TemplateItemWidget::~TemplateItemWidget()
{
    delete ui;
}

void TemplateItemWidget::setChecked(bool ck)
{
    ui->downloadOption->setChecked(ck);
}

bool TemplateItemWidget::isChecked() const
{
    return ui->downloadOption->isChecked();
}

void TemplateItemWidget::setTemplate(const TemplateItem &item)
{
    _item = item;
    QString state = tr("New");
    QColor color = Qt::darkMagenta;
    if (item.file().exists()) {
        color = Qt::darkGreen;
        state = tr("Updated");
    }
    auto savedHash = AppConfig::instance().fileHash(item.file().absoluteFilePath());
    if (item.hash() != savedHash && !savedHash.isEmpty()) {
        color = Qt::darkYellow;
        state = tr("Updatable");
    }
    if (!item.url().isValid()) {
        color = Qt::darkRed;
        state = tr("Local");
    }
    ui->urlLabel->setText(tr(R"(<a href="%1">%2</a>&nbsp;<tt style="color: %3">[%4]</tt>)")
                          .arg(item.url().url())
                          .arg(item.url().fileName())
                          .arg(color.name())
                          .arg(state));
    ui->progress->setValue(0);
}

void TemplateItemWidget::startDownload(QNetworkAccessManager *net)
{
    qDebug() << _item.file().fileName() << _item.url().toString();
    if (_item.url().toString().isEmpty()) {
        emit downloadEnd(_item);
        return;
    }
    auto reply = net->get(QNetworkRequest(_item.url()));
    if (!reply) {
        emit downloadError(tr("Cannot start download to %1").arg(_item.url().toString()));
        return;
    }
    auto file = new QSaveFile(_item.file().absoluteFilePath(), this);
    connect(reply, &QNetworkReply::destroyed, file, &QFile::deleteLater);
    if (!file->open(QFile::WriteOnly)) {
        emit downloadError(tr("Cannot create file %1: %2").arg(file->fileName()).arg(file->errorString()));
        reply->deleteLater();
        return;
    }
    emit downloadMessage(tr("Start download of %1...").arg(_item.url().fileName()));
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            [this, reply](QNetworkReply::NetworkError)
    {
        emit downloadError(reply->errorString());
        emit downloadEnd(_item);
        reply->deleteLater();
    });
    connect(reply, &QNetworkReply::downloadProgress, [this](quint64 rcv, quint64 total) {
        ui->progress->setValue(int(rcv*100.00/total)/100);
    });
    connect(reply, &QNetworkReply::readyRead, [this, reply, file]() {
        file->write(reply->readAll());
    });
    connect(reply, &QNetworkReply::finished, [this, reply, file]() {
        auto targetName = _item.file().absoluteFilePath();
        if (!file->commit())
            emit downloadError(tr("Cannot write temporay file %1 to %2").arg(file->fileName()).arg(targetName));
        reply->deleteLater();
        ui->progress->setValue(100);
        emit downloadMessage(tr("Download of %1 finished ok").arg(_item.file().fileName()));
        emit downloadEnd(_item);
    });
}
