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
#include "consoleinterceptor.h"
#include "templateitemwidget.h"
#include "templatemanager.h"
#include "ui_templatemanager.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QScrollBar>

static const QStringList KNOWED_TEMPLATE_SUFFIX{ "template", "tar.gz" };

TemplateManager::TemplateManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TemplateManager)
{
    ui->setupUi(this);
    const struct { QAbstractButton *b; const char *name; } buttonmap[] = {
        { ui->updateRepository, "view-refresh" },
        { ui->selectAll, "edit-select-all" },
        { ui->unselectAll, "deletecell" },
        { ui->downloadFromRepo, "edit-download" },
    };
    for (const auto& e: buttonmap)
        e.b->setIcon(QIcon{AppConfig::resourceImage({ "actions", e.name })});

    ui->unselectAll->setIcon(QIcon{AppConfig::resourceImage({ "actions", "deletecell" })});
    setProperty("firstTime", true);
    auto net = new QNetworkAccessManager(this);

    auto percentChange = [this](quint64 rcv, quint64 total) {
        if (total == 0) {
            rcv = 0;
            total = 1;
        }
        constexpr auto FRAC_A = 1000000;
        constexpr auto FRAC_B = FRAC_A / 100;
        ui->totalProgressBar->setValue(int(quint64(rcv * FRAC_A) / quint64(total * FRAC_B)));
    };

    auto setSelectAllItems = [this](bool select) {
        for (int row = 0; row < ui->widgetList->count(); row++) {
            auto item = ui->widgetList->item(row);
            auto widget = qobject_cast<TemplateItemWidget*>(ui->widgetList->itemWidget(item));
            widget->setChecked(select);
        }
    };
    connect(ui->downloadFromRepo, &QToolButton::clicked, [this, net]() {
        auto first = qobject_cast<TemplateItemWidget*>(ui->widgetList->itemWidget(ui->widgetList->item(0)));
        if (first) {
            ui->groupBox->setEnabled(false);
            ui->totalProgressBar->setValue(0);
            first->startDownload(net);
        }
    });
    connect(this, &TemplateManager::message, this, &TemplateManager::logMsg);
    connect(this, &TemplateManager::errorMessage, this, &TemplateManager::logError);
    connect(ui->selectAll, &QToolButton::clicked, [setSelectAllItems]() { setSelectAllItems(true); });
    connect(ui->unselectAll, &QToolButton::clicked, [setSelectAllItems]() { setSelectAllItems(false); });

    connect(ui->updateRepository, &QToolButton::clicked, [net, percentChange, this]() {
        updateLocalTemplates();
        ui->groupBox->setEnabled(false);
        auto reply = net->get(QNetworkRequest(QUrl(ui->repoUrl->text())));
        emit message(tr("Downloading metadata..."));
        if (!reply) {
            ui->groupBox->setEnabled(true);
            emit errorMessage(tr("Cannot download template metadata!"));
            return;
        }

        connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
                [this, reply] (QNetworkReply::NetworkError)
        {
            reply->deleteLater();
            ui->groupBox->setEnabled(true);
            emit errorMessage(tr("Network error downloading template metadata: %1")
                              .arg(reply->errorString()));
        });
        connect(reply, &QNetworkReply::downloadProgress, percentChange);
        connect(reply, &QNetworkReply::finished, [this, percentChange, reply, net]() {
            reply->deleteLater();
            ui->groupBox->setEnabled(true);
            constexpr auto PERCENT = 100;
            ui->totalProgressBar->setValue(PERCENT);
            auto contents = QJsonDocument::fromJson(reply->readAll());
            for (const auto entry : contents.array()) {
                auto oEntry = entry.toObject();
                auto name = oEntry.value("name").toString();
                if (KNOWED_TEMPLATE_SUFFIX.contains(QFileInfo(name).suffix())) {
                    auto url = QUrl(oEntry.value("download_url").toString());
                    auto hash = QByteArray::fromHex(oEntry.value("sha").toString().toLatin1());
                    auto templateItem = TemplateItem(url, hash);
                    if (itemList.contains(templateItem.file().fileName())) {
                        auto item = itemList.value(templateItem.file().fileName());
                        auto w = qobject_cast<TemplateItemWidget*>(ui->widgetList->itemWidget(item));
                        w->setTemplate(templateItem);
                    } else {
                        auto item = new QListWidgetItem();
                        auto w = new TemplateItemWidget();
                        w->setTemplate(templateItem);
                        ui->widgetList->addItem(item);
                        ui->widgetList->setItemWidget(item, w);
                        item->setSizeHint(w->sizeHint());
                        itemList.insert(templateItem.file().fileName(), item);
                    }
                }
            }
            emit message(tr("Metadata download finished"));
            ui->selectAll->click();
            AppConfig::instance().purgeHash();
            emit haveMetadata();
            int elementCount = ui->widgetList->count();
            auto onDownloadEnd =  [this, percentChange, elementCount, net](const TemplateItem& templateItem) {
                AppConfig::instance().addHash(templateItem.file().absoluteFilePath(), templateItem.hash());
                auto item = ui->widgetList->takeItem(0);
                delete item;
                percentChange(quint64(elementCount - ui->widgetList->count()), quint64(elementCount));
                if (ui->widgetList->count() > 0)
                    qobject_cast<TemplateItemWidget*>(ui->widgetList->itemWidget(ui->widgetList->item(0)))->startDownload(net);
                else
                    ui->groupBox->setEnabled(true);
            };
            for(int i=0; i<ui->widgetList->count(); i++) {
                auto item = ui->widgetList->item(i);
                auto w = qobject_cast<TemplateItemWidget*>(ui->widgetList->itemWidget(item));
                connect(w, &TemplateItemWidget::downloadMessage, [this](const QString& s) { logMsg(s); });
                connect(w, &TemplateItemWidget::downloadError, [this](const QString& s) { logError(s); });
                connect(w, &TemplateItemWidget::downloadStart, [net, w]() { w->startDownload(net); });
                // FIXME: QCoreApplication::postEvent: Unexpected null receiver
                connect(w, &TemplateItemWidget::downloadEnd, this, onDownloadEnd, Qt::QueuedConnection);
            }
        });
    });
}

TemplateManager::~TemplateManager()
{
    delete ui;
}

QUrl TemplateManager::repositoryUrl() const
{
    return QUrl(ui->repoUrl->text());
}

QList<TemplateItemWidget *> TemplateManager::itemWidgets() const
{
    QList<TemplateItemWidget *> list;
    for(int i=0; i<ui->widgetList->count(); i++) {
        auto item = ui->widgetList->item(i);
        auto w = qobject_cast<TemplateItemWidget*>(ui->widgetList->itemWidget(item));
        if (w)
            list.append(w);
    }
    return list;
}

void TemplateManager::setRepositoryUrl(const QUrl &url)
{
    ui->repoUrl->setText(url.toString());
}

void TemplateManager::startUpdate()
{
    ui->updateRepository->click();
}

void TemplateManager::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    if (property("firstTime").toBool()) {
        startUpdate();
        setProperty("firstTime", false);
    }
}

void TemplateManager::updateLocalTemplates()
{
    itemList.clear();
    ui->widgetList->clear();
    for(const auto& t: QDir(AppConfig::instance().templatesPath()).entryInfoList({ "*.template" })) {
        auto item = new QListWidgetItem(ui->widgetList);
        auto w = new TemplateItemWidget();
        w->setTemplate(TemplateItem(t));
        ui->widgetList->setItemWidget(item, w);
        itemList.insert(t.fileName(), item);
        item->setSizeHint(w->sizeHint());
    }
}

void TemplateManager::msgLog(const QString &text, const QColor &color)
{
    ConsoleInterceptor::writeMessageTo(ui->textBrowser, text, color);
}

void TemplateManager::logError(const QString &text) {
    msgLog(text + "\n", Qt::red);
}

void TemplateManager::logMsg(const QString &text) {
    msgLog(text + "\n", Qt::darkBlue);
}
