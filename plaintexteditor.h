#ifndef PLAINTEXTEDITOR_H
#define PLAINTEXTEDITOR_H

#include <idocumenteditor.h>
#include <Qsci/qsciscintilla.h>

class PlainTextEditor : public IDocumentEditor, public QsciScintilla
{
public:
    PlainTextEditor(QWidget *parent = nullptr);

    virtual QWidget *widget() { return this; }
    virtual bool load(const QString& path) override;
    virtual bool save(const QString& path) override;
    virtual QString path() const override;
    virtual bool isReadonly() const override;
    virtual void setReadonly(bool rdOnly) override;
    virtual bool isModified() const override;
    virtual void setModified(bool m) override;
    virtual QPoint cursor() const override;
    virtual void setCursor(const QPoint& pos) override;

    static IDocumentEditorCreator *creator();

private slots:
    void adjustLineNumberMargin();
    int findText(const QString &text, int flags, int start, int *targend);

private:
    void loadConfig();
    bool loadStyle(const QString &xmlStyleFile);
};

#endif // PLAINTEXTEDITOR_H
