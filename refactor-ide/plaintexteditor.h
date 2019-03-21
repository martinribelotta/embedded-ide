#ifndef PLAINTEXTEDITOR_H
#define PLAINTEXTEDITOR_H

#include <idocumenteditor.h>
#include <Qsci/qsciscintilla.h>

class PlainTextEditor : public IDocumentEditor, public QsciScintilla
{
public:
    explicit PlainTextEditor(QWidget *parent = nullptr);
    virtual ~PlainTextEditor() override;

    virtual const QWidget *widget() const override { return this; }
    virtual QWidget *widget() override { return this; }
    virtual bool load(const QString& path) override;
    virtual bool save(const QString& path) override;
    virtual void reload() override;
    virtual bool isReadonly() const override;
    virtual void setReadonly(bool rdOnly) override;
    virtual bool isModified() const override;
    virtual void setModified(bool m) override;
    virtual QPoint cursor() const override;
    virtual void setCursor(const QPoint& pos) override;

    static IDocumentEditorCreator *creator();

    QString wordUnderCursor() const;

    virtual void triggerAutocompletion();

public slots:
    void loadConfigWithStyle(const QString& style, const QFont &editorFont, int tabs, bool tabSpace);
    void loadConfig();

private slots:
    void adjustLineNumberMargin();
    int findText(const QString &text, int flags, int start, int *targend);

protected:
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

    bool loadStyle(const QString &xmlStyleFile);
    QStringList allWords();

    virtual QMenu *createContextualMenu();
};

#endif // PLAINTEXTEDITOR_H
