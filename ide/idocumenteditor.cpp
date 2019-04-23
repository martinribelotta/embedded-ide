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
#include "idocumenteditor.h"

#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>

DocumentEditorFactory::DocumentEditorFactory()
= default;

DocumentEditorFactory *DocumentEditorFactory::instance()
{
    static DocumentEditorFactory *staticInstance = nullptr;
    if (!staticInstance)
        staticInstance = new DocumentEditorFactory();
    return staticInstance;
}

void DocumentEditorFactory::registerDocumentInterface(IDocumentEditorCreator *creator)
{
    creators << creator;
}

IDocumentEditor *DocumentEditorFactory::create(const QString &path, QWidget *parent)
{
    QMimeDatabase db;
    auto mime = db.mimeTypeForFile(path);
    auto info = QFileInfo(path);
    auto suffixes = QStringList(info.suffix()) << mime.suffixes();
    if (info.size() == 0) {
        // FIXME: Force the content type of empty files to plain-text
        mime = db.mimeTypeForData(QByteArray{"
"});
    }

    // Try first from suffix
    for(auto c: creators)
        if (c->canHandleExtentions(suffixes))
            return c->create(parent);
    // Try second from mimetype
    for(auto c: creators)
        if (c->canHandleMime(mime))
            return c->create(parent);
    return nullptr;
}

IDocumentEditor::~IDocumentEditor()
= default;

IDocumentEditorCreator::~IDocumentEditorCreator()
= default;
