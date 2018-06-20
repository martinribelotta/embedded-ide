#include "configwidget.h"
#include "ui_configwidget.h"

#include "appconfig.h"

#include <QStringListModel>
#include <QFileInfo>
#include <QFileDialog>

#include <QtDebug>

static const QStringList ASTYLE_STYLES = {
    "1tbs",
    "allman",
    "attach",
    "banner",
    "break",
    "bsd",
    "gnu",
    "google",
    "horstmann",
    "java",
    "k",
    "knf",
    "kr",
    "linux",
    "lisp",
    "otbs",
    "pico",
    "stroustrup",
    "vtk",
    "whitesmith"
};

ConfigWidget::ConfigWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigWidget)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);
    for(const auto& fi: QDir(":/styles").entryInfoList({ "*.xml" }))
        ui->editorStyle->addItem(fi.baseName());
    ui->codeEditor->load(":/reference-code.c");
    ui->codeEditor->setReadonly(true);
    ui->formatterStyle->addItems(ASTYLE_STYLES);
    auto updateEditor = [this]() {
        // Defer execution to next event loop due sender actualization
        QTimer::singleShot(0, [this]() {
            auto font = ui->editorFontName->currentFont();
            font.setPointSize(ui->editorFontSize->value());
            ui->codeEditor->loadConfigWithStyle(
                        ui->editorStyle->currentText(),
                        font,
                        ui->editorTabWidth->value(),
                        ui->editorReplaceTabs->isChecked());
            ui->codeEditor->reload();
        });
    };

    connect(ui->editorStyle, &QComboBox::currentTextChanged, updateEditor);
    connect(ui->editorFontName, &QComboBox::currentTextChanged, updateEditor);
    connect(ui->editorFontSize, QOverload<int>::of(&QSpinBox::valueChanged), updateEditor);
    connect(ui->editorReplaceTabs, &QCheckBox::toggled, updateEditor);
    connect(ui->editorTabWidth, QOverload<int>::of(&QSpinBox::valueChanged), updateEditor);

    connect(ui->projectPathSetButton, &QToolButton::clicked, [this]() {
        auto dir = QFileInfo(AppConfig::instance().workspacePath()).absolutePath();
        auto path = QFileDialog::getExistingDirectory(this, tr("Select workspace directory"), dir);
        if (!path.isEmpty())
            ui->workspacePath->setText(path);
    });
    connect(ui->tbPathRm, &QToolButton::clicked, [this]() {
        auto idx = ui->additionalPathList->currentIndex().row();
        if (idx != -1) {
            auto m = qobject_cast<QStringListModel*>(ui->additionalPathList->model());
            m->removeRows(idx, 1);
        }
    });
    connect(ui->tbPathAdd, &QToolButton::clicked, [this]() {
        auto path = QDir::homePath();
        auto idx = ui->additionalPathList->currentIndex();
        if (idx.isValid()) {
            path = ui->additionalPathList->model()->data(idx, Qt::DisplayRole).toString();
            path = AppConfig::replaceWithEnv(path);
        }
        path = QFileDialog::getExistingDirectory(window(), tr("Select directory"), path, 0);
        if (!path.isEmpty()) {
            auto m = qobject_cast<QStringListModel*>(ui->additionalPathList->model());
            auto list = m->stringList();
            list.append(path);
            m->setStringList(list);
        }
    });
}

ConfigWidget::~ConfigWidget()
{
    delete ui;
}

void ConfigWidget::save()
{
    auto &conf = AppConfig::instance();
    conf.setWorkspacePath(ui->workspacePath->text());
    conf.setAdditionalPaths(qobject_cast<QStringListModel*>(ui->additionalPathList->model())->stringList());
    conf.setEditorStyle(ui->editorStyle->currentText());
    auto editorFont = ui->editorFontName->currentFont();
    editorFont.setPointSize(ui->editorFontSize->value());
    qDebug() << editorFont;
    conf.setEditorFont(editorFont);
    conf.setEditorSaveOnAction(ui->saveOnActionTarget->isChecked());
    conf.setEditorTabsToSpaces(ui->editorReplaceTabs->isChecked());
    conf.setEditorTabWidth(ui->editorTabWidth->value());
    conf.setEditorFormatterStyle(ui->formatterStyle->currentText());
    conf.setTemplatesUrl(ui->templateSettings->repositoryUrl().toString());
    auto loggerFont = ui->loggerFontName->currentFont();
    loggerFont.setPointSize(ui->loggerFontSize->value());
    conf.setLoggerFont(loggerFont);
    // TODO Implement network proxy settings
    // conf.setNetworkProxyHost(const QString& name);
    // conf.setNetworkProxyPort(const QString& port);
    // conf.setNetworkProxyUseCredentials(bool use);
    // conf.setNetworkProxyType(NetworkProxyType type);
    // conf.setNetworkProxyUsername(const QString& user);
    // conf.setNetworkProxyPassword(const QString& pass);
    conf.setProjectTemplatesAutoUpdate(ui->autoUpdateProjectTmplates->isChecked());
    conf.setUseDevelopMode(ui->useDevelopment->isChecked());
    conf.save();
}

void ConfigWidget::load()
{
    auto &conf = AppConfig::instance();
    ui->workspacePath->setText(conf.workspacePath());
    ui->additionalPathList->setModel(new QStringListModel(conf.additionalRawPaths(), this));
    ui->editorStyle->setCurrentText(conf.editorStyle());
    auto editorFont = conf.editorFont();
    ui->editorFontName->setCurrentFont(editorFont);
    ui->editorFontSize->setValue(editorFont.pointSize());
    ui->saveOnActionTarget->setChecked(conf.editorSaveOnAction());
    ui->editorReplaceTabs->setChecked(conf.editorTabsToSpaces());
    ui->editorTabWidth->setValue(conf.editorTabWidth());
    ui->formatterStyle->setCurrentText(conf.editorFormatterStyle());
    ui->formatterExtra->setText(conf.editorFormatterExtra());
    ui->templateSettings->setRepositoryUrl(conf.templatesUrl());
    auto loggerFont = conf.loggerFont();
    qDebug() << "logger font" << loggerFont;
    ui->loggerFontName->setCurrentFont(loggerFont);
    ui->loggerFontSize->setValue(loggerFont.pointSize());
    // conf.setNetworkProxyHost(const QString& name);
    // conf.setNetworkProxyPort(const QString& port);
    // conf.setNetworkProxyUseCredentials(bool use);
    // conf.setNetworkProxyType(NetworkProxyType type);
    // conf.setNetworkProxyUsername(const QString& user);
    // conf.setNetworkProxyPassword(const QString& pass);
    ui->autoUpdateProjectTmplates->setChecked(conf.projectTemplatesAutoUpdate());
    ui->useDevelopment->setChecked(conf.useDevelopMode());
}
