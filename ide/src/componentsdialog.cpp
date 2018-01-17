#include "appconfig.h"
#include "codetemplate.h"
#include "componentitemwidget.h"
#include "componentsdialog.h"
#include "filedownloader.h"
#include "ui_componentsdialog.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringListModel>
#include <QInputDialog>

#include <QtDebug>

static const QStringList TEMPLATES_FILTER{ "*.template" };

struct ComponentsDialog::Priv_t
{
    Ui_ComponentsDialog *ui;
    ComponentsDialog *self;
    QHash<QFileInfo, QListWidgetItem*> localFileList;

    Priv_t(ComponentsDialog *_self, Ui_ComponentsDialog *_ui) : self(_self), ui(_ui) {}

    void loadTemplates(const QJsonDocument& contents = QJsonDocument())
    {
        localFileList.clear();
        ui->templateList->clear();
        auto path = AppConfig::mutableInstance().buildTemplatePath();
        auto list = QDir(path).entryInfoList(TEMPLATES_FILTER, QDir::Files);
        for(const auto& e: list) {
            CodeTemplate t{e};
            localFileList.insert(t.path(), addItemToList(t));
        }
        if (!contents.isNull() && contents.isArray()) {
            auto templatePath = QDir(AppConfig::mutableInstance().buildTemplatePath());
            for(const auto& entry: contents.array()) {
                auto oEntry = entry.toObject();
                auto name = oEntry.value("name").toString();
                auto download_url = QUrl(oEntry.value("download_url").toString());
                auto localPath = QFileInfo(templatePath.absoluteFilePath(name));
                if (TEMPLATES_FILTER.contains(QString("*.%1").arg(localPath.suffix()))) {
                    CodeTemplate t(download_url, localPath);
                    auto item = localFileList.value(t.path(), nullptr);
                    if (!item)
                        item = addItemToList(t);
                    auto widget = qobject_cast<ComponentItemWidget*>(ui->templateList->itemWidget(item));
                    if (t.url().isValid())
                        widget->setUrl(t.url());
                }
            }
        }
    }

private:
    QListWidgetItem *addItemToList(const CodeTemplate& t)
    {
        auto item = new QListWidgetItem();
        auto widget = new ComponentItemWidget(t, self);
        item->setSizeHint(widget->sizeHint());
        ui->templateList->addItem(item);
        ui->templateList->setItemWidget(item, widget);
        return item;
    }
};

ComponentsDialog::ComponentsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ComponentsDialog),
    d_ptr(new Priv_t(this, ui))
{
    ui->setupUi(this);
    QStringList urlList{AppConfig::mutableInstance().buildTemplateUrl()};
    ui->urlList->setModel(new QStringListModel(urlList));
    // on_buttonUpdateRepos_clicked();
}

ComponentsDialog::~ComponentsDialog()
{
    delete d_ptr;
    delete ui;
}

void ComponentsDialog::on_buttonAddUrl_clicked()
{
    QUrl url(QInputDialog::getText(this, tr("URL Input"), tr("Enter url to add")));
    if (!url.isEmpty() && url.isValid()) {
        auto m = qobject_cast<QStringListModel*>(ui->urlList->model());
        if (m) {
            QStringList list = m->stringList();
            list.append(url.toString());
            m->setStringList(list);
        }
    }
}

void ComponentsDialog::on_buttonDelUrl_clicked()
{
    auto m = qobject_cast<QStringListModel*>(ui->urlList->model());
    if (m) {
        auto idx = ui->urlList->currentIndex();
        if (idx.isValid()) {
            auto strList = m->stringList();
            strList.removeAt(idx.row());
            m->setStringList(strList);
        }
    }
}

void ComponentsDialog::on_buttonDownloadAll_clicked()
{
}

void ComponentsDialog::on_buttonUpdateRepos_clicked()
{
    auto downloader = new FileDownloader(this);
    for(const auto& templateUrl: qobject_cast<QStringListModel*>(ui->urlList->model())->stringList())
        downloader->enqueueDownload(templateUrl);
    downloader->setProgressBar(ui->progressTotal, ui->progressItem);
    connect(downloader, &FileDownloader::downloadDataFinished, [this](const QUrl&, const QByteArray& data) {
        d_ptr->loadTemplates(QJsonDocument::fromJson(data));
    });
    connect(downloader, &FileDownloader::downloadError, [this]() { d_ptr->loadTemplates(); });
    auto finalizeDownload = [this, downloader]() {
        downloader->deleteLater();
        ui->buttonDownloadAll->setEnabled(true);
        ui->buttonUpdateRepos->setEnabled(true);
    };
    connect(downloader, &FileDownloader::downloadError, finalizeDownload);
    connect(downloader, &FileDownloader::allDownloadsFinished, finalizeDownload);
    ui->buttonDownloadAll->setEnabled(false);
    ui->buttonUpdateRepos->setEnabled(false);
    downloader->startDownload();
}
