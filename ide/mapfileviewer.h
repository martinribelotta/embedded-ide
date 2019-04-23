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
#ifndef MAPFILEVIEWER_H
#define MAPFILEVIEWER_H

#include <QTreeView>

#include "idocumenteditor.h"

class MapFileViewer : public QTreeView, public IDocumentEditor
{
public:
    explicit MapFileViewer(QWidget *parent = nullptr);
    virtual ~MapFileViewer() override;

    const QWidget *widget() const override { return this; }
    QWidget *widget() override { return this; }
    bool load(const QString& path) override;
    bool save(const QString& path) override { Q_UNUSED(path); return false; }
    void reload() override { load(path()); }
    bool isReadonly() const override { return true; }
    void setReadonly(bool rdOnly) override { Q_UNUSED(rdOnly); }
    bool isModified() const override { return false; }
    void setModified(bool m) override { Q_UNUSED(m); }
    QPoint cursor() const override { return QPoint(); }
    void setCursor(const QPoint& pos) override { Q_UNUSED(pos); }

    static IDocumentEditorCreator *creator();
};

#endif // MAPFILEVIEWER_H
