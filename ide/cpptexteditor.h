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
#ifndef CPPTEXTEDITOR_H
#define CPPTEXTEDITOR_H

#include "codetexteditor.h"

class ICodeModelProvider;

class CPPTextEditor : public CodeTextEditor
{
public:
    explicit CPPTextEditor(QWidget *parent = nullptr);
    virtual ~CPPTextEditor() override;

    bool load(const QString &path) override;

    static IDocumentEditorCreator *creator();

signals:
    void queryToOpen(const QString& path);

private slots:
    void findReference();
    void formatCode();
    void openIncludeInCursor();

protected:
    QMenu *createContextualMenu() override;
    void triggerAutocompletion() override;
    QsciLexer *lexerFromFile(const QString &name) override;
};

#endif // CPPTEXTEDITOR_H
