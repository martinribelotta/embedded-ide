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
#ifndef FINDINFILESDIALOG_H
#define FINDINFILESDIALOG_H

#include <QDialog>
#include <QFuture>
#include <QEvent>

class ProjectView;
class DocumentArea;

namespace Ui {
class FindInFilesDialog;
}

class QStandardItemModel;
class QStandardItem;

class FindInFilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindInFilesDialog(const QString& path, QWidget *parent = nullptr);
    virtual ~FindInFilesDialog() override;

signals:
    void queryToOpen(const QString& path, int line, int column);

protected:
    virtual void closeEvent(QCloseEvent *) override;
private:
    Ui::FindInFilesDialog *ui;
};

#endif // FINDINFILESDIALOG_H
