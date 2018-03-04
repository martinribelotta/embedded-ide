#include "binaryviewer.h"

BinaryViewer::BinaryViewer(QWidget *parent) : QHexView(parent)
{
}

bool BinaryViewer::load(const QString &path)
{
    try {
        setData(new DataStorageFile(path));
        setWindowFilePath(path);
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
    bool canHandleMime(const QMimeType &mime) const override {
        return mime.inherits("application/octet-stream");
    }

    IDocumentEditor *create(QWidget *parent = nullptr) const override {
        return new BinaryViewer(parent);
    }
};

IDocumentEditorCreator *BinaryViewer::creator()
{
    IDocumentEditorCreator *staticCreator = nullptr;
    if (!staticCreator)
        staticCreator = new BinaryViewerCreator();
    return staticCreator;
}
