#ifndef CODETEXTEDITOR_H
#define CODETEXTEDITOR_H

#include "plaintexteditor.h"

class CodeTextEditor : public PlainTextEditor
{
    Q_OBJECT
public:
    explicit CodeTextEditor(QWidget *parent = nullptr);
    virtual ~CodeTextEditor();

    virtual bool load(const QString &path);

    static IDocumentEditorCreator *creator();

protected:

    QMenu *createContextualMenu() override;
    void triggerAutocompletion() override;

    virtual QsciLexer *lexerFromFile(const QString& name);
};

#endif // CODETEXTEDITOR_H
