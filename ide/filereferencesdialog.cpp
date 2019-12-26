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
        auto url = item->data(Qt::UserRole).toUrl();
        auto ref = ICodeModelProvider::FileReference::decode(url);
        emit itemClicked(ref.path, ref.line);
        accept();
    });
    if (!refList.isEmpty()) {
        for(const auto& r: refList) {
            auto url = r.encode();
            auto item = new QListWidgetItem(QString("%1: %2\n%3").arg(r.path).arg(r.line).arg(r.meta));
            item->setData(Qt::UserRole, url);
            ui->listWidget->addItem(item);
        }
        ui->listWidget->setMinimumWidth(ui->listWidget->sizeHintForColumn(0) + 2 + ui->listWidget->frameWidth());
        resize(minimumWidth(), height());
    }
}

FileReferencesDialog::~FileReferencesDialog()
{
    delete ui;
}
