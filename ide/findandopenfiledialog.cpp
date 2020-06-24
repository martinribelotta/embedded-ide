#include "findandopenfiledialog.h"
#include "ui_findandopenfiledialog.h"

#include <QDirIterator>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QPushButton>

FindAndOpenFileDialog::FindAndOpenFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindAndOpenFileDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Open)->setDisabled(true);
    connect(ui->fileList, &QAbstractItemView::activated, this, &QDialog::accept);
}

FindAndOpenFileDialog::~FindAndOpenFileDialog()
{
    delete ui;
}

void FindAndOpenFileDialog::setFileList(const QString& prefix, const QStringList &list)
{
    if (ui->fileList->model())
        ui->fileList->model()->deleteLater();
    auto m = new QStandardItemModel(this);
    for (const auto& e: list) {
        auto item = new QStandardItem{"..." + QString{e}.remove(prefix)};
        item->setData(e, Qt::UserRole + 1);
        m->appendRow(item);
    }
    ui->fileList->setModel(m);
    ui->buttonBox->button(QDialogButtonBox::Open)->setDisabled(list.isEmpty());
}

QString FindAndOpenFileDialog::selectedFile() const
{
    auto idx = ui->fileList->currentIndex();
    return ui->fileList->model()->data(idx, Qt::UserRole + 1).toString();
}

QStringList FindAndOpenFileDialog::findFilesInPath(const QString &file, const QString &path)
{
    QStringList list;
    QDirIterator it(path, { file }, QDir::NoDotAndDotDot | QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        list.append(it.next());
    }
    return list;
}
