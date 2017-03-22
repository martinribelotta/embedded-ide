#include "configdialog.h"
#include "ui_configdialog.h"

#include <QSettings>
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

void adjustPath()
{
    const QChar path_separator
#ifdef Q_OS_WIN
    (';')
#else
    (':')
#endif
    ;
    QString path = qgetenv("PATH");
    QStringList pathList = path.split(path_separator);
    QStringList additional = QSettings().value("build/additional_path").toStringList()
#ifdef Q_OS_WIN
            .replaceInStrings("/", R"(\)")
#endif
    ;
    pathList = additional + pathList;
    path = pathList.join(path_separator);
    qputenv("PATH", path.toLocal8Bit());
}

static QString stylePath(const QString& styleName)
{
    return QString(":/styles/%1.xml").arg(styleName);
}

static QString styleName(const QString& stylePath)
{
    return QFileInfo(stylePath).completeBaseName();
}

QString defaultTemplateUrl()
{
    return "https://api.github.com/repos/ciaa/EmbeddedIDE-templates/contents";
}

QString defaultApplicationResources()
{
    return QDir::home().absoluteFilePath("embedded-ide-workspace");
}

QString defaultProjectPath()
{
    return QDir(defaultApplicationResources()).absoluteFilePath("projects");
}

QString defaultTemplatePath()
{
    return QDir(defaultApplicationResources()).absoluteFilePath("templates");
}

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog),
    set(new QSettings(this))
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

void ConfigDialog::load()
{
    ui->codeEditor->load(":/help/reference-code.c");
    ui->codeEditor->setReadOnly(true);
    QDir d(":/styles/");
    foreach(QString path, d.entryList(QStringList("*.xml"))) {
        ui->colorStyleComboBox->addItem(styleName(path));
    }
    ui->colorStyleComboBox->setCurrentIndex(
                ui->colorStyleComboBox->findText(
                    styleName(set->value("editor/style", "stylers.model").toString())));

    ui->fontSpinBox->setValue(set->value("editor/font/size", 10).toInt());
    ui->fontComboBox->setCurrentFont(QFont(set->value("editor/font/style", "DejaVu Sans Mono").toString()));

    ui->checkBox_saveOnAction->setChecked(set->value("editor/saveOnAction", true).toBool());
    ui->checkBox_replaceTabsWithSpaces->setChecked(set->value("editor/tabsToSpaces", true).toBool());
    ui->spinTabWidth->setValue(set->value("editor/tabWidth", 4).toInt());

    ui->projectPath->setText(set->value("build/defaultprojectpath", defaultProjectPath()).toString());
    ui->projectTemplatesPath->setText(set->value("build/templatepath", defaultTemplatePath()).toString());
    ui->templateUrl->setText(set->value("build/templateurl", defaultTemplateUrl()).toString());

    QStringList additionalPaths = set->value("build/additional_path").toStringList();
    QStringListModel *model = qobject_cast<QStringListModel*>(ui->additionalPathList->model());
    model->setStringList(additionalPaths);
}

void ConfigDialog::save()
{
    set->setValue("editor/style", stylePath(ui->colorStyleComboBox->currentText()));
    set->setValue("editor/font/size", ui->fontSpinBox->value());
    set->setValue("editor/font/style", ui->fontComboBox->currentFont().family());
    set->setValue("editor/saveOnAction", ui->checkBox_saveOnAction->isChecked());
    set->setValue("editor/tabsToSpaces", ui->checkBox_replaceTabsWithSpaces->isChecked());
    set->setValue("editor/tabWidth", ui->spinTabWidth->value());
    set->setValue("build/defaultprojectpath", ui->projectPath->text());
    set->setValue("build/templatepath", ui->projectTemplatesPath->text());
    set->setValue("build/templateurl", ui->templateUrl->text());

    QStringListModel *model = qobject_cast<QStringListModel*>(ui->additionalPathList->model());
    QStringList additionalPaths = model->stringList();
    set->setValue("build/additional_path", additionalPaths);


    adjustPath();
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
    QUrl templateUrl(ui->templateUrl->text());
    if (!templateUrl.isValid())
        templateUrl = QUrl(set->value("build/templateurl").toString());
    if (!templateUrl.isValid()) {
        QMessageBox::critical(this, tr("Error"), tr("No valid URL: %1").arg(templateUrl.toString()));
        return;
    }
    QDir templatePath(QSettings().value("build/templatepath").toString());
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
