#include "codetexteditor.h"

#include <QFileInfo>
#include <QMenu>
#include <QMimeDatabase>
#include <QtDebug>

#include <Qsci/qscilexeravs.h>
#include <Qsci/qscilexerbash.h>
#include <Qsci/qscilexerbatch.h>
#include <Qsci/qscilexercmake.h>
#include <Qsci/qscilexercoffeescript.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexercsharp.h>
#include <Qsci/qscilexercss.h>
#include <Qsci/qscilexercustom.h>
#include <Qsci/qscilexerd.h>
#include <Qsci/qscilexerdiff.h>
#include <Qsci/qscilexerfortran77.h>
#include <Qsci/qscilexerfortran.h>
#include <Qsci/qscilexer.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexeridl.h>
#include <Qsci/qscilexerjava.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerjson.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscilexermakefile.h>
#include <Qsci/qscilexermarkdown.h>
#include <Qsci/qscilexermatlab.h>
#include <Qsci/qscilexeroctave.h>
#include <Qsci/qscilexerpascal.h>
#include <Qsci/qscilexerperl.h>
#include <Qsci/qscilexerpo.h>
#include <Qsci/qscilexerpostscript.h>
#include <Qsci/qscilexerpov.h>
#include <Qsci/qscilexerproperties.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexerruby.h>
#include <Qsci/qscilexerspice.h>
#include <Qsci/qscilexersql.h>
#include <Qsci/qscilexertcl.h>
#include <Qsci/qscilexertex.h>
#include <Qsci/qscilexerverilog.h>
#include <Qsci/qscilexervhdl.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexeryaml.h>

#if 0
#define DBG qDebug()
#else
#define DBG qDebug()
#endif

class MyQsciLexerCPP: public QsciLexerCPP {
    mutable QLatin1String keywordList;
public:
    MyQsciLexerCPP(QObject *parent = 0, bool caseInsensitiveKeywords = false) :
        QsciLexerCPP(parent, caseInsensitiveKeywords)
    {
        setFoldCompact(false);
    }

    virtual const char *keywords(int set) const
    {
        if (set == 5) {
            updateKeywordList();
            return keywordList.data();
        } else {
            return QsciLexerCPP::keywords(set);
        }
    }

private:
    void updateKeywordList() const {
        auto c = qobject_cast<CodeTextEditor*>(editor());
        if (c) {
            // keywordList = c->keywordList();
        }
    }
};

template<typename T> T *helperCreator() { return new T(); }

typedef QsciLexer* (*creator_t)();

#define _(mime, type) { mime, reinterpret_cast<creator_t>(&helperCreator<type>) }

static QHash<QString, creator_t> creatorMap = {
    _("application/json", QsciLexerJSON),
    _("text/x-octave", QsciLexerOctave),
    _("text/x-fortran", QsciLexerFortran),
    _("text/x-yaml", QsciLexerYAML),
    _("text/x-css", QsciLexerCSS),
    _("text/x-ps", QsciLexerPostScript),
    _("text/x-diff", QsciLexerDiff),
    _("text/x-avs", QsciLexerAVS),
    _("text/x-markdown", QsciLexerMarkdown),
    _("text/markdown", QsciLexerMarkdown),
    _("text/x-makefile", QsciLexerMakefile),
    _("text/x-pov", QsciLexerPOV),
    _("text/x-sql", QsciLexerSQL),
    _("text/x-html", QsciLexerHTML),
    _("text/x-po", QsciLexerPO),
    _("text/x-python", QsciLexerPython),
    _("text/x-lua", QsciLexerLua),
    _("text/x-xml", QsciLexerXML),
    _("text/x-idl", QsciLexerIDL),
    _("text/x-fortran", QsciLexerFortran),
    _("text/x-ruby", QsciLexerRuby),
    _("text/x-tex", QsciLexerTeX),
    _("text/x-bat", QsciLexerBatch),
    _("application/x-shellscript", QsciLexerBash),
    _("text/x-perl", QsciLexerPerl),
    _("text/x-cmake", QsciLexerCMake),
    _("text/x-java", QsciLexerJava),
    _("text/x-csharp", QsciLexerCSharp),
    _("text/x-properties", QsciLexerProperties),
    _("text/x-vhdl", QsciLexerVHDL),
    _("text/x-csrc", MyQsciLexerCPP),
    _("text/x-pascal", QsciLexerPascal),
    _("text/x-spice", QsciLexerSpice),
    _("text/x-matlab", QsciLexerMatlab),
    _("text/x-tcl", QsciLexerTCL),
    _("text/x-verilog", QsciLexerVerilog),
    _("text/x-javascript", QsciLexerJavaScript),
    _("text/x-dlang", QsciLexerD),
    _("text/x-coffe", QsciLexerCoffeeScript)
};

static QHash<QString, creator_t> creatorFromExtMap = {
    _("c", MyQsciLexerCPP),
    _("cc", MyQsciLexerCPP),
    _("cxx", MyQsciLexerCPP),
    _("cpp", MyQsciLexerCPP),
    _("c++", MyQsciLexerCPP),
    _("h", MyQsciLexerCPP),
    _("hh", MyQsciLexerCPP),
    _("hxx", MyQsciLexerCPP),
    _("hpp", MyQsciLexerCPP),
    _("h++", MyQsciLexerCPP),
#if 0
    _("s", SciLexerASM),
    _("S", SciLexerASM),
#endif
};
#undef _

static QsciLexer *lexerFromFile(const QString& name) {
    auto type = QMimeDatabase().mimeTypeForFile(name);
    auto mimename = type.name();
    if (creatorMap.contains(mimename)) {
        qDebug() << "for" << name << "mime found as" << mimename;
        return creatorMap.value(mimename)();
    }
    for(const auto& mname: type.parentMimeTypes()) {
        if (creatorMap.contains(mname)) {
            qDebug() << "for" << name << "parent mime found as" << mname;
            return creatorMap.value(mname)();
        }
    }
    auto suffix = QFileInfo(name).suffix();
    for(const auto& ext: creatorFromExtMap.keys())
        if (suffix == ext) {
            qDebug() << "Lexer found for suffix" << suffix;
            return creatorFromExtMap.value(ext)();
        }
    qDebug() << "No lexer found";
    return nullptr;
}

CodeTextEditor::CodeTextEditor(QWidget *parent) : PlainTextEditor(parent)
{
}

CodeTextEditor::~CodeTextEditor()
{
}

bool CodeTextEditor::load(const QString &path)
{
    if (!PlainTextEditor::load(path))
        return false;
    setLexer(lexerFromFile(path));
    loadConfig();
    return true;
}

class CodeEditorCreator: public IDocumentEditorCreator
{
public:
    bool canHandleExtentions(const QStringList &suffixes) const override {
        for(const auto& suffix: suffixes)
            if (creatorFromExtMap.contains(suffix))
                return true;
        return false;
    }

    bool canHandleMime(const QMimeType &mime) const override {
        return creatorMap.contains(mime.name());
    }

    IDocumentEditor *create(QWidget *parent = nullptr) const override {
        return new CodeTextEditor(parent);
    }
};

IDocumentEditorCreator *CodeTextEditor::creator()
{
    return IDocumentEditorCreator::staticCreator<CodeEditorCreator>();
}

QMenu *CodeTextEditor::createContextualMenu()
{
    QMenu *menu = PlainTextEditor::createContextualMenu();
    return menu;
}
