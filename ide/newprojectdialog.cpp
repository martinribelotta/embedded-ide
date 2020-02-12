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
#include "newprojectdialog.h"
#include "templatefile.h"
#include "ui_newprojectdialog.h"

#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QItemDelegate>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include <QtDebug>

class ItemDelegate: public QItemDelegate
{
    static QComboBox *addToCombo(QComboBox *c, const QStringList& items) {
        c->addItems(items);
        return c;
    }

public:
    ItemDelegate(QObject *parent) : QItemDelegate(parent) { }
    ~ItemDelegate() override;

    static QStandardItem *addUserData(QStandardItem *item, const QVariant& data)
    {
        item->setData(data, Qt::UserRole);
        return item;
    }

    static QStandardItem *addUserData(QStandardItem *item, const QString& type, const QString& data)
    {
        item->setEditable(true);
        if (type == "items") {
            QStringList v = data.split('|');
            item->setData(v, Qt::UserRole);
            item->setText(v.first());
        } else if (type == "string" ) {
            item->setData(data, Qt::UserRole);
            item->setText(data);
        }
        return item;
    }

    static QStandardItemModel *extractParameterToModel(QTableView *parent, const QList<DiffParameter> &parameters)
    {
        auto model = new QStandardItemModel(parent);
        /*QRegularExpression re(R"(\$\{\{(?P<name>[a-zA-Z_0-9]+)(?:\s+(?P<type>string|items)\:(?P<params>.*?))?\}\})",
                              QRegularExpression::MultilineOption);*/
        for(const auto& p: parameters) {
            const auto& name = p.name;
            const auto& type = p.type;
            const auto& params = p.params;
            const auto visualName = QString(name).replace('_', ' ');
            if (!type.isEmpty() && !params.isEmpty())
                model->appendRow({
                    addUserData(new QStandardItem(visualName), name),
                    addUserData(new QStandardItem(), type, params )
                });
        }
        model->setHorizontalHeaderLabels({ QObject::tr("Name"), QObject::tr("Value") });
        return model;
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem& opt, const QModelIndex &idx) const override
    {
        auto meta = idx.data(Qt::UserRole);
        switch (meta.type()) {
        case QVariant::StringList:
            return addToCombo(new QComboBox(parent), meta.toStringList());
        case QVariant::String:
            return new QLineEdit(meta.toString(), parent);
        default:
            return QItemDelegate::createEditor(parent, opt, idx);
        }
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        QString value = index.model()->data(index, Qt::EditRole).toString();

        auto *cBox = qobject_cast<QComboBox*>(editor);
        if (cBox) {
            cBox->setCurrentIndex(cBox->findText(value));
            return;
        }
        auto *ed = qobject_cast<QLineEdit*>(editor);
        if (ed) {
            ed->setText(value);
            return;
        }
        QItemDelegate::setEditorData(editor, index);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override
    {
        auto *cBox = qobject_cast<QComboBox*>(editor);
        if (cBox) {
            model->setData(index, cBox->currentText(), Qt::EditRole);
            return;
        }
        auto *ed = qobject_cast<QLineEdit*>(editor);
        if (ed) {
            model->setData(index, ed->text(), Qt::EditRole);
            return;
        }
        QItemDelegate::setModelData(editor, model, index);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const override
    {
        editor->setGeometry(option.rect);
    }
};

NewProjectDialog::NewProjectDialog(QWidget *parent) :
    QDialog(parent),
    ui(std::make_unique<Ui::NewProjectDialog>())
{
    ui->setupUi(this);
    const struct { QAbstractButton *b; const char *name; } buttonmap[] = {
        { ui->pathSelect, "document-open" },
        { ui->templateSelect, "document-open" },
        { ui->buttonOk, "dialog-ok-apply" },
        { ui->buttonCancel, "dialog-close" },
    };
    for (const auto& e: buttonmap)
        e.b->setIcon(QIcon{AppConfig::resourceImage({ "actions", e.name })});

    ui->parameterTable->setItemDelegateForColumn(1, new ItemDelegate(this));
    for(const QFileInfo& info: QDir(":/templates").entryInfoList({ "*.template" }))
        ui->templateName->addItem(info.baseName(), info.absoluteFilePath());
    for(const QFileInfo& info: QDir(AppConfig::instance().templatesPath()).entryInfoList({ "*.template", "*.tar.gz" }))
        if (ui->templateName->findData(info.absoluteFilePath()) == -1)
            ui->templateName->addItem(info.baseName(), info.absoluteFilePath());

    auto completePath = [this]() {
        ui->projectNameAndPath->setText(QDir(ui->projectPath->text()).absoluteFilePath(ui->projectName->text()));
        ui->buttonOk->setDisabled(ui->projectPath->text().isEmpty() || ui->projectName->text().isEmpty());
    };
    connect(ui->projectName, &QLineEdit::textChanged, completePath);
    connect(ui->projectPath, &QLineEdit::textChanged, completePath);

    auto templateSelectedCallback = [this](int index) {
        if (ui->parameterTable->model()) {
            ui->parameterTable->model()->deleteLater();
            ui->parameterTable->setModel(nullptr);
        }
        auto path = ui->templateName->itemData(index).toString();
        auto tm = TemplateFile{QFileInfo{path}};
        if (tm.type() == TemplateFile::Type::DiffFile) {
            DiffFile dFile{path};
            ui->infoView->setHtml(dFile.manifest);
            ui->parameterTable->setModel(ItemDelegate::extractParameterToModel(ui->parameterTable, dFile.parameters));
            ui->parameterTable->resizeColumnToContents(0);
            ui->parameterTable->resizeRowsToContents();
        } else if (tm.type() == TemplateFile::Type::TarGzFile) {
            TarGzFile tFile{path};
            if (QStringList{ "htm", "html" }.contains(tFile.metadataSuffix))
                ui->infoView->setHtml(tFile.metadata);
            else if (tFile.metadataSuffix == "md")
                ui->infoView->setHtml(MarkdownView::renderHtml(tFile.metadata));
            else
                ui->infoView->setPlainText(tFile.metadata);
        } else
            ui->infoView->setHtml(tr("<h1>Compressed project in tar.gz from:</h1><p><tt>%1</tt>").arg(path));
    };
    connect(ui->templateName, QOverload<int>::of(&QComboBox::currentIndexChanged), templateSelectedCallback);
    connect(ui->pathSelect, &QToolButton::clicked, [this]() {
        auto path = QFileDialog::getExistingDirectory(this, tr("Select Directory"), ui->projectPath->text());
        if (!path.isEmpty())
            ui->pathSelect->setText(path);
    });
    connect(ui->templateSelect, &QToolButton::clicked, [this]() {
        auto dir = AppConfig::instance().templatesPath();
        auto path = QFileDialog::getOpenFileName(this, tr("Select File"), dir,
                                                 TemplateFile::TEMPLATE_FILEDIALOG_FILTER);
        if (!path.isEmpty()) {
            ui->templateName->insertItem(0, QFileInfo(path).baseName(), path);
            ui->templateName->setCurrentIndex(0);
        }
    });

    completePath();
    templateSelectedCallback(ui->templateName->findData(":/templates/empty.template"));
    ui->projectPath->setText(AppConfig::instance().projectsPath());
}

NewProjectDialog::~NewProjectDialog()
{
}

QString NewProjectDialog::absoluteProjectPath() const
{
    return ui->projectNameAndPath->text();
}

QString NewProjectDialog::templateFile() const
{
    if (!isTemplate())
        return QString{};
    auto templatePath = ui->templateName->currentData().toString();
    qDebug() << "create project from template" << templatePath;
    auto text = QString(AppConfig::readEntireTextFile(templatePath));
    auto m = ui->parameterTable->model();
    for(int row = 0; row < m->rowCount(); row++) {
        auto name = m->data(m->index(row, 0), Qt::UserRole).toString().replace(' ', '_');
        auto value = m->data(m->index(row, 1), Qt::EditRole).toString();
        auto reText = QString(R"(\$\{\{%1\s+.*\}\})").arg(name);
        qDebug() << name << value << reText;
        text.replace(QRegularExpression(reText), value);
    }
    return text;
}

QFileInfo NewProjectDialog::selectedTemplateFile() const
{
    return QFileInfo(ui->templateName->currentData().toString());
}

bool NewProjectDialog::isTemplate() const
{
    return selectedTemplateFile().suffix().compare("template", Qt::CaseInsensitive) == 0;
}

ItemDelegate::~ItemDelegate() = default;
