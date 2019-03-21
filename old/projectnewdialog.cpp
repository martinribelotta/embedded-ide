#include "projectnewdialog.h"
#include "ui_projectnewdialog.h"
#include "appconfig.h"

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QPushButton>
#include <QFileDialog>
#include <QItemDelegate>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCheckBox>
#include <QStyledItemDelegate>

#include <QtDebug>

struct Parameter_t {
    QString name;
    QString type;
    QString value;
};

class EditableItemDelegate : public QStyledItemDelegate {
public:
    EditableItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {
    }

    virtual QVariant getDefault() = 0;
};

class ListEditorDelegate : public EditableItemDelegate {
public:
    const QStringList items;

    ListEditorDelegate(const QString& data, QObject *parent = nullptr) :
        EditableItemDelegate(parent), items(data.split('|'))
    {
    }

    QVariant getDefault() override {
        return items.first();
    }

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        auto cb = new QComboBox(parent);
        cb->addItems(items);
        return cb;
    }
};


class StringEditorDelegate : public EditableItemDelegate {
public:
    QString data;
    StringEditorDelegate(QString  d, QObject *parent = nullptr) :
        EditableItemDelegate(parent), data(std::move(d))
    {
    }

    QVariant getDefault() override {
        return data;
    }

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        auto ed = new QLineEdit(parent);
        ed->setText(data);
        return ed;
    }
};

#if 0
class CheckEditorDelegate : public EditableItemDelegate {
public:
    bool data;
    CheckEditorDelegate(QString  d, QObject *parent = nullptr) :
        EditableItemDelegate(parent), data(d.compare("true"))
    {
    }

    QVariant getDefault() override {
        return data;
    }

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        auto ed = new QCheckBox(parent);
        ed->setStyleSheet("QCheckBox {margin-left: 43%; margin-right: 57%;}");
        ed->setChecked(data);
        return ed;
    }
};
#else
class BooleanCheckBoxDelegate : public EditableItemDelegate
{
public:
    bool data;

    BooleanCheckBoxDelegate(QString d, QObject *parent = 0):
        EditableItemDelegate(parent), data(d.compare("true")){}

    QVariant getDefault() override {
        return data;
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        //create the checkbox editor
        QCheckBox *editor = new QCheckBox(parent);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        //set if checked or not
        QCheckBox *cb = qobject_cast<QCheckBox *>(editor);
        cb->setChecked(index.data().toBool());
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        QCheckBox *cb = static_cast<QCheckBox *>(editor);
        int value = (cb->checkState()==Qt::Checked)?1:0;
        model->setData(index, value, Qt::EditRole);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (index.column() == 0) {
            EditableItemDelegate::paint(painter, option, index);
            return;
        }
        //retrieve data
        bool data = index.model()->data(index, Qt::DisplayRole).toBool();

        //create CheckBox style
        QStyleOptionButton checkboxstyle;
        QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxstyle);

        //center
        checkboxstyle.rect = option.rect;
        checkboxstyle.rect.setLeft(option.rect.x() +
                                   option.rect.width()/2 - checkbox_rect.width()/2);
        //checked or not checked
        if(data)
            checkboxstyle.state = QStyle::State_On|QStyle::State_Enabled;
        else
            checkboxstyle.state = QStyle::State_Off|QStyle::State_Enabled;

        //done! we can draw!
        QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkboxstyle, painter);

    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(index);
        QStyleOptionButton checkboxstyle;
        QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxstyle);

        //center
        checkboxstyle.rect = option.rect;
        checkboxstyle.rect.setLeft(option.rect.x() +
                                   option.rect.width()/2 - checkbox_rect.width()/2);

        editor->setGeometry(checkboxstyle.rect);
    }

};
#endif

class DelegateFactory {
public:
    static EditableItemDelegate *create(const Parameter_t& p, QWidget *parent = nullptr)
    {
        Constructor constructor = constructors().value( p.type );
        if ( constructor == nullptr )
            return nullptr;
        return (*constructor)( p.value, parent );
    }

    template<typename T>
    static void registerClass(const QString& key)
    {
        constructors().insert( key, &constructorHelper<T> );
    }
private:
    typedef EditableItemDelegate* (*Constructor)( const QString& data, QWidget* parent );

    template<typename T>
    static EditableItemDelegate* constructorHelper( const QString& data, QWidget* parent )
    {
        return new T( data, parent );
    }

    static QHash<QString, Constructor>& constructors()
    {
        static QHash<QString, Constructor> instance;
        return instance;
    }
};

QString projectPath(const QString& path)
{
    QDir projecPath(path.isEmpty()? AppConfig::mutableInstance().defaultProjectPath() : path);
    if (!projecPath.exists())
        projecPath.mkpath(".");
    return projecPath.absolutePath();
}

static QString readAll(const QString& fileName) {
    QFile f(fileName);
    return f.open(QFile::ReadOnly)? QTextStream(&f).readAll() : QString();
}

static QJsonObject loadJSONMetadata(const QString& text)
{
    int endOfJson = text.indexOf(QRegularExpression("^diff ", QRegularExpression::MultilineOption));
    if (endOfJson == -1)
        return QJsonObject();
    auto str = text.leftRef(endOfJson).toLatin1();
    return QJsonDocument::fromJson(str).object();
}

static QJsonObject loadJSONMetadata(QFile& f)
{
    return loadJSONMetadata(readAll(f.fileName()));
}

ProjectNewDialog::ProjectNewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectNewDialog)
{
    DelegateFactory::registerClass<ListEditorDelegate>("items");
    DelegateFactory::registerClass<StringEditorDelegate>("string");
    DelegateFactory::registerClass<BooleanCheckBoxDelegate>("bool");
    QDir defTemplates(":/build/templates/");
    QDir localTemplates(
          AppConfig::mutableInstance().buildTemplatePath());
    ui->setupUi(this);
    ui->parameterTable->horizontalHeader()->setStretchLastSection(true);
    QStringList prjList;
    int defaultIdx = 0;
    for(const auto& info: localTemplates.entryInfoList(QStringList{"*.template", "*.jtemplate"})) {
        if (info.suffix() == "jtemplate") {
            QFile f(info.absoluteFilePath());
            auto root = loadJSONMetadata(f);
            if (!root.isEmpty()) {
                auto name = root.value("name").toString();
                if (!prjList.contains(name))
                    ui->templateFile->addItem(name, info.absoluteFilePath());
            }
        } else {
            if (!prjList.contains(info.baseName()))
                ui->templateFile->addItem(info.baseName(), info.absoluteFilePath());
        }
    }
    // Prefer downloaded template name over bundled template
    for(const auto& info: defTemplates.entryInfoList(QStringList{"*.template", "*.jtemplate"})) {
        if (info.suffix() == "jtemplate") {
            QFile f(info.absoluteFilePath());
            auto root = loadJSONMetadata(f);
            if (!root.isEmpty()) {
                auto name = root.value("name").toString();
                if (!prjList.contains(name))
                    ui->templateFile->addItem(name, info.absoluteFilePath());
            }
        } else {
            if (!prjList.contains(info.baseName()))
                ui->templateFile->addItem(info.baseName(), info.absoluteFilePath());
            if (info.baseName() == "empty")
                defaultIdx = ui->templateFile->count() - 1;
        }
    }
    ui->templateFile->setCurrentIndex(defaultIdx);
    ui->projectPath->setText(::projectPath(AppConfig::mutableInstance().buildDefaultProjectPath()));
    refreshProjectName();
}

ProjectNewDialog::~ProjectNewDialog()
{
    delete ui;
}

QString ProjectNewDialog::projectPath() const
{
    return ui->projectFileText->text();
}

static QString readAllText(QFile &f) {
    QTextStream s(&f);
    return s.readAll();
}

QString ProjectNewDialog::templateText() const
{
    QFile f(ui->templateFile->currentData(Qt::UserRole).toString());
    return f.open(QFile::ReadOnly)? replaceTemplates(readAllText(f)) : QString();
}

void ProjectNewDialog::refreshProjectName()
{
    ui->projectFileText->setText(QString("%1%2%3")
                                 .arg(ui->projectPath->text())
                                 .arg(ui->projectPath->text().isEmpty()? "" : QString(QDir::separator()))
                                 .arg(ui->projectName->text()));
    QString tamplate_path = ui->templateFile->currentData(Qt::UserRole).toString();
    bool en = QFileInfo(tamplate_path).exists() &&
            QFileInfo(ui->projectPath->text()).exists() &&
            !ui->projectName->text().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(en);
}

void ProjectNewDialog::on_toolFindProjectPath_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Project location"));
    if (!path.isEmpty())
        ui->projectPath->setText(path);
}

void ProjectNewDialog::on_toolLoadTemplate_clicked()
{
    QString templateName = QFileDialog::getOpenFileName(this,
                                                        tr("Open template"),
                                                        QString(),
                                                        tr("Template files (*.template *.jtemplate);;"
                                                           "Diff files (*.diff);;"
                                                           "All files (*)"));
    if (!templateName.isEmpty()) {
        QFileInfo info(templateName);
        int idx = ui->templateFile->findText(info.baseName());
        if (idx == -1)
            ui->templateFile->insertItem(idx = 0, info.baseName(), info.absoluteFilePath());
        ui->templateFile->setCurrentIndex(idx);
    }
}

void ProjectNewDialog::on_templateFile_editTextChanged(const QString &fileName)
{
    Q_UNUSED(fileName);
    QString name = ui->templateFile->currentData(Qt::UserRole).toString();
    QString text = readAll(name);
    ui->parameterTable->setRowCount(0);
    if (!text.isEmpty()) {
        QFileInfo inf(name);
        if (inf.suffix() == "jtemplate") {
            setProperty("isJTemplate", true);
            auto metadata = loadJSONMetadata(text);
            auto name = metadata.value("name").toString();
            auto author = metadata.value("author").toString();
            auto comment = metadata.value("comment").toString();
            ui->infoView->setText(
                        tr("<html><body>"
                           "<center><h1>%1</h1></center>"
                           "<h3><font color=\"blue\">%2</font></h3>"
                           "<tt>%3</tt>"
                           "</body></html>")
                        .arg(name.toHtmlEscaped())
                        .arg(author.toHtmlEscaped())
                        .arg(comment.toHtmlEscaped())
                        );
            QJsonObject params = metadata.value("params").toObject();
            for(auto key: params.keys()) {
                auto oval = params.value(key).toObject();
                auto text = oval.value("text").toString();
                auto val = oval.value("value");
                Parameter_t p = { text, "", val.toString() };
                if (val.isArray()) {
                    p.type = "items";
                    auto d = val.toArray();
                    QStringList d2;
                    for(auto n: d)
                        d2.append(n.toString());
                    p.value = d2.join('|');
                } else if (val.isBool()) {
                    // FIXME: Bool not work properly
                    p.type = "bool";
                    p.value = val.toBool()? "true" : "false";
                } else {
                    p.type = "string";
                    p.value = val.toString();
                }
                auto name = new QTableWidgetItem(p.name);
                name->setData(Qt::UserRole, key);
                name->setFlags(name->flags()&~Qt::ItemIsEditable);
                QTableWidgetItem *value = new QTableWidgetItem(QString());
                value->setFlags(value->flags()|Qt::ItemIsEditable);
                int rows = ui->parameterTable->rowCount();
                ui->parameterTable->setRowCount(rows+1);
                ui->parameterTable->setItem(rows, 0, name);
                ui->parameterTable->setItem(rows, 1, value);
                EditableItemDelegate *ed = DelegateFactory::create(p, this);
                value->setData(Qt::EditRole, ed->getDefault());
                ui->parameterTable->setItemDelegateForRow(rows, ed);
            }
        } else {
            ui->infoView->clear();
            setProperty("isJTemplate", false);
            QRegularExpressionMatch matchComments = QRegularExpression(R"((^[\s\S]*?)^diff )", QRegularExpression::MultilineOption).match(text);
            if (matchComments.hasMatch()) {
                auto text = matchComments.captured(1);
                ui->infoView->setText(text);
            }
            QRegExp re(R"(\$\{\{([a-zA-Z0-9_]+)\s*([a-zA-Z0-9_]+)*\s*\:*(.*)\}\})");
            re.setMinimal(true);
            re.setPatternSyntax(QRegExp::RegExp2);
            int idx = 0;
            while ((idx = re.indexIn(text, idx)) != -1) {
                Parameter_t p = { re.cap(1).replace('_', ' '), re.cap(2), re.cap(3) };
                auto name = new QTableWidgetItem(p.name);
                name->setFlags(name->flags()&~Qt::ItemIsEditable);
                QTableWidgetItem *value = new QTableWidgetItem(QString());
                value->setFlags(value->flags()|Qt::ItemIsEditable);
                int rows = ui->parameterTable->rowCount();
                ui->parameterTable->setRowCount(rows+1);
                ui->parameterTable->setItem(rows, 0, name);
                ui->parameterTable->setItem(rows, 1, value);
                EditableItemDelegate *ed = DelegateFactory::create(p, this);
                value->setData(Qt::EditRole, ed->getDefault());
                ui->parameterTable->setItemDelegateForRow(rows, ed);
                idx += re.matchedLength();
            }
        }
        ui->parameterTable->resizeColumnToContents(0);
    }
}

QString ProjectNewDialog::replaceTemplates(QString text) const
{
    for(int row=0; row<ui->parameterTable->rowCount(); row++) {
        if (property("isJTemplate").toBool()) {
            QString key = ui->parameterTable->item(row, 0)->data(Qt::UserRole).toString();
            QString val = ui->parameterTable->item(row, 1)->text();
            QRegExp re(QString(R"(\$\{\{%1\}\})").arg(key));
            re.setMinimal(true);
            re.setPatternSyntax(QRegExp::RegExp2);
            text.replace(re, val);
        } else {
            QString key = ui->parameterTable->item(row, 0)->text().replace(' ', '_');
            QString val = ui->parameterTable->item(row, 1)->text();
            QRegExp re(QString(R"(\$\{\{%1\s*([a-zA-Z0-9_]+)*\s*\:*(.*)\}\})").arg(key));
            re.setMinimal(true);
            re.setPatternSyntax(QRegExp::RegExp2);
            text.replace(re, val);
        }
    }
    return text;
}
