#ifndef BINARYVIEWER_H
#define BINARYVIEWER_H

#include <idocumenteditor.h>
#include <QHexView.h>

class BinaryViewer : public IDocumentEditor, public QHexView
{
public:
    BinaryViewer(QWidget *parent = nullptr);

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
