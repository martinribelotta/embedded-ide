#include "configdialog.h"
#include "ui_configdialog.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QStringListModel>
#include <QProcess>
#include <QProcessEnvironment>
#include <QProgressDialog>
#include <QNetworkRequest>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <QDebug>

#include "filedownloader.h"

#include "appconfig.h"

static QString stylePath(const QString& styleName)
{
    return QString(":/styles/%1.xml").arg(styleName);
}

static QString styleName(const QString& stylePath)
{
    return QFileInfo(stylePath).completeBaseName();
}

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    ui->additionalPathList->setModel(new QStringListModel(this));

    load();

    connect(ui->fontComboBox, SIGNAL(activated(int)), this, SLOT(refreshEditor()));
    connect(ui->fontSpinBox, SIGNAL(valueChanged(int)), this, SLOT(refreshEditor()));
    connect(ui->colorStyleComboBox, SIGNAL(activated(int)), this, SLOT(refreshEditor()));

    refreshEditor();
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::setEditorSaveOnAction(bool val)
{
  AppConfig::mutableInstance().setEditorSaveOnAction(val);
}

void ConfigDialog::load()
{
    ui->codeEditor->load(":/help/reference-code.c");
    ui->codeEditor->setReadOnly(true);
    QDir d(":/styles/");
    foreach(QString path, d.entryList(QStringList("*.xml"))) {
        ui->colorStyleComboBox->addItem(styleName(path));
    }

    AppConfig& config = AppConfig::mutableInstance();

    ui->colorStyleComboBox->setCurrentIndex(
          ui->colorStyleComboBox->findText(styleName(config.editorStyle())));

    ui->fontSpinBox->setValue(config.editorFontSize());
    ui->fontComboBox->setCurrentFont(config.editorFontStyle());

    ui->checkBox_saveOnAction->setChecked(config.editorSaveOnAction());
    ui->checkBox_replaceTabsWithSpaces->setChecked(config.editorTabsToSpaces());
    ui->spinTabWidth->setValue(config.editorTabWidth());

    ui->projectPath->setText(config.builDefaultProjectPath());
    ui->projectTemplatesPath->setText(config.builTemplatePath());
    ui->templateUrl->setText(config.builTemplateUrl());

    QStringListModel *model =
        qobject_cast<QStringListModel*>(ui->additionalPathList->model());
    model->setStringList(config.buildAdditionalPaths());
}

void ConfigDialog::save()
{
  AppConfig& config = AppConfig::mutableInstance();
  config.setEditorStyle(ui->colorStyleComboBox->currentText());
  config.setEditorFontSize(ui->fontSpinBox->value());
  config.setEditorFontStyle(ui->fontComboBox->currentFont().family());
  config.setEditorSaveOnAction(ui->checkBox_saveOnAction->isChecked());
  config.setEditorTabsToSpaces(ui->checkBox_replaceTabsWithSpaces->isChecked());
  config.setEditorTabWidth(ui->spinTabWidth->value());
  config.setBuilDefaultProjectPath(ui->projectPath->text());
  config.setBuilTemplatePath(ui->projectTemplatesPath->text());
  config.setBuilTemplateUrl(ui->templateUrl->text());
  QStringListModel *model = qobject_cast<QStringListModel*>(ui->additionalPathList->model());
  QStringList additionalPaths = model->stringList();
  config.setBuildAdditionalPaths(model->stringList());
  config.save();
}

void ConfigDialog::on_buttonBox_accepted()
{
    save();
}

void ConfigDialog::refreshEditor()
{
    QString currentStyle = ui->colorStyleComboBox->currentText();
    ui->codeEditor->loadStyle(stylePath(currentStyle));
}

void ConfigDialog::on_projectPathSetButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select directory"), QDir::homePath());
    if (!path.isEmpty()) {
        ui->projectPath->setText(path);
    }
}

void ConfigDialog::on_tbPathAdd_clicked()
{
    QString path = QDir::homePath();
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select file"), path);
    if (!dir.isEmpty()) {
        QStringListModel *model = qobject_cast<QStringListModel*>(ui->additionalPathList->model());
        QStringList list = model->stringList();
        list.append(dir);
        model->setStringList(list);
    }
}

void ConfigDialog::on_tbPathRm_clicked()
{
    foreach(QModelIndex idx, ui->additionalPathList->selectionModel()->selectedIndexes()) {
        ui->additionalPathList->model()->removeRow(idx.row());
    }
}

void ConfigDialog::on_projectTemplatesPathChange_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select directory"), QDir::homePath());
    if (!path.isEmpty()) {
        ui->projectTemplatesPath->setText(path);
    }
}

void ConfigDialog::on_projectTemplatesDownload_clicked()
{
  AppConfig& config = AppConfig::mutableInstance();
    QUrl templateUrl(ui->templateUrl->text());
    if (!templateUrl.isValid())
        templateUrl = QUrl(config.builTemplateUrl());
    if (!templateUrl.isValid()) {
        QMessageBox::critical(this, tr("Error"), tr("No valid URL: %1").arg(templateUrl.toString()));
        return;
    }
    QDir templatePath(config.builTemplatePath());
    if (!templatePath.exists()) {
        if (!QDir::root().mkpath(templatePath.absolutePath())) {
            QMessageBox::critical(this, tr("Error"), tr("Error creating %1")
                                  .arg(templatePath.absolutePath()));
            return;
        }
    }

    QProgressDialog *dialog = new QProgressDialog(this);
    dialog->setWindowModality(Qt::WindowModal);
    dialog->setLabelText(tr("Downloading template list..."));
    dialog->setAutoClose(false);
    dialog->setAutoReset(false);
    dialog->setProperty("ignoreFirst", true);
    dialog->show();

    FileDownloader *downloader = new FileDownloader(this);
    connect(downloader, &FileDownloader::allDownloadsFinished, [this, dialog, downloader] ()
    {
        if (!dialog->property("ignoreFirst").toBool()) {
            dialog->close();
            dialog->deleteLater();
            downloader->deleteLater();
        } else {
            dialog->setProperty("ignoreFirst", false);
        }
    });
    connect(downloader, &FileDownloader::downloadError, [downloader, dialog, this] (const QString& msg) {
        QMessageBox::critical(this, tr("Network error"), msg);
        downloader->deleteLater();
        dialog->close();
        dialog->deleteLater();
    });
    connect(downloader, &FileDownloader::downloadProgress, [this, dialog](const QUrl& url, int percent)
    {
        QString msg = tr("Downloading %1").arg(url.fileName());
        dialog->setLabelText(msg);
        dialog->setValue(percent);
    });
    connect(downloader, &FileDownloader::downloadDataFinished,
            [downloader, templatePath] (const QUrl& url, const QByteArray& data)
    {
        Q_UNUSED(url);
        QJsonDocument contents = QJsonDocument::fromJson(data);
        if (!contents.isNull() && contents.isArray()) {
            foreach(QJsonValue entry, contents.array()) {
                QJsonObject oEntry = entry.toObject();
                QString name = oEntry.value("name").toString();
                QString download_url = oEntry.value("download_url").toString();
                if (QFileInfo(name).suffix() == "template") {
                    QString localPath = templatePath.absoluteFilePath(name);
                    downloader->enqueueDownload(QUrl(download_url), localPath);
                }
            }
            downloader->processEnqueued();
        }
    });
    downloader->startDownload(templateUrl);
}
