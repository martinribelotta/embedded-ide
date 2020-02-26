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
#ifndef MARKDOWNEDITOR_H
#define MARKDOWNEDITOR_H

#include <QWidget>
#include <idocumenteditor.h>

#include <codetexteditor.h>

class MarkdownView;

class MarkdownEditor: public QWidget, public IDocumentEditor
{
    Q_OBJECT
public:
    MarkdownEditor(QWidget *parent = nullptr);

    virtual const QWidget *widget() const override { return this; }
    virtual QWidget *widget() override { return this; }
    virtual bool load(const QString& path) override;
    virtual bool save(const QString& path) override;
    virtual void reload() override {
        editor->reload();
        updateView();
    }
    virtual QString path() const override { return widget()->windowFilePath(); }
    virtual void setPath(const QString& path) override;
    virtual bool isReadonly() const override { return editor->isReadonly(); }
    virtual void setReadonly(bool rdOnly) override { return editor->setReadonly(rdOnly); }
    virtual bool isModified() const override { return editor->isModified(); }
    virtual void setModified(bool m) override { return editor->setModified(m); }
    virtual QPoint cursor() const override { return editor->cursor(); }
    virtual void setCursor(const QPoint& pos) override { editor->setCursor(pos); }

    static IDocumentEditorCreator *creator();

public slots:
    void updateView();

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    MarkdownView *view;
    CodeTextEditor *editor;
    QTimer *renderTimer;
};

#endif // MARKDOWNEDITOR_H
