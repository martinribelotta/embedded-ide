#include "unsavedfilesdialog.h"
#include "ui_unsavedfilesdialog.h"

#include "filesystemmanager.h"

#include <QFileInfo>
#include <QStandardItemModel>

UnsavedFilesDialog::UnsavedFilesDialog(const QStringList& unsaved, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UnsavedFilesDialog)
{
    ui->setupUi(this);
    auto m = new QStandardItemModel(this);
    ui->listView->setModel(m);
    for(const auto& a: unsaved) {
        auto f = QFileInfo(a);
        auto item = new QStandardItem(FileSystemManager::iconForFile(f), f.fileName());
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);
        item->setData(f.absoluteFilePath());
        item->setToolTip(f.absoluteFilePath());
        m->appendRow(item);
    }
    auto setCheckAll = [m](bool ch) {
        for(int i=0; i<m->rowCount(); i++)
            m->item(i)->setCheckState(ch? Qt::Checked : Qt::Unchecked);
    };
    connect(ui->buttonAbort, &QToolButton::clicked, this, &UnsavedFilesDialog::reject);
    connect(ui->buttonSaveSelected, &QToolButton::clicked, this, &UnsavedFilesDialog::accept);
    connect(ui->buttonUnselectAll, &QToolButton::clicked, [setCheckAll]() { setCheckAll(false); });
    connect(ui->buttonSelectAll, &QToolButton::clicked, [setCheckAll]() { setCheckAll(true); });
}

UnsavedFilesDialog::~UnsavedFilesDialog()
{
    delete ui;
}

QStringList UnsavedFilesDialog::checkedForSave() const
{
    QStringList selecteds;
    auto m = qobject_cast<QStandardItemModel*>(ui->listView->model());
    for(int i=0; i<m->rowCount(); i++)
        if (m->item(i)->checkState() == Qt::Checked)
            selecteds.append(m->item(i)->data().toString());
    return selecteds;
}
