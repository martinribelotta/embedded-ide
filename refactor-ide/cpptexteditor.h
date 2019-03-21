#ifndef CPPTEXTEDITOR_H
#define CPPTEXTEDITOR_H

#include "codetexteditor.h"

class ICodeModelProvider;

class CPPTextEditor : public CodeTextEditor
{
public:
    explicit CPPTextEditor(QWidget *parent = nullptr);
    virtual ~CPPTextEditor() override;

    bool load(const QString &path) override;

    static IDocumentEditorCreator *creator();

private slots:
    void findReference();
    void formatCode();

protected:
    QMenu *createContextualMenu() override;
    void triggerAutocompletion() override;
    QsciLexer *lexerFromFile(const QString &name) override;
};

#endif // CPPTEXTEDITOR_H
