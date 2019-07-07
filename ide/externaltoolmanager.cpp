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
#include "externaltoolmanager.h"
#include "processmanager.h"
#include "buildmanager.h"
#include "ui_externaltoolmanager.h"
#include "projectmanager.h"
#include "buttoneditoritemdelegate.h"

#include <QFileDialog>
#include <QItemDelegate>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QStandardItemModel>

#include <QtDebug>

static QList<QStandardItem*> makeItem(const QString& name=QString(), const QString& command=QString())
{
    QList<QStandardItem*> l{ new QStandardItem(name), new QStandardItem(command) };
    l[1]->setFont(AppConfig::instance().editorFont());
    return l;
}

ExternalToolManager::ExternalToolManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExternalToolManager)
{
    ui->setupUi(this);
#define _(b, name) ui->b->setIcon(QIcon{AppConfig::resourceImage({ "actions", name })})
    _(itemDown, "go-down");
    _(itemUp, "go-up");
    _(itemDel, "list-remove");
    _(itemAdd, "list-add");

    _(pushButton_2, "dialog-close");
    _(pushButton, "dialog-ok-apply");
#undef _
    auto model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({ tr("Description"), tr("Command") });
    ui->tableView->setModel(model);
    auto delegateFunc = [this, model](const QModelIndex& m)
    {
        auto item = model->itemFromIndex(m);
        if (item) {
            auto s = QFileDialog::getOpenFileName(window());
            if (!s.isEmpty())
                item->setText(s);
        } else {
            qDebug() << "no item";
        }
    };
    auto delegate = new ButtonEditorItemDelegate<decltype (delegateFunc)>(tr("Select File"), delegateFunc);
    ui->tableView->setItemDelegateForColumn(1, delegate);
    connect(ui->itemAdd, &QToolButton::clicked, [model]() {
        model->appendRow(makeItem());
    });
    connect(ui->itemDel, &QToolButton::clicked, [model, this]() {
        QModelIndexList m = ui->tableView->selectionModel()->selectedRows();
        while (!m.isEmpty()){
            model->removeRow(m.last().row());
            m.removeLast();
        }
    });
    connect(ui->itemUp, &QToolButton::clicked, [model, this]() {
        QModelIndexList m = ui->tableView->selectionModel()->selectedRows();
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
        for(auto row: selectable)
            ui->tableView->selectRow(row);
    });
    connect(ui->itemDown, &QToolButton::clicked, [model, this]() {
        QModelIndexList m = ui->tableView->selectionModel()->selectedRows();
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
        for(auto row: selectable)
            ui->tableView->selectRow(row);
    });

    auto tools = AppConfig::instance().externalTools();
    for(const auto& it: tools)
        model->appendRow(makeItem(it.first, it.second));
}

ExternalToolManager::~ExternalToolManager()
{
    delete ui;
}

QMenu *ExternalToolManager::makeMenu(QWidget *parent, ProcessManager *pman, ProjectManager *proj)
{
    auto p = pman->processFor(BuildManager::PROCESS_NAME);
    auto m = new QMenu(parent);
    auto t = AppConfig::instance().externalTools();
    for(const auto& it: t) {
        auto cmd = it.second;
        m->addAction(QIcon(AppConfig::resourceImage({ "actions", "run-build" })),
                     it.first, [p, cmd, proj]() {
            p->setWorkingDirectory(proj->projectPath());
            p->start(AppConfig::replaceWithEnv(cmd));
        });
    }
    m->addSeparator();
    m->addAction(QIcon(AppConfig::resourceImage({ "actions", "window-new" })),
                 QObject::tr("Open Tool Manager"), [parent]() {
        ExternalToolManager d(parent);
        if (d.exec()) {
            auto model = qobject_cast<QStandardItemModel*>(d.ui->tableView->model());
            QList<QPair<QString, QString>> map;
            for(int i=0; i<model->rowCount(); i++) {
                auto name = model->item(i, 0)->text();
                auto cmd = model->item(i, 1)->text();
                map.append({ name, cmd });
            }
            AppConfig::instance().setExternalTools(map);
            AppConfig::instance().save();
        }
    });
    return m;
}
