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
#include "unsavedfilesdialog.h"
#include "ui_unsavedfilesdialog.h"
#include "appconfig.h"

#include "filesystemmanager.h"

#include <QFileInfo>
#include <QStandardItemModel>

UnsavedFilesDialog::UnsavedFilesDialog(const QStringList& unsaved, QWidget *parent) :
    QDialog(parent),
    ui(std::make_unique<Ui::UnsavedFilesDialog>())
{
    ui->setupUi(this);
    const struct { QAbstractButton *b; const char *name; } buttonmap[] = {
        { ui->buttonSave, "document-save-all" },
        { ui->buttonDiscart, "edit-delete" },
        { ui->buttonSelectAll, "dialog-ok-apply" },
        { ui->buttonCancel, "dialog-cancel" },
    };
    for (const auto& e: buttonmap)
        e.b->setIcon(QIcon{AppConfig::resourceImage({ "actions", e.name })});

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
    connect(ui->buttonCancel, &QToolButton::clicked, this, &UnsavedFilesDialog::reject);
    connect(ui->buttonSave, &QToolButton::clicked, this, &UnsavedFilesDialog::accept);
    connect(ui->buttonDiscart, &QToolButton::clicked, [setCheckAll, this]() { setCheckAll(false); accept(); });
    connect(ui->buttonSelectAll, &QToolButton::clicked, [setCheckAll]() { setCheckAll(true); });
}

UnsavedFilesDialog::~UnsavedFilesDialog()
{
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
