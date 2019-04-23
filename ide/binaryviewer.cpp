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
#include "binaryviewer.h"

BinaryViewer::BinaryViewer(QWidget *parent) : QHexView(parent)
{
}

bool BinaryViewer::load(const QString &path)
{
    try {
        setData(new DataStorageFile(path));
        setPath(path);
        return true;
    } catch(std::runtime_error) {
        return false;
    }
}

QPoint BinaryViewer::cursor() const
{
    return { cursorPos(), 0 };
}

void BinaryViewer::setCursor(const QPoint &pos)
{
    setCursorPos(pos.x());
}


class BinaryViewerCreator: public IDocumentEditorCreator
{
public:
    ~BinaryViewerCreator() override;
    bool canHandleMime(const QMimeType &mime) const override {
        return mime.inherits("application/octet-stream");
    }

    IDocumentEditor *create(QWidget *parent = nullptr) const override {
        return new BinaryViewer(parent);
    }
};

IDocumentEditorCreator *BinaryViewer::creator()
{
    return IDocumentEditorCreator::staticCreator<BinaryViewerCreator>();
}

BinaryViewerCreator::~BinaryViewerCreator()
= default;
