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
#ifndef FILEREFERENCESDIALOG_H
#define FILEREFERENCESDIALOG_H

#include <QDialog>
#include "icodemodelprovider.h"

#include <memory>

namespace Ui {
class FileReferencesDialog;
}

class FileReferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileReferencesDialog(const ICodeModelProvider::FileReferenceList& refList, QWidget *parent = nullptr);
    ~FileReferencesDialog();

signals:
    void itemClicked(const QString& path, int line);

private:
    std::unique_ptr<Ui::FileReferencesDialog> ui;
};

#endif // FILEREFERENCESDIALOG_H
