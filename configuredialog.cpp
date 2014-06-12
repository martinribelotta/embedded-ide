#include "configuredialog.h"
#include "ui_configuredialog.h"

#include <QtDebug>
#include <QStandardItemModel>

#include <QInputDialog>
#include <QComboBox>

static void insertRow(QStandardItemModel *model, const QStringList& cols, const QString& perm) {
    QList<QStandardItem*> items;
    foreach(QString s, cols)
        items.append(new QStandardItem(s));

    QStandardItem *r = new QStandardItem("Read");
    QStandardItem *w = new QStandardItem("Write");
    QStandardItem *x = new QStandardItem("Execute");

    r->setCheckable(true);
    w->setCheckable(true);
    x->setCheckable(true);

    r->setEditable(false);
    w->setEditable(false);
    x->setEditable(false);

    r->setCheckState(perm.contains('r')? Qt::Checked : Qt::Unchecked);
    w->setCheckState(perm.contains('w')? Qt::Checked : Qt::Unchecked);
    x->setCheckState(perm.contains('x')? Qt::Checked : Qt::Unchecked);

    items.append(r);
    items.append(w);
    items.append(x);

    model->insertRow(0, items);
}

static void createDefault(QStandardItemModel *model) {
    insertRow(model, QStringList("RAM") << "0x20000000" << "112k", "rwx");
    insertRow(model, QStringList("FLASH") << "0x08000000" << "1M", "rx");
}

static void deserializeModel(QStandardItemModel *m, const QStringList& l) {
    foreach(QString e, l) {
        QRegExp re("([\\w_]+)\\:([\\w_]+)\\:([\\w_]+)\\:(r*w*x*)");
        if (re.exactMatch(e)) {
            QString name = re.cap(1);
            QString orig = re.cap(2);
            QString len = re.cap(3);
            QString access = re.cap(4);
            insertRow(m, QStringList(name) << orig << len, access);
        }
    }
}

ConfigureDialog::ConfigureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureDialog)
{
    ProgrammSettings s;

    ui->setupUi(this);
    QStandardItemModel *model = new QStandardItemModel(this);
    ui->memTable->setModel(model);
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(on_memLayoutViewRefresh_clicked()));
    deserializeModel(model, s.memLayout());
    if (model->rowCount() == 0)
        createDefault(model);
    ui->ldPreview->setFont(QFont("monospace", 8));
    //ui->editCommandBuild->setText(s.buildCommand());
    ui->memLayoutViewRefresh->click();
}

ConfigureDialog::~ConfigureDialog()
{
    delete ui;
}

static QStringList serializeMemLayout(QStandardItemModel *m) {
    QStringList list;
    for(int r=0; r < m->rowCount(); r++)
        list.insert(0, QString("%1:%2:%3:%4%5%6")
                    .arg(m->item(r, 0)->text())
                    .arg(m->item(r, 1)->text())
                    .arg(m->item(r, 2)->text())
                    .arg(m->item(r, 3)->checkState()? "r" : QString())
                    .arg(m->item(r, 4)->checkState()? "w" : QString())
                    .arg(m->item(r, 5)->checkState()? "x" : QString())
        );
    return list;
}

void ConfigureDialog::on_buttonBox_accepted()
{
    ProgrammSettings s;
    //s.setBuildCommand(ui->editCommandBuild->text());
    s.setMemLayout(serializeMemLayout(qobject_cast<QStandardItemModel*>(ui->memTable->model())));
}

void ConfigureDialog::on_memLayoutViewRefresh_clicked()
{
    QString memLayout = "MEMORY\n{\n";
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->memTable->model());
    for(int i=0; i<model->rowCount(); i++) {
        QString name = model->item(i, 0)->text();
        QString addr = model->item(i, 1)->text();
        QString size = model->item(i, 2)->text();
        QString perms = "";
        if (model->item(i, 3)->checkState() == Qt::Checked)
            perms += "r";
        if (model->item(i, 4)->checkState() == Qt::Checked)
            perms += "w";
        if (model->item(i, 5)->checkState() == Qt::Checked)
            perms += "x";
        memLayout.append(QString("  %1 (%4) : ORIGIN = %2, LENGTH = %3\n")
                .arg(name)
                .arg(addr)
                .arg(size)
                .arg(perms));
    }
    memLayout.append("}\n");
    ui->ldPreview->setText(memLayout);
    ui->memTable->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void ConfigureDialog::on_MemLayoutAddEntry_clicked()
{
    QString s = QInputDialog::getText(this, tr("Enter data"), tr("Enter section name"));
    if (!s.isNull()) {
        insertRow(qobject_cast<QStandardItemModel*>(ui->memTable->model()), QStringList(s) << "0x00000000" << "0", "rwx");
        on_memLayoutViewRefresh_clicked();
    }
}

void ConfigureDialog::on_MemLayoutDeleteEntry_clicked()
{
    ui->memTable->model()->removeRow(ui->memTable->selectionModel()->currentIndex().row());
    on_memLayoutViewRefresh_clicked();
}
