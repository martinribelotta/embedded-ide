#include "filereferencesdialog.h"
#include "ui_filereferencesdialog.h"

#include <QStandardItemModel>
#include <QtDebug>

FileReferencesDialog::FileReferencesDialog(const ICodeModelProvider::FileReferenceList &refList, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileReferencesDialog)
{
    ui->setupUi(this);
    connect(ui->listWidget, &QListWidget::itemActivated, [this](QListWidgetItem *item) {
        auto url = item->data(Qt::UserRole).value<QUrl>();
        auto ref = ICodeModelProvider::FileReference::decode(url);
        emit itemClicked(ref.path, ref.line);
        accept();
    });
    for(const auto& r: refList) {
        auto url = r.encode();
        auto item = new QListWidgetItem(QString("%1: %2\n%3").arg(r.path).arg(r.line).arg(r.meta));
        item->setData(Qt::UserRole, url);
        ui->listWidget->addItem(item);
    }
    ui->listWidget->setMinimumWidth(ui->listWidget->sizeHintForColumn(0) + 2 + ui->listWidget->frameWidth());
    resize(minimumWidth(), height());
}

FileReferencesDialog::~FileReferencesDialog()
{
    delete ui;
}
