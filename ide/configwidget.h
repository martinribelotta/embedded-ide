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
#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <QDialog>

namespace Ui {
class ConfigWidget;
}

class ConfigWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigWidget(QWidget *parent = nullptr);
    virtual ~ConfigWidget();

public slots:
    void save();
    void load();

protected:
    void showEvent(QShowEvent *) { load(); }

private:
    Ui::ConfigWidget *ui;
};

#endif // CONFIGWIDGET_H
