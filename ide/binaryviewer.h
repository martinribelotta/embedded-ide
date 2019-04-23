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
#ifndef BINARYVIEWER_H
#define BINARYVIEWER_H

#include <idocumenteditor.h>
#include <QHexView.h>

class BinaryViewer : public IDocumentEditor, public QHexView
{
public:
    explicit BinaryViewer(QWidget *parent = nullptr);
    virtual ~BinaryViewer() override {}

    virtual const QWidget *widget() const override { return this; }
    virtual QWidget *widget() override { return this; }
    virtual bool load(const QString& path) override;
    virtual bool save(const QString& path) override { Q_UNUSED(path); return false; }
    virtual void reload() override { load(path()); }
    virtual QString path() const override { return widget()->windowFilePath(); }
    virtual void setPath(const QString& path) override { widget()->setWindowFilePath(path); }
    virtual bool isReadonly() const override { return true; }
    virtual void setReadonly(bool rdOnly) override { Q_UNUSED(rdOnly); }
    virtual bool isModified() const override { return false; }
    virtual void setModified(bool m) override { Q_UNUSED(m); }
    virtual QPoint cursor() const override;
    virtual void setCursor(const QPoint& pos) override;

    static IDocumentEditorCreator *creator();
};

#endif // BINARYVIEWER_H
