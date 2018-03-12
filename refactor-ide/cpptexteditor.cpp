#include "cpptexteditor.h"

#include <Qsci/qscilexercpp.h>

#include <QMenu>

class MyQsciLexerCPP: public QsciLexerCPP {
    mutable QLatin1String keywordList;
public:
    MyQsciLexerCPP(QObject *parent = 0, bool caseInsensitiveKeywords = false) :
        QsciLexerCPP(parent, caseInsensitiveKeywords)
    {
        setFoldCompact(false);
    }

    void refreshProperties()
    {
        QsciLexerCPP::refreshProperties();
        emit propertyChanged("lexer.cpp.track.preprocessor", "0");
    }

    virtual const char *keywords(int set) const
    {
        //if (set == 5) {
        //    updateKeywordList();
        //    return keywordList.data();
        //} else {
        return QsciLexerCPP::keywords(set);
        //}
    }

private:
    void updateKeywordList() const {
        auto c = qobject_cast<CodeTextEditor*>(editor());
        if (c) {
            // keywordList = c->keywordList();
        }
    }
};

CPPTextEditor::CPPTextEditor(QWidget *parent) : CodeTextEditor(parent)
{
}

CPPTextEditor::~CPPTextEditor()
{
}

static const QStringList C_CXX_EXTENSIONS = { "c", "cpp", "h", "hpp", "cc", "hh", "hxx", "cxx", "c++", "h++" };
static const QStringList C_CXX_MIMETYPES = { "text/x-c", "text/x-csrc", "text/x-c++src", "text/x-chdr", "text/x-c++hdr" };

class CPPEditorCreator: public IDocumentEditorCreator
{
public:

    bool canHandleExtentions(const QStringList &suffixes) const override {
        for(const auto& suffix: suffixes)
            if (C_CXX_EXTENSIONS.contains(suffix))
                return true;
        return false;
    }

    bool canHandleMime(const QMimeType &mime) const override {
        for(const auto& mtype: C_CXX_MIMETYPES)
            if (mime.inherits(mtype))
                return true;
        return false;
    }

    IDocumentEditor *create(QWidget *parent = nullptr) const override {
        return new CPPTextEditor(parent);
    }
};

IDocumentEditorCreator *CPPTextEditor::creator()
{
    return IDocumentEditorCreator::staticCreator<CPPEditorCreator>();
}

QMenu *CPPTextEditor::createContextualMenu()
{
    auto menu = CodeTextEditor::createContextualMenu();
    menu->addAction(QIcon(":/images/actions/edit-find-replace.svg"), tr("Find symbol under cursor"), [this]() {});
    return menu;
}

QsciLexer *CPPTextEditor::lexerFromFile(const QString &name)
{
    Q_UNUSED(name);
    return new MyQsciLexerCPP(this);
}
