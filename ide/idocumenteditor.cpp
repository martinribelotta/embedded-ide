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
        mime = db.mimeTypeForData(QByteArray{"\n"});
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
