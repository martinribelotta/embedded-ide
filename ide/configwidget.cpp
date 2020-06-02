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
#include "configwidget.h"
#include "ui_configwidget.h"

#include "appconfig.h"
#include "buttoneditoritemdelegate.h"
#include "envinputdialog.h"

#include <QStringListModel>
#include <QFileInfo>
#include <QFileDialog>
#include <QTimer>
#include <QInputDialog>
#include <QStandardItemModel>

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

class EnvironmentModel: public QStandardItemModel {
public:
    EnvironmentModel(const QMap<QString, QString>& m, QTableView *parent = nullptr):
        QStandardItemModel(parent)
    {
        setHorizontalHeaderLabels({ tr("Name"), tr("Value") });
        for (auto it = m.constBegin(); it != m.constEnd(); ++it)
            appendEntry(it.key(), it.value());
        connect(this, &QStandardItemModel::itemChanged, [parent]() {
            parent->resizeColumnToContents(0);
        });
    }

    void appendEntry(const QString& key, const QString& val)
    {
        appendRow({new QStandardItem(key), new QStandardItem(val)});
    }

    QMap<QString, QString> toMap() const {
        QMap<QString, QString> m;
        for (int r = 0; r < rowCount(); r++) {
            auto k = item(r, 0)->text();
            auto v = item(r, 1)->text();
            m.insert(k, v);
        }
        return m;
    }
};

ConfigWidget::ConfigWidget(QWidget *parent) :
    QDialog(parent),
    ui(std::make_unique<Ui::ConfigWidget>())
{
    ui->setupUi(this);
    const struct { QToolButton *b; const char *icon; } buttonmap[] = {
        { ui->projectPathSetButton, "document-open" },
        { ui->tbPathAdd, "list-add" },
        { ui->tbPathRm, "list-remove" },
        { ui->tbEnvAdd, "list-add" },
        { ui->tbEnvRm, "list-remove" },
    };
    for (const auto& e: buttonmap)
        e.b->setIcon(QIcon{AppConfig::resourceImage({ "actions", e.icon })});

    ui->languageList->addItems(QStringList{ "" } + AppConfig::langList());
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

    auto delegateFunc = [this](const QModelIndex& m)
    {
        auto model = ui->additionalPathList->model();
        auto currPath = AppConfig::replaceWithEnv(model->data(m).toString());
        auto newPath = QFileDialog::getExistingDirectory(window(), tr("Select directory"), currPath);
        if (!newPath.isEmpty())
            model->setData(m, newPath);
    };

    auto delegate = new ButtonEditorItemDelegate<decltype (delegateFunc)>(tr("Select File"), delegateFunc);
    ui->additionalPathList->setItemDelegateForColumn(0, delegate);

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
        path = QFileDialog::getExistingDirectory(window(), tr("Select directory"), path, nullptr);
        if (!path.isEmpty()) {
            auto m = qobject_cast<QStringListModel*>(ui->additionalPathList->model());
            auto list = m->stringList();
            list.append(path);
            m->setStringList(list);
        }
    });
    connect(ui->tbEnvRm, &QToolButton::clicked, [this]() {
        auto idx = ui->additionalEnvTable->currentIndex().row();
        if (idx != -1) {
            auto m = qobject_cast<QStandardItemModel*>(ui->additionalEnvTable->model());
            if (m) {
                m->removeRows(idx, 1);
                ui->additionalEnvTable->selectionModel()->clear();
            }
        }
    });
    connect(ui->tbEnvAdd, &QToolButton::clicked, [this]() {
        EnvInputDialog d(window());
        if (d.exec() == QDialog::Accepted) {
            auto m = static_cast<EnvironmentModel*>(ui->additionalEnvTable->model());
            if (m) {
                m->appendEntry(d.envName(), d.envValue());
            }
        }
    });
}

ConfigWidget::~ConfigWidget()
{
}

void ConfigWidget::save()
{
    auto &conf = AppConfig::instance();
    conf.setWorkspacePath(ui->workspacePath->text());
    conf.setAdditionalPaths(qobject_cast<QStringListModel*>(ui->additionalPathList->model())->stringList());
    auto envModel = static_cast<EnvironmentModel*>(ui->additionalEnvTable->model());
    if (envModel)
        conf.setAdditionalEnv(envModel->toMap());
    conf.setEditorStyle(ui->editorStyle->currentText());
    auto editorFont = ui->editorFontName->currentFont();
    editorFont.setPointSize(ui->editorFontSize->value());
    conf.setEditorFont(editorFont);
    conf.setEditorSaveOnAction(ui->saveOnActionTarget->isChecked());
    conf.setEditorTabsToSpaces(ui->editorReplaceTabs->isChecked());
    conf.setEditorTabWidth(ui->editorTabWidth->value());
    conf.setEditorShowSpaces(ui->editorShowSpaces->isChecked());
    conf.setEditorFormatterStyle(ui->formatterStyle->currentText());
    conf.setEditorDetectIdent(ui->editorDetectIdent->isChecked());
    conf.setTemplatesUrl(ui->templateSettings->repositoryUrl().toString());
    auto loggerFont = ui->loggerFontName->currentFont();
    loggerFont.setPointSize(ui->loggerFontSize->value());
    conf.setLoggerFont(loggerFont);
    conf.setNetworkProxyHost(ui->proxyHost->text());
    conf.setNetworkProxyPort(ui->proxyPort->text());
    conf.setNetworkProxyUseCredentials(ui->useAutentication->isChecked());
    conf.setNetworkProxyType([this]() {
        if(ui->systemProxy->isChecked()) {
            return AppConfig::NetworkProxyType::System;
        }
        if(ui->userProxy->isChecked()) {
            return AppConfig::NetworkProxyType::Custom;
        }
        { return AppConfig::NetworkProxyType::None;}
    }());
    conf.setNetworkProxyUsername(ui->username->text());
    conf.setNetworkProxyPassword(ui->password->text());
    conf.setProjectTemplatesAutoUpdate(ui->autoUpdateProjectTmplates->isChecked());
    conf.setUseDevelopMode(ui->useDevelopment->isChecked());
    conf.setUseDarkStyle(ui->useDarkStyle->isChecked());
    conf.setLanguage(ui->languageList->currentText());
    conf.setNumberOfJobs(ui->numberOfJobs->value());
    conf.setNumberOfJobsOptimal(ui->numberOfJobsOptimal->isChecked());
    conf.save();
}

void ConfigWidget::load()
{
    auto &conf = AppConfig::instance();
    ui->workspacePath->setText(conf.workspacePath());
    ui->additionalPathList->setModel(new QStringListModel(conf.additionalPaths(), this));
    ui->additionalEnvTable->setModel(new EnvironmentModel(conf.additionalEnv(), ui->additionalEnvTable));
    ui->additionalEnvTable->resizeColumnToContents(0);
    ui->editorStyle->setCurrentText(conf.editorStyle());
    auto editorFont = conf.editorFont();
    ui->editorFontName->setCurrentFont(editorFont);
    ui->editorFontSize->setValue(editorFont.pointSize());
    ui->saveOnActionTarget->setChecked(conf.editorSaveOnAction());
    ui->editorReplaceTabs->setChecked(conf.editorTabsToSpaces());
    ui->editorTabWidth->setValue(conf.editorTabWidth());
    ui->editorDetectIdent->setChecked(conf.editorDetectIdent());
    ui->editorShowSpaces->setChecked(conf.editorShowSpaces());
    ui->formatterStyle->setCurrentText(conf.editorFormatterStyle());
    ui->formatterExtra->setText(conf.editorFormatterExtra());
    ui->templateSettings->setRepositoryUrl(conf.templatesUrl());
    auto loggerFont = conf.loggerFont();
    ui->loggerFontName->setCurrentFont(loggerFont);
    ui->loggerFontSize->setValue(loggerFont.pointSize());
    ui->proxyHost->setText(conf.networkProxyHost());
    ui->proxyPort->setText(conf.networkProxyPort());
    ui->useAutentication->setChecked(conf.networkProxyUseCredentials());
    switch (static_cast<AppConfig::NetworkProxyType>(conf.networkProxyType())) {
    case AppConfig::NetworkProxyType::None:
        ui->noProxy->setChecked(true);
        break;
    case AppConfig::NetworkProxyType::System:
        ui->systemProxy->setChecked(true);
        break;
    case AppConfig::NetworkProxyType::Custom:
        ui->userProxy->setChecked(true);
        break;
    }
    ui->username->setText(conf.networkProxyUsername());
    ui->password->setText(conf.networkProxyPassword());
    ui->autoUpdateProjectTmplates->setChecked(conf.projectTemplatesAutoUpdate());
    ui->useDevelopment->setChecked(conf.useDevelopMode());
    ui->useDarkStyle->setChecked(conf.useDarkStyle());
    ui->languageList->setCurrentText(conf.language());
    ui->numberOfJobs->setValue(conf.numberOfJobs());
    ui->numberOfJobsOptimal->setChecked(conf.numberOfJobsOptimal());
}
