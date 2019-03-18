#ifndef MAPFILEVIEWER_H
#define MAPFILEVIEWER_H

#include <QTreeView>

#include "idocumenteditor.h"

class MapFileViewer : public QTreeView, public IDocumentEditor
{
public:
    explicit MapFileViewer(QWidget *parent = nullptr);
    virtual ~MapFileViewer() override;

    const QWidget *widget() const override { return this; }
    QWidget *widget() override { return this; }
    bool load(const QString& path) override;
    bool save(const QString& path) override { Q_UNUSED(path); return false; }
    void reload() override { load(path()); }
    bool isReadonly() const override { return true; }
    void setReadonly(bool rdOnly) override { Q_UNUSED(rdOnly); }
    bool isModified() const override { return false; }
    void setModified(bool m) override { Q_UNUSED(m); }
    QPoint cursor() const override { return QPoint(); }
    void setCursor(const QPoint& pos) override { Q_UNUSED(pos); }

    static IDocumentEditorCreator *creator();
};

#endif // MAPFILEVIEWER_H
