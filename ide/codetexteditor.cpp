#include "appconfig.h"
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
#if QSCINTILLA_VERSION > 0x020900
#include <Qsci/qscilexerjson.h>
#include <Qsci/qscilexermarkdown.h>
#endif
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscilexermakefile.h>
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

CodeTextEditor::CodeTextEditor(QWidget *parent) : PlainTextEditor(parent)
{
}

CodeTextEditor::~CodeTextEditor() = default;

bool CodeTextEditor::load(const QString &path)
{
    setLexer(lexerFromFile(path));
    return PlainTextEditor::load(path);
}

template<typename T> T *helperCreator() { return new T(); }

using creator_t = QsciLexer *(*)();

#define _(mime, type) { mime, reinterpret_cast<creator_t>(&helperCreator<type>) }
static const QHash<QString, creator_t> EXTENTION_MAP = {
#if QSCINTILLA_VERSION > 0x020900
    _("json", QsciLexerJSON),
    _("md", QsciLexerMarkdown),
    _("md", QsciLexerMarkdown),
#endif
    _("sh", QsciLexerBash),
    _("diff", QsciLexerDiff),
    _("patch", QsciLexerDiff),
    _("bat", QsciLexerBatch),
    _("coffee", QsciLexerCoffeeScript),
    _("litcoffee", QsciLexerCoffeeScript),
    _("cs", QsciLexerCSharp),
    _("css", QsciLexerCSS),
    _("html", QsciLexerHTML),
    _("htm", QsciLexerHTML),
    _("java", QsciLexerJava),
    _("js", QsciLexerJavaScript),
    _("lua", QsciLexerLua),
    _("mk", QsciLexerMakefile),
    _("pas", QsciLexerPascal),
    _("ps", QsciLexerPostScript),
    _("py", QsciLexerPython),
    _("rb", QsciLexerRuby),
    _("tcl", QsciLexerTCL),
    _("tex", QsciLexerTeX),
    _("v", QsciLexerVerilog),
    _("vhdl", QsciLexerVHDL),
    _("vhd", QsciLexerVHDL),
    _("xml", QsciLexerXML),
    _("yaml", QsciLexerYAML),
    _("yml", QsciLexerYAML),
};

static const QHash<QString, creator_t> MIMETYPE_MAP = {
#if QSCINTILLA_VERSION > 0x020900
    _("application/json", QsciLexerJSON),
    _("text/markdown", QsciLexerMarkdown),
    _("text/x-markdown", QsciLexerMarkdown),
#endif
    _("application/x-shellscript", QsciLexerBash),
    _("text/x-avs", QsciLexerAVS),
    _("text/x-bat", QsciLexerBatch),
    _("text/x-cmake", QsciLexerCMake),
    _("text/x-coffe", QsciLexerCoffeeScript),
    _("text/x-csharp", QsciLexerCSharp),
    _("text/x-csrc", QsciLexerCPP),
    _("text/x-css", QsciLexerCSS),
    _("text/x-diff", QsciLexerDiff),
    _("text/x-dlang", QsciLexerD),
    _("text/x-fortran", QsciLexerFortran),
    _("text/x-html", QsciLexerHTML),
    _("text/x-idl", QsciLexerIDL),
    _("text/x-java", QsciLexerJava),
    _("text/x-javascript", QsciLexerJavaScript),
    _("text/x-lua", QsciLexerLua),
    _("text/x-makefile", QsciLexerMakefile),
    _("text/x-matlab", QsciLexerMatlab),
    _("text/x-octave", QsciLexerOctave),
    _("text/x-pascal", QsciLexerPascal),
    _("text/x-perl", QsciLexerPerl),
    _("text/x-po", QsciLexerPO),
    _("text/x-pov", QsciLexerPOV),
    _("text/x-properties", QsciLexerProperties),
    _("text/x-ps", QsciLexerPostScript),
    _("text/x-python", QsciLexerPython),
    _("text/x-ruby", QsciLexerRuby),
    _("text/x-spice", QsciLexerSpice),
    _("text/x-sql", QsciLexerSQL),
    _("text/x-tcl", QsciLexerTCL),
    _("text/x-tex", QsciLexerTeX),
    _("text/x-verilog", QsciLexerVerilog),
    _("text/x-vhdl", QsciLexerVHDL),
    _("text/x-xml", QsciLexerXML),
    _("text/x-yaml", QsciLexerYAML),
};
#undef _

class CodeEditorCreator: public IDocumentEditorCreator
{
public:
    ~CodeEditorCreator() override;
    bool canHandleExtentions(const QStringList &suffixes) const override  {
        for (const auto& suffix: suffixes)
            if (EXTENTION_MAP.contains(suffix))
                return true;
        return false;
    }

    bool canHandleMime(const QMimeType &mime) const override {
        return MIMETYPE_MAP.contains(mime.name());
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

QsciLexer *CodeTextEditor::lexerFromFile(const QString& name)
{
    auto suffix = QFileInfo(name).suffix();
    if (EXTENTION_MAP.contains(suffix)) {
        qDebug() << "for" << name << "suffix found as" << suffix;
        return EXTENTION_MAP.value(suffix)();
    }
    auto type = QMimeDatabase().mimeTypeForFile(name);
    auto mimename = type.name();
    if (MIMETYPE_MAP.contains(mimename)) {
        qDebug() << "for" << name << "mime found as" << mimename;
        return MIMETYPE_MAP.value(mimename)();
    }
    for(const auto& mname: type.parentMimeTypes()) {
        if (MIMETYPE_MAP.contains(mname)) {
            qDebug() << "for" << name << "parent mime found as" << mname;
            return MIMETYPE_MAP.value(mname)();
        }
    }
    qDebug() << "No lexer found";
    return nullptr;
}

CodeEditorCreator::~CodeEditorCreator()
= default;
