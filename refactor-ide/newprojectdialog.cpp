#include "appconfig.h"
#include "newprojectdialog.h"
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

static const QString textBeforeDiff(const QString& text, int *startOfDiff = nullptr)
{
    QRegularExpressionMatch m = QRegularExpression(R"((^[\s\S]*?)^diff )", QRegularExpression::MultilineOption).match(text);
    if (m.hasMatch()) {
        if (startOfDiff)
            *startOfDiff = m.capturedEnd(1);
        return m.captured(1);
    }
    return QString();
}

class ItemDelegate: public QItemDelegate
{
    static QComboBox *addToCombo(QComboBox *c, const QStringList& items) {
        c->addItems(items);
        return c;
    }

public:
    ItemDelegate(QObject *parent) : QItemDelegate(parent) { }

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

    static QStandardItemModel *extractParameterToModel(QTableView *parent, const QString& diffText, int offset)
    {
        auto model = new QStandardItemModel(parent);
        QRegularExpression re(R"(\$\{\{(?P<name>[a-zA-Z_0-9]+)(?:\s+(?P<type>string|items)\:(?P<params>.*?))?\}\})",
                              QRegularExpression::MultilineOption);
        auto it = re.globalMatch(diffText, offset);
        while (it.hasNext()) {
            auto m = it.next();
            QString name = m.captured("name");
            QString type = m.captured("type");
            QString params = m.captured("params");
            QString visualName = name;
            visualName.replace('_', ' ');
            if (!type.isEmpty() && !params.isEmpty())
                model->appendRow({ addUserData(new QStandardItem(visualName), name), addUserData(new QStandardItem(), type, params ) });
        }
        model->setHorizontalHeaderLabels({ QObject::tr("Name"), QObject::tr("Value") });
        return model;
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem& opt, const QModelIndex &idx) const
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

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QString value = index.model()->data(index, Qt::EditRole).toString();

        QComboBox *cBox = qobject_cast<QComboBox*>(editor);
        if (cBox) {
            cBox->setCurrentIndex(cBox->findText(value));
            return;
        }
        QLineEdit *ed = qobject_cast<QLineEdit*>(editor);
        if (ed) {
            ed->setText(value);
            return;
        }
        QItemDelegate::setEditorData(editor, index);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const
    {
        QComboBox *cBox = qobject_cast<QComboBox*>(editor);
        if (cBox) {
            model->setData(index, cBox->currentText(), Qt::EditRole);
            return;
        }
        QLineEdit *ed = qobject_cast<QLineEdit*>(editor);
        if (ed) {
            model->setData(index, ed->text(), Qt::EditRole);
            return;
        }
        QItemDelegate::setModelData(editor, model, index);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
    {
        editor->setGeometry(option.rect);
    }
};

NewProjectDialog::NewProjectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewProjectDialog)
{
    ui->setupUi(this);
    ui->parameterTable->setItemDelegateForColumn(1, new ItemDelegate(this));
    for(const QFileInfo& info: QDir(":/templates").entryInfoList({ "*.template" }))
        ui->templateName->addItem(info.baseName(), info.absoluteFilePath());
    for(const QFileInfo& info: QDir(AppConfig::instance().templatesPath()).entryInfoList({ "*.template" }))
        if (ui->templateName->findData(info.absoluteFilePath()) == -1)
            ui->templateName->addItem(info.baseName(), info.absoluteFilePath());

    auto completePath = [this]() {
        ui->projectNameAndPath->setText(QDir(ui->projectPath->text()).absoluteFilePath(ui->projectName->text()));
        ui->buttonOk->setDisabled(ui->projectPath->text().isEmpty() || ui->projectName->text().isEmpty());
    };
    connect(ui->projectName, &QLineEdit::textChanged, completePath);
    connect(ui->projectPath, &QLineEdit::textChanged, completePath);

    auto templateSelectedCallback = [this](int index) {
        auto path = ui->templateName->itemData(index).toString();
        auto text = AppConfig::readEntireTextFile(path);
        int startOfDiff = 0;
        ui->infoView->setHtml(textBeforeDiff(text, &startOfDiff));
        if (ui->parameterTable->model())
            ui->parameterTable->model()->deleteLater();
        ui->parameterTable->setModel(ItemDelegate::extractParameterToModel(ui->parameterTable, text, startOfDiff));
        ui->parameterTable->resizeColumnToContents(0);
        ui->parameterTable->resizeRowsToContents();
    };
    connect(ui->templateName, QOverload<int>::of(&QComboBox::activated), templateSelectedCallback);
    connect(ui->pathSelect, &QToolButton::clicked, [this]() {
        auto path = QFileDialog::getExistingDirectory(this, tr("Select Directory"), ui->projectPath->text());
        if (!path.isEmpty())
            ui->pathSelect->setText(path);
    });
    connect(ui->templateSelect, &QToolButton::clicked, [this]() {
        auto dir = AppConfig::instance().templatesPath();
        auto path = QFileDialog::getOpenFileName(this, tr("Select File"), dir, tr("Templates (*.template);;All files (*)"));
        if (!path.isEmpty())
            ui->templateName->insertItem(0, QFileInfo(path).baseName(), path);
    });

    completePath();
    templateSelectedCallback(ui->templateName->findData(":/templates/empty.template"));
    ui->projectPath->setText(AppConfig::instance().projectsPath());
}

NewProjectDialog::~NewProjectDialog()
{
    delete ui;
}

QString NewProjectDialog::absoluteProjectPath() const
{
    return ui->projectNameAndPath->text();
}

QString NewProjectDialog::templateFile() const
{
    auto templatePath = ui->templateName->currentData().toString();
    qDebug() << "create project from template" << templatePath;
    auto text = QString(AppConfig::readEntireTextFile(templatePath));
    auto m = ui->parameterTable->model();
    for(int row = 0; row < m->rowCount(); row++) {
        auto name = m->data(m->index(row, 0), Qt::UserRole).toString();
        auto value = m->data(m->index(row, 1), Qt::UserRole).toString();
        text.replace(QRegularExpression(QString(R"(\$\{\{%1\s+.*?\}\})").arg(name)), value);
    }
    return text;
}
