#include "projectnewdialog.h"
#include "ui_projectnewdialog.h"

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QPushButton>
#include <QFileDialog>
#include <QSettings>
#include <QItemDelegate>

#include <QtDebug>

struct Parameter_t {
    QString name;
    QString type;
    QString value;
};

class EditableItemDelegate : public QItemDelegate {
public:
    EditableItemDelegate(QObject *parent = 0l) : QItemDelegate(parent) {
    }

    virtual QVariant getDefault() = 0;
};

class ListEditorDelegate : public EditableItemDelegate {
public:
    const QStringList items;

    ListEditorDelegate(const QString& data, QObject *parent = 0l) :
        EditableItemDelegate(parent), items(data.split('|'))
    {
    }

    virtual QVariant getDefault() {
        return items.first();
    }

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        QComboBox *cb = new QComboBox(parent);
        cb->addItems(items);
        return cb;
    }
};


class StringEditorDelegate : public EditableItemDelegate {
public:
    QString data;
    StringEditorDelegate(const QString& d, QObject *parent = 0l) :
        EditableItemDelegate(parent), data(d)
    {
    }

    virtual QVariant getDefault() {
        return data;
    }

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        QLineEdit *ed = new QLineEdit(parent);
        ed->setText(data);
        return ed;
    }
};

class DelegateFactory {
public:
    static EditableItemDelegate *create(const Parameter_t& p, QWidget *parent = 0l)
    {
        Constructor constructor = constructors().value( p.type );
        if ( constructor == 0l )
            return 0l;
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

extern QString defaultProjectPath();

QString projectPath(const QString& path)
{
    QDir projecPath(path.isEmpty()? defaultProjectPath() : path);
    if (!projecPath.exists())
        projecPath.mkpath(".");
    return projecPath.absolutePath();
}

ProjectNewDialog::ProjectNewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectNewDialog)
{
    DelegateFactory::registerClass<ListEditorDelegate>("items");
    DelegateFactory::registerClass<StringEditorDelegate>("string");
    QDir defTemplates(":/build/templates/");
    ui->setupUi(this);
    foreach(QFileInfo info, defTemplates.entryInfoList(QStringList("*.template"))) {
        ui->templateFile->addItem(info.absoluteFilePath());
    }
    ui->projectPath->setText(::projectPath(QSettings().value("build/defaultprojectpath").toString()));
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

QString ProjectNewDialog::templateText() const
{
    QFile f(ui->templateFile->lineEdit()->text());
    return f.open(QFile::ReadOnly)? replaceTemplates(f.readAll()) : QString();
}

void ProjectNewDialog::refreshProjectName()
{
    ui->projectFileText->setText(QString("%1%2%3")
                                 .arg(ui->projectPath->text())
                                 .arg(ui->projectPath->text().isEmpty()? "" : QString(QDir::separator()))
                                 .arg(ui->projectName->text()));
    bool en = QFileInfo(ui->templateFile->lineEdit()->text()).exists() &&
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
                                                        tr("Template files (*.template);;"
                                                           "Diff files (*.diff);;"
                                                           "All files (*)"));
    if (!templateName.isEmpty()) {
        int idx = ui->templateFile->findText(templateName);
        if (idx == -1)
            ui->templateFile->insertItem(idx = 0, templateName);
        ui->templateFile->setCurrentIndex(idx);
    }
}

static QString readAll(const QString& fileName) {
    QFile f(fileName);
    return f.open(QFile::ReadOnly)? f.readAll() : QString();
}

void ProjectNewDialog::on_templateFile_editTextChanged(const QString &fileName)
{
    QString text = readAll(fileName);
    if (!text.isEmpty()) {
        QRegExp re("\\$\\{\\{([a-zA-Z0-9_]+)\\s*([a-zA-Z0-9_]+)*\\s*\\:*(.*)\\}\\}");
        re.setMinimal(true);
        re.setPatternSyntax(QRegExp::RegExp2);
        int idx = 0;
        while ((idx = re.indexIn(text, idx)) != -1) {
            Parameter_t p = { re.cap(1), re.cap(2), re.cap(3) };
            QTableWidgetItem *name = new QTableWidgetItem(p.name);
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
}

QString ProjectNewDialog::replaceTemplates(const QString &text) const
{
    QString newString(text);
    for(int row=0; row<ui->parameterTable->rowCount(); row++) {
        QString key = ui->parameterTable->item(row, 0)->text();
        QString val = ui->parameterTable->item(row, 1)->text();

        QRegExp re(QString("\\$\\{\\{%1\\s*([a-zA-Z0-9_]+)*\\s*\\:*(.*)\\}\\}").arg(key));
        re.setMinimal(true);
        re.setPatternSyntax(QRegExp::RegExp2);
        newString.replace(re, val);
    }
    return newString;
}
