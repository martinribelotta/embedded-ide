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
#include "mapfileviewer.h"

#include <QHeaderView>
#include <mapviewmodel.h>

MapFileViewer::MapFileViewer(QWidget *parent) : QTreeView(parent)
{
    setAlternatingRowColors(true);
    setEditTriggers(QTreeView::NoEditTriggers);
    setItemDelegateForColumn(MapViewModel::PERCENT_COLUMN, new BarItemDelegate(this));
    auto adjust = [this](const QModelIndex&) { header()->resizeSections(QHeaderView::ResizeToContents); };
    QObject::connect(this, &QTreeView::expanded, adjust);
    QObject::connect(this, &QTreeView::collapsed, adjust);
}

MapFileViewer::~MapFileViewer()
= default;

bool MapFileViewer::load(const QString &path)
{
    auto model = new MapViewModel(this);
    setModel(model);
    if (!model->load(path))
        return false;
    header()->resizeSections(QHeaderView::ResizeToContents);
    setPath(path);
    return true;
}

class MAPEditorCreator: public IDocumentEditorCreator
{
public:
    ~MAPEditorCreator() override;
    bool canHandleExtentions(const QStringList &suffixes) const override {
        for(const QString& suffix: suffixes)
            if (suffix.compare("map", Qt::CaseInsensitive) == 0)
                return true;
        return false;
    }

    IDocumentEditor *create(QWidget *parent = nullptr) const override {
        return new MapFileViewer(parent);
    }
};

IDocumentEditorCreator *MapFileViewer::creator()
{
    return IDocumentEditorCreator::staticCreator<MAPEditorCreator>();
}

MAPEditorCreator::~MAPEditorCreator()
= default;
