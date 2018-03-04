#ifndef BINARYVIEWER_H
#define BINARYVIEWER_H

#include <idocumenteditor.h>
#include <QHexView.h>

class BinaryViewer : public IDocumentEditor, public QHexView
{
public:
    BinaryViewer(QWidget *parent = nullptr);

    virtual const QWidget *widget() const { return this; }
    virtual QWidget *widget() { return this; }
    virtual bool load(const QString& path);
    virtual bool save(const QString& path) { Q_UNUSED(path); return false; }
    virtual QString path() const { return widget()->windowFilePath(); }
    virtual void setPath(const QString& path) { widget()->setWindowFilePath(path); }
    virtual bool isReadonly() const { return true; }
    virtual void setReadonly(bool rdOnly) { Q_UNUSED(rdOnly); }
    virtual bool isModified() const { return false; }
    virtual void setModified(bool m) { Q_UNUSED(m); }
    virtual QPoint cursor() const;
    virtual void setCursor(const QPoint& pos);

    static IDocumentEditorCreator *creator();
};

#endif // BINARYVIEWER_H
