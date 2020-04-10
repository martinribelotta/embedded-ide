/*
 * This file is part of Embedded-IDE
 * 
 * Copyright 2019 Martin Ribelotta <martinribelotta@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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

static const QStringList MAKEFILES_NAME = {
    "makefile",
    "Makefile",
    "GNUMakefile",
};

CodeTextEditor::CodeTextEditor(QWidget *parent) : PlainTextEditor(parent)
{
}

CodeTextEditor::~CodeTextEditor() {}

bool CodeTextEditor::load(const QString &path)
{
    setLexer(lexerFromFile(path));
    auto r = PlainTextEditor::load(path);
    QFileInfo info(path);
    auto name = info.fileName();
    auto suffix = info.suffix();
    if (suffix == "mk" || MAKEFILES_NAME.contains(name)) {
        setTabIndents(false);
        setIndentationsUseTabs(true);
    }
    return r;
}

template<typename T> QsciLexer *helperCreator() { return new T(); }
using creator_t = QsciLexer *(*)();

static const QHash<QString, creator_t> EXTENTION_MAP = {
#if QSCINTILLA_VERSION > 0x020900
    { "json", &helperCreator<QsciLexerJSON> },
   { "md", &helperCreator<QsciLexerMarkdown> },
#endif
   { "sh", &helperCreator<QsciLexerBash> },
   { "diff", &helperCreator<QsciLexerDiff> },
   { "patch", &helperCreator<QsciLexerDiff> },
   { "bat", &helperCreator<QsciLexerBatch> },
   { "coffee", &helperCreator<QsciLexerCoffeeScript> },
   { "litcoffee", &helperCreator<QsciLexerCoffeeScript> },
   { "cs", &helperCreator<QsciLexerCSharp> },
   { "css", &helperCreator<QsciLexerCSS> },
   { "html", &helperCreator<QsciLexerHTML> },
   { "htm", &helperCreator<QsciLexerHTML> },
   { "java", &helperCreator<QsciLexerJava> },
   { "js", &helperCreator<QsciLexerJavaScript> },
   { "lua", &helperCreator<QsciLexerLua> },
   { "mk", &helperCreator<QsciLexerMakefile> },
   { "pas", &helperCreator<QsciLexerPascal> },
   { "ps", &helperCreator<QsciLexerPostScript> },
   { "py", &helperCreator<QsciLexerPython> },
   { "rb", &helperCreator<QsciLexerRuby> },
   { "tcl", &helperCreator<QsciLexerTCL> },
   { "tex", &helperCreator<QsciLexerTeX> },
   { "v", &helperCreator<QsciLexerVerilog> },
   { "vhdl", &helperCreator<QsciLexerVHDL> },
   { "vhd", &helperCreator<QsciLexerVHDL> },
   { "xml", &helperCreator<QsciLexerXML> },
   { "yaml", &helperCreator<QsciLexerYAML> },
   { "yml", &helperCreator<QsciLexerYAML> },
};

static const QHash<QString, creator_t> MIMETYPE_MAP = {
#if QSCINTILLA_VERSION > 0x020900
   { "application/json", &helperCreator<QsciLexerJSON> },
   { "text/markdown", &helperCreator<QsciLexerMarkdown> },
   { "text/x-markdown", &helperCreator<QsciLexerMarkdown> },
#endif
   { "application/x-shellscript", &helperCreator<QsciLexerBash> },
   { "text/x-avs", &helperCreator<QsciLexerAVS> },
   { "text/x-bat", &helperCreator<QsciLexerBatch> },
   { "text/x-cmake", &helperCreator<QsciLexerCMake> },
   { "text/x-coffe", &helperCreator<QsciLexerCoffeeScript> },
   { "text/x-csharp", &helperCreator<QsciLexerCSharp> },
   { "text/x-csrc", &helperCreator<QsciLexerCPP> },
   { "text/x-css", &helperCreator<QsciLexerCSS> },
   { "text/x-diff", &helperCreator<QsciLexerDiff> },
   { "text/x-dlang", &helperCreator<QsciLexerD> },
   { "text/x-fortran", &helperCreator<QsciLexerFortran> },
   { "text/x-html", &helperCreator<QsciLexerHTML> },
   { "text/x-idl", &helperCreator<QsciLexerIDL> },
   { "text/x-java", &helperCreator<QsciLexerJava> },
   { "text/x-javascript", &helperCreator<QsciLexerJavaScript> },
   { "text/x-lua", &helperCreator<QsciLexerLua> },
   { "text/x-makefile", &helperCreator<QsciLexerMakefile> },
   { "text/x-matlab", &helperCreator<QsciLexerMatlab> },
   { "text/x-octave", &helperCreator<QsciLexerOctave> },
   { "text/x-pascal", &helperCreator<QsciLexerPascal> },
   { "text/x-perl", &helperCreator<QsciLexerPerl> },
   { "text/x-po", &helperCreator<QsciLexerPO> },
   { "text/x-pov", &helperCreator<QsciLexerPOV> },
   { "text/x-properties", &helperCreator<QsciLexerProperties> },
   { "text/x-ps", &helperCreator<QsciLexerPostScript> },
   { "text/x-python", &helperCreator<QsciLexerPython> },
   { "text/x-ruby", &helperCreator<QsciLexerRuby> },
   { "text/x-spice", &helperCreator<QsciLexerSpice> },
   { "text/x-sql", &helperCreator<QsciLexerSQL> },
   { "text/x-tcl", &helperCreator<QsciLexerTCL> },
   { "text/x-tex", &helperCreator<QsciLexerTeX> },
   { "text/x-verilog", &helperCreator<QsciLexerVerilog> },
   { "text/x-vhdl", &helperCreator<QsciLexerVHDL> },
   { "text/x-xml", &helperCreator<QsciLexerXML> },
   { "text/x-yaml", &helperCreator<QsciLexerYAML> },
};
#undef _

class CodeEditorCreator: public IDocumentEditorCreator
{
public:
    ~CodeEditorCreator() override = default;
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
