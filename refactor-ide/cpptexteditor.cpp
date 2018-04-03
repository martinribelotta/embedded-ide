#include "cpptexteditor.h"
#include "filereferencesdialog.h"
#include "icodemodelprovider.h"

#include <Qsci/qscilexercpp.h>
#include <Qsci/qsciabstractapis.h>

#include <QMenu>

#include <QMimeDatabase>
#include <QRegularExpression>
#include <QShortcut>
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
    connect(new QShortcut(QKeySequence("Ctrl+Return"), this), &QShortcut::activated, this, &CPPTextEditor::findReference);
}

CPPTextEditor::~CPPTextEditor()
{
}

bool CPPTextEditor::load(const QString &path)
{
    if (codeModel())
        codeModel()->startIndexingFile(path);
    return CodeTextEditor::load(path);
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

void CPPTextEditor::findReference()
{
    if (codeModel()) {
        auto word = wordUnderCursor();

        codeModel()->referenceOf(word, [this, word](const ICodeModelProvider::FileReferenceList& refs)
        {
            FileReferencesDialog d(refs, window());
            connect(&d, &FileReferencesDialog::itemClicked, [this](const QString& path, int line) {
                qDebug() << "open" << path << "at" << line;
                documentManager()->openDocumentHere(path, line, 0);
            });
            d.exec();
        });
    } else
        qDebug() << "No code model defined";
}

QMenu *CPPTextEditor::createContextualMenu()
{
    auto menu = CodeTextEditor::createContextualMenu();
    menu->addAction(QIcon(":/images/actions/code-context.svg"),
                    tr("Find Reference"),
                    this, &CPPTextEditor::findReference)
            ->setShortcut(QKeySequence("CTRL+ENTER"));
    return menu;
}

void CPPTextEditor::triggerAutocompletion()
{
    if (codeModel()) {
        int line, index;
        getCursorPosition(&line, &index);
        codeModel()->completionAt({ path(), line, index, QString() }, text(),
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
