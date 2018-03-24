#include "cpptexteditor.h"
#include "icodemodelprovider.h"

#include <Qsci/qscilexercpp.h>
#include <Qsci/qsciabstractapis.h>

#include <QMenu>

#include <QMimeDatabase>
#include <QtDebug>

static const QStringList C_CXX_EXTENSIONS = { "c", "cpp", "h", "hpp", "cc", "hh", "hxx", "cxx", "c++", "h++" };
static const QStringList C_CXX_MIMETYPES = { "text/x-c", "text/x-csrc", "text/x-c++src", "text/x-chdr", "text/x-c++hdr" };

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
    provider = nullptr;
    setAutoCompletionSource(AcsNone);
    connect(this, &CPPTextEditor::userListActivated, [this](int id, const QString& text) {
        Q_UNUSED(id);
        int line, index;
        getCursorPosition(&line, &index);
        if (hasSelectedText()) {
            qDebug() << "replace selection" << selectedText() << "by" << text;
            removeSelectedText();
        }
        insert(text);
        setCursorPosition(line, index + text.length());
    });
}

CPPTextEditor::~CPPTextEditor()
{
}

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

void CPPTextEditor::findDeclaration()
{
    if (provider) {
    }
}

void CPPTextEditor::findDefinition()
{
    if (provider) {
    }
}

QMenu *CPPTextEditor::createContextualMenu()
{
    auto menu = CodeTextEditor::createContextualMenu();
#define _(icon, text, functor, keys) do { \
    auto a = menu->addAction(QIcon(":/images/actions/" icon ".svg"), text, this, &CPPTextEditor::functor); \
    a->setShortcut(QKeySequence(keys)); \
} while(0)
    _("edit-find-replace", tr("Find declaration"), findDeclaration, "CTRL+ENTER");
    _("edit-find-replace", tr("Find definition"), findDefinition, "CTRL+SHIFT+ENTER");
#undef _
    return menu;
}

void CPPTextEditor::triggerAutocompletion()
{
    if (provider) {
        int line, index;
        getCursorPosition(&line, &index);
        provider->completionAt({ path(), line, index }, text(),
                               [this](const QStringList& completions)
        {
            showUserList(1, completions);
        });
    } else {
        CodeTextEditor::triggerAutocompletion();
    }
}

QsciLexer *CPPTextEditor::lexerFromFile(const QString &name)
{
    Q_UNUSED(name);
    return new MyQsciLexerCPP(this);
}
