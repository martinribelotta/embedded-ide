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
#ifndef PLAINTEXTEDITOR_H
#define PLAINTEXTEDITOR_H

#include <idocumenteditor.h>
#include <Qsci/qsciscintilla.h>

class PlainTextEditor : public IDocumentEditor, public QsciScintilla
{
public:
    explicit PlainTextEditor(QWidget *parent = nullptr);
    virtual ~PlainTextEditor() override;

    virtual const QWidget *widget() const override { return this; }
    virtual QWidget *widget() override { return this; }
    virtual bool load(const QString& path) override;
    virtual bool save(const QString& path) override;
    virtual void reload() override;
    virtual bool isReadonly() const override;
    virtual void setReadonly(bool rdOnly) override;
    virtual bool isModified() const override;
    virtual void setModified(bool m) override;
    virtual QPoint cursor() const override;
    virtual void setCursor(const QPoint& pos) override;

    static IDocumentEditorCreator *creator();

    QString wordUnderCursor() const;

    virtual void triggerAutocompletion();

public slots:
    void loadConfigWithStyle(const QString& style, const QFont &editorFont, int tabs, bool tabsToSpace);
    void loadConfig();

private slots:
    void adjustLineNumberMargin();
    int findText(const QString &text, int flags, int start, int *targend);

protected:
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

    bool loadStyle(const QString &xmlStyleFile);
    QStringList allWords();

    virtual QMenu *createContextualMenu();
};

#endif // PLAINTEXTEDITOR_H
