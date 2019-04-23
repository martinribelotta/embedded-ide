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
#ifndef FINDLINEEDIT_H
#define FINDLINEEDIT_H

#include <QLineEdit>

class QToolButton;

class FindLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit FindLineEdit(QWidget *parent = nullptr);
    virtual ~FindLineEdit() {}

    void addMenuActions(const QHash<QString, QString>& actionList);

    void setPropertyChecked(const QString &propertyName, bool state);
    bool isPropertyChecked(const char *name) const { return property(name).toBool(); }

signals:
    void menuActionClicked(const QString& prop, bool status);

private:
    QToolButton *optionsButton;
    QHash<QString, QAction*> actionMap;
};

#endif // FINDLINEEDIT_H
