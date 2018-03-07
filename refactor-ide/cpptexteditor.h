#ifndef CPPTEXTEDITOR_H
#define CPPTEXTEDITOR_H

#include "codetexteditor.h"

class CPPTextEditor : public CodeTextEditor
{
public:
    explicit CPPTextEditor(QWidget *parent = nullptr);
    virtual ~CPPTextEditor();

    static IDocumentEditorCreator *creator();

protected:
    QMenu *createContextualMenu() override;
    QsciLexer *lexerFromFile(const QString &name) override;
};

#endif // CPPTEXTEDITOR_H
