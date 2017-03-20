#include "toolmanager.h"
#include "ui_toolmanager.h"

#include <QSettings>
#include <QStandardItemModel>

extern const QFont monoFont();

ToolManager::ToolManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ToolManager)
{
    ui->setupUi(this);
    model = new QStandardItemModel(this);
    ui->itemsTableView->setModel(model);
}

ToolManager::~ToolManager()
{
    delete ui;
}

static QList<QStandardItem*> makeItem(const QString& name, const QString& command)
{
    QList<QStandardItem*> l{ new QStandardItem(name), new QStandardItem(command) };
    l[1]->setFont(monoFont());
    return l;
}

void ToolManager::setTools(const ProjectView::EntryList_t &toolList)
{
    model->clear();
    model->setHorizontalHeaderLabels(QStringList{ tr("Name"), tr("Command") });
    foreach(auto e, toolList) {
        QString k = e.first;
        QString v = e.second.toString();
        model->appendRow(makeItem(k, v));
    }
    ui->itemsTableView->resizeColumnsToContents();
}

void ToolManager::on_toolButton_add_clicked()
{
    model->appendRow(makeItem("", ""));
}

void ToolManager::on_toolButton_del_clicked()
{
    QModelIndexList m = ui->itemsTableView->selectionModel()->selectedRows();
    while (!m.isEmpty()){
        model->removeRow(m.last().row());
        m.removeLast();
    }
}

void ToolManager::on_ToolManager_accepted()
{
    QSettings sets;
    sets.beginGroup("tools");
    sets.remove(""); // Remove keys in group
    sets.beginWriteArray("external");
    for(int row=0; row<model->rowCount(); row++) {
        sets.setArrayIndex(row);
        QString key = model->item(row, 0)->text();
        QString val = model->item(row, 1)->text();
        sets.setValue("name", key);
        sets.setValue("command", val);
    }
    sets.endArray();
}

void ToolManager::on_toolButton_itemUp_clicked()
{
    QModelIndexList m = ui->itemsTableView->selectionModel()->selectedRows();
    QList<int> selectable;
    while (!m.isEmpty()) {
        QModelIndex e = m.last();
        int toRow = e.row() - 1;
        if (toRow >= 0) {
            QList<QStandardItem*> items = model->takeRow(e.row());
            model->insertRow(toRow, items);
        }
        selectable.append(toRow);
        m.removeLast();
    }
    foreach(int row, selectable)
        ui->itemsTableView->selectRow(row);
}

void ToolManager::on_toolButton_itemDown_clicked()
{
    QModelIndexList m = ui->itemsTableView->selectionModel()->selectedRows();
    QList<int> selectable;
    while (!m.isEmpty()){
        QModelIndex e = m.last();
        int toRow = e.row() + 1;
        if (toRow < model->rowCount()) {
            QList<QStandardItem*> items = model->takeRow(e.row());
            model->insertRow(toRow, items);
        }
        selectable.append(toRow);
        m.removeLast();
    }
    foreach(int row, selectable)
        ui->itemsTableView->selectRow(row);
}
