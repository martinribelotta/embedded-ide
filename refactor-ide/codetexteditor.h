#ifndef CODETEXTEDITOR_H
#define CODETEXTEDITOR_H

#include "plaintexteditor.h"

class CodeTextEditor : public PlainTextEditor
{
    Q_OBJECT
public:
    explicit CodeTextEditor(QWidget *parent = nullptr);
    virtual ~CodeTextEditor() override;

    virtual bool load(const QString &path) override;

    static IDocumentEditorCreator *creator();

protected:

    QMenu *createContextualMenu() override;

    virtual QsciLexer *lexerFromFile(const QString& name);
};

#endif // CODETEXTEDITOR_H
