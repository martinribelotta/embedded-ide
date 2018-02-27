#include "idocumenteditor.h"

#include <QFile>
#include <QMimeDatabase>

DocumentEditorFactory::DocumentEditorFactory()
{
}

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
    auto mime = QMimeDatabase().mimeTypeForFile(path);
    for(auto c: creators)
        if (c->canHandleMime(mime))
            return c->create(parent);
    return nullptr;
}
