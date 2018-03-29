#include "cpptexteditor.h"
#include "icodemodelprovider.h"

#include <Qsci/qscilexercpp.h>
#include <Qsci/qsciabstractapis.h>

#include <QMenu>

#include <QMimeDatabase>
#include <QRegularExpression>
#include <QtDebug>

static const QStringList C_CXX_EXTENSIONS = { "c", "cpp", "h", "hpp", "cc", "hh", "hxx", "cxx", "c++", "h++" };
static const QStringList C_MIMETYPE = { "text/x-c++src", "text/x-c++hdr" };
static const QStringList CXX_MIMETYPE = { "text/x-c", "text/x-csrc", "text/x-chdr" };

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
    setProperty("isCXX", false);
    setAutoCompletionSource(AcsNone);
    connect(this, &CPPTextEditor::userListActivated, [this](int id, const QString& text) {
        Q_UNUSED(id);
        auto position = SendScintilla(SCI_GETCURRENTPOS);
        auto start = SendScintilla(SCI_WORDSTARTPOSITION, position, true);
        auto end = SendScintilla(SCI_WORDENDPOSITION, position, true);
        SendScintilla(SCI_SETSELECTIONSTART, start);
        SendScintilla(SCI_SETSELECTIONEND, end);
        SendScintilla(SCI_REPLACESEL, textAsBytes(text).constData());
        SendScintilla(SCI_GOTOPOS, start + text.length());
    });
}

CPPTextEditor::~CPPTextEditor()
{
}

class CPPEditorCreator: public IDocumentEditorCreator
{
public:
    static bool in(const QMimeType& t, const QStringList list) {
        for(const auto& mtype: list)
            if (t.inherits(mtype))
                return true;
        return false;
    }

    bool canHandleExtentions(const QStringList &suffixes) const override {
        for(const auto& suffix: suffixes)
            if (C_CXX_EXTENSIONS.contains(suffix))
                return true;
        return false;
    }

    bool canHandleMime(const QMimeType &mime) const override {
        if (in(mime, C_MIMETYPE))
            return true;
        if (in(mime, CXX_MIMETYPE))
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
    if (codeModel()) {
        codeModel()->declarationOf(wordUnderCursor(), [](const ICodeModelProvider::FileReferenceList& refs)
        {
            Q_UNUSED(refs);
            // TODO
        });
    } else
        qDebug() << "No code model defined";
}

void CPPTextEditor::findDefinition()
{
    if (codeModel()) {
        codeModel()->declarationOf(wordUnderCursor(), [](const ICodeModelProvider::FileReferenceList& refs)
        {
            Q_UNUSED(refs);
            // TODO
        });
    } else
        qDebug() << "No code model defined";
}

QMenu *CPPTextEditor::createContextualMenu()
{
    auto menu = CodeTextEditor::createContextualMenu();
#define _(icon, text, functor, keys) do { \
    auto a = menu->addAction(QIcon(icon), text, this, &CPPTextEditor::functor); \
    a->setShortcut(QKeySequence(keys)); \
} while(0)
    _(":/images/actions/code-context.svg", tr("Find declaration"), findDeclaration, "CTRL+ENTER");
    _(":/images/actions/code-function.svg", tr("Find definition"), findDefinition, "CTRL+SHIFT+ENTER");
#undef _
    return menu;
}

void CPPTextEditor::triggerAutocompletion()
{
    if (codeModel()) {
        int line, index;
        getCursorPosition(&line, &index);
        codeModel()->completionAt({ path(), line, index }, text(),
                                  [this](const QStringList& completions)
        {
            auto w = wordUnderCursor();
            auto filtered = completions.filter(QRegularExpression(QString(R"(^%1)").arg(w)));
            if (!filtered.isEmpty()) {
                showUserList(1, filtered);
            } else // Fallback autocompletion
                CodeTextEditor::triggerAutocompletion();
        });
    } else {
        CodeTextEditor::triggerAutocompletion();
    }
}

QsciLexer *CPPTextEditor::lexerFromFile(const QString &name)
{
    Q_UNUSED(name);
    setProperty("isCXX", CPPEditorCreator::in(QMimeDatabase().mimeTypeForFile(name), CXX_MIMETYPE));
    return new MyQsciLexerCPP(this);
}
