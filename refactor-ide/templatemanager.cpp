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
            first->startDownload(net);
        }
    });
    connect(this, &TemplateManager::message, logMsg);
    connect(this, &TemplateManager::errorMessage, logError);
    connect(ui->selectAll, &QToolButton::clicked, [setSelectAllItems]() { setSelectAllItems(true); });
    connect(ui->unselectAll, &QToolButton::clicked, [setSelectAllItems]() { setSelectAllItems(false); });
    connect(ui->updateRepository, &QToolButton::clicked, [net, percentChange, this]() {
        ui->groupBox->setEnabled(false);
        auto reply = net->get(QNetworkRequest(QUrl(ui->repoUrl->text())));
        emit message(tr("Downloading metadata..."));
        if (!reply) {
            ui->groupBox->setEnabled(true);
            emit errorMessage(tr("Cannot download template metadata!"));
        } else {
            connect(reply, &QNetworkReply::downloadProgress, percentChange);
            connect(reply, &QNetworkReply::finished, [this, reply, percentChange, net]() {
                reply->deleteLater();
                ui->groupBox->setEnabled(true);
                ui->totalProgressBar->setValue(100);
                auto contents = QJsonDocument::fromJson(reply->readAll());
                if (!contents.isNull() && contents.isArray()) {
                    ui->widgetList->clear();
                    auto entryList = contents.array();
                    for (const auto& entry : entryList) {
                        auto oEntry = entry.toObject();
                        auto name = oEntry.value("name").toString();
                        if ("template" == QFileInfo(name).suffix()) {
                            auto item = new QListWidgetItem();
                            auto url = QUrl(oEntry.value("download_url").toString());
                            auto hash = QByteArray::fromHex(oEntry.value("sha").toString().toLatin1());
                            auto templateItem = TemplateItem(url, hash);
                            auto w = new TemplateItemWidget(templateItem);
                            ui->widgetList->addItem(item);
                            ui->widgetList->setItemWidget(item, w);
                            item->setSizeHint(w->sizeHint());
                            int row = ui->widgetList->row(item);
                            connect(w, &TemplateItemWidget::downloadMessage, this, &TemplateManager::message);
                            connect(w, &TemplateItemWidget::downloadError, this, &TemplateManager::errorMessage);
                            connect(w, &TemplateItemWidget::downloadStart,
                                    [this, net, w, item]()
                            {
                                ui->widgetList->scrollToItem(item, QAbstractItemView::PositionAtCenter);
                                w->startDownload(net);
                            });
                            connect(w, &TemplateItemWidget::downloadEnd,
                                    [this, row, percentChange, item, hash, net](const TemplateItem& templateItem)
                            {
                                AppConfig::instance().addHash(templateItem.file().absoluteFilePath(), hash);
                                QListWidgetItem *nextItem = item;
                                int elements = property("elementCount").toInt();
                                do {
                                    ui->widgetList->removeItemWidget(nextItem);
                                    delete ui->widgetList->takeItem(0);
                                    percentChange(elements - ui->widgetList->count(), elements);
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
                    }
                }
                setProperty("elementCount", ui->widgetList->count());
                emit message(tr("Metadata download finished"));
                ui->selectAll->click();
                AppConfig::instance().purgeHash();
            });
            connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
                    [this, reply] (QNetworkReply::NetworkError)
            {
                ui->groupBox->setEnabled(true);
                reply->deleteLater();
                emit errorMessage(tr("Network error downloading template metadata: %1")
                                  .arg(reply->errorString()));
            });
        }
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
