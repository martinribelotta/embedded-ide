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

#include <QMenu>
#include <QStandardItemModel>

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
    auto model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({ tr("Description"), tr("Command") });
    ui->tableView->setModel(model);
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
    for(auto it=tools.begin(); it!=tools.end(); ++it)
        model->appendRow(makeItem(it.key(), it.value()));
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
    for(auto it = t.begin(); it != t.end(); ++it) {
        auto cmd = it.value();
        m->addAction(QIcon(":/images/actions/run-build.svg"), it.key(), [p, cmd, proj]() {
            p->setWorkingDirectory(proj->projectPath());
            p->start(AppConfig::replaceWithEnv(cmd));
        });
    }
    m->addSeparator();
    m->addAction(QIcon(":/images/actions/window-new.svg"), QObject::tr("Open Tool Manager"), [parent]() {
        ExternalToolManager d(parent);
        if (d.exec()) {
            auto model = qobject_cast<QStandardItemModel*>(d.ui->tableView->model());
            QHash<QString, QString> map;
            for(int i=0; i<model->rowCount(); i++) {
                auto name = model->item(i, 0)->text();
                auto cmd = model->item(i, 1)->text();
                map.insert(name, cmd);
            }
            AppConfig::instance().setExternalTools(map);
            AppConfig::instance().save();
        }
    });
    return m;
}
