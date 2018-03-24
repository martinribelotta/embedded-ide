#include "appconfig.h"
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

TemplateManager::TemplateManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TemplateManager)
{
    ui->setupUi(this);
    setProperty("firstTime", true);
    auto net = new QNetworkAccessManager(this);

    auto percentChange = [this](quint64 rcv, quint64 total) {
        ui->totalProgressBar->setValue(quint64(rcv*1000000)/quint64(total*10000));
    };

    auto msgLog = [this](const QString& text, const QColor& color) {
        auto cursor = ui->textBrowser->textCursor();
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::End);
        QTextCharFormat fmt;
        auto font = AppConfig::instance().loggerFont();
        fmt.setFontFamily(font.family());
        fmt.setFontPointSize(font.pointSize());
        fmt.setForeground(color);
        cursor.setCharFormat(fmt);
        cursor.insertText(text);
        cursor.insertText("\n");
        cursor.endEditBlock();
        ui->textBrowser->verticalScrollBar()->setValue(ui->textBrowser->verticalScrollBar()->maximum());
    };
    auto logError = [msgLog](const QString& text) { msgLog(text, Qt::red); };
    auto logMsg = [msgLog](const QString& text) { msgLog(text, Qt::darkBlue); };

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
    connect(this, &TemplateManager::message, logMsg);
    connect(this, &TemplateManager::errorMessage, logError);
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

        connect(reply, &QNetworkReply::downloadProgress, percentChange);
        connect(reply, &QNetworkReply::finished, [this, percentChange, reply, net]() {
            reply->deleteLater();
            ui->groupBox->setEnabled(true);
            ui->totalProgressBar->setValue(100);
            auto contents = QJsonDocument::fromJson(reply->readAll());
            for (const auto& entry : contents.array()) {
                auto oEntry = entry.toObject();
                auto name = oEntry.value("name").toString();
                if ("template" == QFileInfo(name).suffix()) {
                    auto url = QUrl(oEntry.value("download_url").toString());
                    auto hash = QByteArray::fromHex(oEntry.value("sha").toString().toLatin1());
                    auto templateItem = TemplateItem(url, hash);
                    if (itemList.contains(templateItem.file().fileName())) {
                        auto item = itemList.value(templateItem.file().fileName());
                        auto w = qobject_cast<TemplateItemWidget*>(ui->widgetList->itemWidget(item));
                        w->setTemplate(templateItem);
                    } else {
                        auto item = new QListWidgetItem();
                        auto w = new TemplateItemWidget(templateItem);
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
            int elementCount = ui->widgetList->count();
            for(int i=0; i<ui->widgetList->count(); i++) {
                auto item = ui->widgetList->item(i);
                auto w = qobject_cast<TemplateItemWidget*>(ui->widgetList->itemWidget(item));
                connect(w, &TemplateItemWidget::downloadMessage, this, &TemplateManager::message);
                connect(w, &TemplateItemWidget::downloadError, this, &TemplateManager::errorMessage);
                connect(w, &TemplateItemWidget::downloadStart, [net, w]() { w->startDownload(net); });
                connect(w, &TemplateItemWidget::downloadEnd,
                        [this, elementCount, percentChange, item, net](const TemplateItem& templateItem)
                {
                    AppConfig::instance().addHash(templateItem.file().absoluteFilePath(), templateItem.hash());
                    auto nextItem = item;
                    do {
                        ui->widgetList->removeItemWidget(nextItem);
                        delete ui->widgetList->takeItem(0);
                        percentChange(elementCount - ui->widgetList->count(), elementCount);
                        nextItem = ui->widgetList->item(0);
                        auto nextW = qobject_cast<TemplateItemWidget*>(ui->widgetList->itemWidget(nextItem));
                        if (!nextW)
                            break;
                        if (nextW->isChecked()) {
                            nextW->startDownload(net);
                            break;
                        }
                        emit message(tr("Skipping download of %1").arg(nextW->templateItem().file().fileName()));
                    } while(nextItem);
                    ui->groupBox->setEnabled(ui->widgetList->count() == 0);
                });
            }
        });
        connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
                [this, reply] (QNetworkReply::NetworkError)
        {
            ui->groupBox->setEnabled(true);
            reply->deleteLater();
            emit errorMessage(tr("Network error downloading template metadata: %1")
                              .arg(reply->errorString()));
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

void TemplateManager::setRepositoryUrl(const QUrl &url)
{
    ui->repoUrl->setText(url.toString());
}

void TemplateManager::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    if (property("firstTime").toBool()) {
        ui->updateRepository->click();
        setProperty("firstTime", false);
    }
}

void TemplateManager::updateLocalTemplates()
{
    auto templates = QDir(AppConfig::instance().templatesPath()).entryInfoList({ "*.template" });
    itemList.clear();
    ui->widgetList->clear();
    for(const QFileInfo& t: templates) {
        auto item = new QListWidgetItem(ui->widgetList);
        auto w = new TemplateItemWidget(TemplateItem(t));
        ui->widgetList->setItemWidget(item, w);
        itemList.insert(t.fileName(), item);
        item->setSizeHint(w->sizeHint());
    }
}
