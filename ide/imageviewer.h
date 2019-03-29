#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include "idocumenteditor.h"

#include <QWidget>

class ImageViewer : public IDocumentEditor, public QWidget
{
public:
    explicit ImageViewer(QWidget *parent = nullptr);
    virtual ~ImageViewer() override {}

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
    virtual QPoint cursor() const override { return QPoint(); }
    virtual void setCursor(const QPoint& pos) override { Q_UNUSED(pos); }

    static IDocumentEditorCreator *creator();
};

#endif // IMAGEVIEWER_H
