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
#include "templateitemwidget.h"
#include "ui_templateitemwidget.h"

#include "appconfig.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSaveFile>
#include <QTemporaryFile>
#include <utility>

TemplateItem::TemplateItem(const QUrl &u, QByteArray h) : _url(u), _hash(std::move(h))
{
    _localFile.setFile(QDir(AppConfig::instance().templatesPath()).filePath(u.fileName()));
}

TemplateItem::TemplateItem(const QFileInfo &local) : _localFile(local)
{
    _url.setHost(AppConfig::instance().templatesUrl());
    _url.setPath(_localFile.fileName());
}

TemplateItem::State TemplateItem::state() const
{
    State s = State::New;
    if (file().exists())
        s = State::Updated;
    auto savedHash = AppConfig::instance().fileHash(file().absoluteFilePath());
    if (hash() != savedHash && !savedHash.isEmpty())
        s = State::Updatable;
    if (!url().isValid())
        s = State::Local;
    return s;
}

TemplateItemWidget::TemplateItemWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TemplateItemWidget)
{
    ui->setupUi(this);
    AppConfig::fixIconTheme(this);
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
    QColor color;
    QString state;
    switch(_item.state()) {
    case TemplateItem::State::New:
        color = Qt::darkMagenta;
        state = tr("New");
        break;
    case TemplateItem::State::Updated:
        color = Qt::darkGreen;
        state = tr("Updated");
        break;
    case TemplateItem::State::Updatable:
        color = Qt::darkYellow;
        state = tr("Updatable");
        break;
    case TemplateItem::State::Local:
        color = Qt::darkRed;
        state = tr("Local");
        break;
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
    if (!isChecked()) {
        emit downloadMessage(tr("Skipping %1").arg(_item.file().fileName()));
        emit downloadEnd(_item);
        return;
    }
    switch (_item.state()) {
    case TemplateItem::State::Local:
        emit downloadMessage(tr("%1 not downloadable").arg(_item.file().fileName()));
        emit downloadEnd(_item);
        return;
    case TemplateItem::State::Updated:
        emit downloadMessage(tr("%1 already updated").arg(_item.file().fileName()));
        emit downloadEnd(_item);
        return;
    default:
        break;
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
    connect(reply, &QNetworkReply::readyRead, [reply, file]() {
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
