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
#ifndef BUTTONEDITORITEMDELEGATE_H
#define BUTTONEDITORITEMDELEGATE_H

#include "appconfig.h"

#include <QItemDelegate>
#include <QWidget>
#include <QLineEdit>
#include <QAction>

template<typename Functor>
class ButtonEditorItemDelegate: public QItemDelegate
{
public:
    ButtonEditorItemDelegate(const QString& ict, const Functor& f) : iconToolTip(ict), func(f) {}
    virtual QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        auto w = QItemDelegate::createEditor(parent, option, index);
        QLineEdit* e = qobject_cast<QLineEdit*>(w);
        if (e) {
            auto a = e->addAction(QIcon(AppConfig::resourceImage({ "actions", "document-open" })),
                QLineEdit::TrailingPosition);
            a->setToolTip(iconToolTip);
            connect(a, &QAction::triggered, [index, this]() {
                func(index);
            });
        }
        return w;
    }
private:
    QString iconToolTip;
    Functor func;
};


#endif // BUTTONEDITORITEMDELEGATE_H
