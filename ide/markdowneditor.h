#ifndef MARKDOWNEDITOR_H
#define MARKDOWNEDITOR_H

#include <QWidget>
#include <idocumenteditor.h>

#include <codetexteditor.h>

class MarkdownView;

class MarkdownEditor: public QWidget, public IDocumentEditor
{
    Q_OBJECT
public:
    MarkdownEditor(QWidget *parent = nullptr);

    virtual const QWidget *widget() const override { return this; }
    virtual QWidget *widget() override { return this; }
    virtual bool load(const QString& path) override;
    virtual bool save(const QString& path) override;
    virtual void reload() override {
        editor->reload();
        updateView();
    }
    virtual QString path() const override { return widget()->windowFilePath(); }
    virtual void setPath(const QString& path) override { widget()->setWindowFilePath(path); }
    virtual bool isReadonly() const override { return editor->isReadonly(); }
    virtual void setReadonly(bool rdOnly) override { return editor->setReadonly(rdOnly); }
    virtual bool isModified() const override { return editor->isModified(); }
    virtual void setModified(bool m) override { return editor->setModified(m); }
    virtual QPoint cursor() const override { return editor->cursor(); }
    virtual void setCursor(const QPoint& pos) override { editor->setCursor(pos); }

    static IDocumentEditorCreator *creator();

public slots:
    void updateView();

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    MarkdownView *view;
    CodeTextEditor *editor;
};

#endif // MARKDOWNEDITOR_H
