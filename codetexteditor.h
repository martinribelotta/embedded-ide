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

    virtual QMenu *createContextualMenu();
};

#endif // CODETEXTEDITOR_H
