#include "findmakefiledialog.h"
#include "ui_findmakefiledialog.h"

#include <QDirIterator>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QTimer>

QString find_root(const QStringList& list) {
    QString root = list.front();
    for(const auto& item : list) {
        if (root.length() > item.length())
            root.truncate(item.length());
        for(int i = 0; i < root.length(); ++i)
            if (root[i] != item[i]) {
                root.truncate(i);
                break;
            }
    }
    return root;
}

static QStringList extractPrefix(const QStringList& list, const QString& root)
{
    QStringList copy;
    for (const auto& e: list)
        copy.append("..." + QString(e).remove(root));
    return copy;
}

FindMakefileDialog::FindMakefileDialog(const QString &path, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindMakefileDialog)
{
    ui->setupUi(this);
    setProperty("path", path);
    QTimer::singleShot(0, [this, path](){
        auto model = new QStringListModel(this);
        ui->makefileList->setModel(model);
        ui->buttonOpen->setDisabled(true);
        model->setStringList(extractPrefix(findInPath(path), path));
        ui->buttonOpen->setDisabled(false);
        if (model->rowCount() > 0) {
            ui->makefileList->setCurrentIndex(model->index(0, 0));
        }
    });
    setProperty("canceling", false);
    connect(ui->buttonCancel, &QAbstractButton::clicked, [this]() {
        reject();
        setProperty("canceling", true);
    });
    connect(ui->buttonOpen, &QAbstractButton::clicked, this, &QDialog::accept);
    connect(ui->makefileList, &QAbstractItemView::activated, this, &QDialog::accept);
}

FindMakefileDialog::~FindMakefileDialog()
{
    delete ui;
}

QString FindMakefileDialog::fileName() const
{
    auto model = qobject_cast<QStringListModel*>(ui->makefileList->model());
    if (!model)
        return {};
    auto text = model->stringList().at(ui->makefileList->currentIndex().row());
    return property("path").toString() + QDir::separator() + text.remove(0, 3);
}

QStringList FindMakefileDialog::findInPath(const QString &path)
{
    QStringList list;
    QDirIterator it(path, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        auto name = it.next();
        ui->labelLog->setText(tr("Finding in ...%1").arg(name.rightRef(30)));
        if (QFileInfo(name).fileName() == "Makefile") {
            list += name;
        }
        QCoreApplication::processEvents();
        if (property("canceling").toBool()) {
            break;
        }
    }
    ui->labelLog->setText(tr("Done."));
    return list;
}
