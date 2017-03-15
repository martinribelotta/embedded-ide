# The project file for the QScintilla library.
#
# Copyright (c) 2017 Riverbank Computing Limited <info@riverbankcomputing.com>
# 
# This file is part of QScintilla.
# 
# This file may be used under the terms of the GNU General Public License
# version 3.0 as published by the Free Software Foundation and appearing in
# the file LICENSE included in the packaging of this file.  Please review the
# following information to ensure the GNU General Public License version 3.0
# requirements will be met: http://www.gnu.org/copyleft/gpl.html.
# 
# If you do not wish to use this file under the terms of the GPL version 3.0
# then you may purchase a commercial license.  For more information contact
# info@riverbankcomputing.com.
# 
# This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
# WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.


# This must be kept in sync with Python/configure.py, Python/configure-old.py,
# example-Qt4Qt5/application.pro and designer-Qt4Qt5/designer.pro.
#!win32:VERSION = 13.0.0

#TEMPLATE = lib
#TARGET = qscintilla2_qt$${QT_MAJOR_VERSION}
CONFIG += qt warn_off thread exceptions hide_symbols
INCLUDEPATH += $$QSCINTILLA_SRC_DIR/Qt4Qt5 $$QSCINTILLA_SRC_DIR/include $$QSCINTILLA_SRC_DIR/lexlib $$QSCINTILLA_SRC_DIR/src

!CONFIG(staticlib) {
    DEFINES += QSCINTILLA_MAKE_DLL
}
DEFINES += SCINTILLA_QT SCI_LEXER

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets printsupport

    greaterThan(QT_MINOR_VERSION, 1) {
	    macx:QT += macextras
    }

    # Work around QTBUG-39300.
    CONFIG -= android_install
}

# Comment this in if you want the internal Scintilla classes to be placed in a
# Scintilla namespace rather than pollute the global namespace.
#DEFINES += SCI_NAMESPACE

#target.path = $$[QT_INSTALL_LIBS]
#INSTALLS += target
#
#header.path = $$[QT_INSTALL_HEADERS]
#header.files = Qsci
#INSTALLS += header

#trans.path = $$[QT_INSTALL_TRANSLATIONS]
#trans.files = qscintilla_*.qm
#INSTALLS += trans

#qsci.path = $$[QT_INSTALL_DATA]
#qsci.files = ../qsci
#INSTALLS += qsci

#greaterThan(QT_MAJOR_VERSION, 4) {
#    features.path = $$[QT_HOST_DATA]/mkspecs/features
#} else {
#    features.path = $$[QT_INSTALL_DATA]/mkspecs/features
#}
#CONFIG(staticlib) {
#    features.files = $$PWD/features_staticlib/qscintilla2.prf
#} else {
#    features.files = $$PWD/features/qscintilla2.prf
#}
#INSTALLS += features

HEADERS += \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qsciglobal.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qsciscintilla.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qsciscintillabase.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qsciabstractapis.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qsciapis.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscicommand.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscicommandset.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscidocument.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexer.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexeravs.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerbash.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerbatch.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexercmake.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexercoffeescript.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexercpp.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexercsharp.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexercss.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexercustom.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerd.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerdiff.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerfortran.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerfortran77.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerhtml.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexeridl.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerjava.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerjavascript.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerjson.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerlua.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexermakefile.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexermarkdown.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexermatlab.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexeroctave.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerpascal.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerperl.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerpostscript.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerpo.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerpov.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerproperties.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerpython.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerruby.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerspice.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexersql.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexertcl.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexertex.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerverilog.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexervhdl.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexerxml.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscilexeryaml.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscimacro.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qsciprinter.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscistyle.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/Qsci/qscistyledtext.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/ListBoxQt.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/SciClasses.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/SciNamespace.h \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/ScintillaQt.h \
        $$QSCINTILLA_SRC_DIR/include/ILexer.h \
        $$QSCINTILLA_SRC_DIR/include/Platform.h \
        $$QSCINTILLA_SRC_DIR/include/Sci_Position.h \
        $$QSCINTILLA_SRC_DIR/include/SciLexer.h \
        $$QSCINTILLA_SRC_DIR/include/Scintilla.h \
        $$QSCINTILLA_SRC_DIR/include/ScintillaWidget.h \
        $$QSCINTILLA_SRC_DIR/lexlib/Accessor.h \
        $$QSCINTILLA_SRC_DIR/lexlib/CharacterCategory.h \
        $$QSCINTILLA_SRC_DIR/lexlib/CharacterSet.h \
        $$QSCINTILLA_SRC_DIR/lexlib/LexAccessor.h \
        $$QSCINTILLA_SRC_DIR/lexlib/LexerBase.h \
        $$QSCINTILLA_SRC_DIR/lexlib/LexerModule.h \
        $$QSCINTILLA_SRC_DIR/lexlib/LexerNoExceptions.h \
        $$QSCINTILLA_SRC_DIR/lexlib/LexerSimple.h \
        $$QSCINTILLA_SRC_DIR/lexlib/OptionSet.h \
        $$QSCINTILLA_SRC_DIR/lexlib/PropSetSimple.h \
        $$QSCINTILLA_SRC_DIR/lexlib/StringCopy.h \
        $$QSCINTILLA_SRC_DIR/lexlib/StyleContext.h \
        $$QSCINTILLA_SRC_DIR/lexlib/SubStyles.h \
        $$QSCINTILLA_SRC_DIR/lexlib/WordList.h \
        $$QSCINTILLA_SRC_DIR/src/AutoComplete.h \
        $$QSCINTILLA_SRC_DIR/src/CallTip.h \
        $$QSCINTILLA_SRC_DIR/src/CaseConvert.h \
        $$QSCINTILLA_SRC_DIR/src/CaseFolder.h \
        $$QSCINTILLA_SRC_DIR/src/Catalogue.h \
        $$QSCINTILLA_SRC_DIR/src/CellBuffer.h \
        $$QSCINTILLA_SRC_DIR/src/CharClassify.h \
        $$QSCINTILLA_SRC_DIR/src/ContractionState.h \
        $$QSCINTILLA_SRC_DIR/src/Decoration.h \
        $$QSCINTILLA_SRC_DIR/src/Document.h \
        $$QSCINTILLA_SRC_DIR/src/EditModel.h \
        $$QSCINTILLA_SRC_DIR/src/Editor.h \
        $$QSCINTILLA_SRC_DIR/src/EditView.h \
        $$QSCINTILLA_SRC_DIR/src/ExternalLexer.h \
        $$QSCINTILLA_SRC_DIR/src/FontQuality.h \
        $$QSCINTILLA_SRC_DIR/src/Indicator.h \
        $$QSCINTILLA_SRC_DIR/src/KeyMap.h \
        $$QSCINTILLA_SRC_DIR/src/LineMarker.h \
        $$QSCINTILLA_SRC_DIR/src/MarginView.h \
        $$QSCINTILLA_SRC_DIR/src/Partitioning.h \
        $$QSCINTILLA_SRC_DIR/src/PerLine.h \
        $$QSCINTILLA_SRC_DIR/src/PositionCache.h \
        $$QSCINTILLA_SRC_DIR/src/RESearch.h \
        $$QSCINTILLA_SRC_DIR/src/RunStyles.h \
        $$QSCINTILLA_SRC_DIR/src/ScintillaBase.h \
        $$QSCINTILLA_SRC_DIR/src/Selection.h \
        $$QSCINTILLA_SRC_DIR/src/SplitVector.h \
        $$QSCINTILLA_SRC_DIR/src/Style.h \
        $$QSCINTILLA_SRC_DIR/src/UnicodeFromUTF8.h \
        $$QSCINTILLA_SRC_DIR/src/UniConversion.h \
        $$QSCINTILLA_SRC_DIR/src/ViewStyle.h \
        $$QSCINTILLA_SRC_DIR/src/XPM.h

SOURCES += \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qsciscintilla.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qsciscintillabase.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qsciabstractapis.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qsciapis.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscicommand.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscicommandset.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscidocument.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexer.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexeravs.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerbash.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerbatch.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexercmake.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexercoffeescript.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexercpp.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexercsharp.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexercss.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexercustom.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerd.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerdiff.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerfortran.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerfortran77.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerhtml.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexeridl.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerjava.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerjavascript.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerjson.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerlua.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexermakefile.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexermarkdown.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexermatlab.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexeroctave.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerpascal.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerperl.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerpostscript.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerpo.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerpov.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerproperties.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerpython.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerruby.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerspice.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexersql.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexertcl.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexertex.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerverilog.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexervhdl.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexerxml.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscilexeryaml.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscimacro.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qsciprinter.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscistyle.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscistyledtext.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/MacPasteboardMime.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/InputMethod.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/SciClasses.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/ListBoxQt.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/PlatQt.cpp \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/ScintillaQt.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexA68k.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexAbaqus.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexAda.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexAPDL.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexAsm.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexAsn1.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexASY.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexAU3.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexAVE.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexAVS.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexBaan.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexBash.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexBasic.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexBatch.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexBullant.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexCaml.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexCLW.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexCmake.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexCOBOL.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexCoffeeScript.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexConf.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexCPP.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexCrontab.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexCsound.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexCSS.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexD.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexDiff.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexDMAP.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexDMIS.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexECL.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexEDIFACT.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexEiffel.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexErlang.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexErrorList.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexEScript.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexFlagship.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexForth.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexFortran.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexGAP.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexGui4Cli.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexHaskell.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexHex.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexHTML.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexInno.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexJSON.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexKix.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexKVIrc.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexLisp.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexLout.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexLua.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexMagik.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexMake.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexMarkdown.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexMatlab.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexMetapost.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexMMIXAL.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexModula.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexMPT.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexMSSQL.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexMySQL.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexNimrod.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexNsis.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexNull.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexOpal.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexOScript.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexPascal.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexPB.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexPerl.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexPLM.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexPO.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexPOV.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexPowerPro.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexPowerShell.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexProgress.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexProps.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexPS.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexPython.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexR.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexRebol.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexRegistry.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexRuby.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexRust.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexScriptol.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexSmalltalk.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexSML.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexSorcus.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexSpecman.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexSpice.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexSQL.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexSTTXT.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexTACL.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexTADS3.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexTAL.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexTCL.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexTCMD.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexTeX.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexTxt2tags.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexVB.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexVerilog.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexVHDL.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexVisualProlog.cpp \
        $$QSCINTILLA_SRC_DIR/lexers/LexYAML.cpp \
        $$QSCINTILLA_SRC_DIR/lexlib/Accessor.cpp \
        $$QSCINTILLA_SRC_DIR/lexlib/CharacterCategory.cpp \
        $$QSCINTILLA_SRC_DIR/lexlib/CharacterSet.cpp \
        $$QSCINTILLA_SRC_DIR/lexlib/LexerBase.cpp \
        $$QSCINTILLA_SRC_DIR/lexlib/LexerModule.cpp \
        $$QSCINTILLA_SRC_DIR/lexlib/LexerNoExceptions.cpp \
        $$QSCINTILLA_SRC_DIR/lexlib/LexerSimple.cpp \
        $$QSCINTILLA_SRC_DIR/lexlib/PropSetSimple.cpp \
        $$QSCINTILLA_SRC_DIR/lexlib/StyleContext.cpp \
        $$QSCINTILLA_SRC_DIR/lexlib/WordList.cpp \
        $$QSCINTILLA_SRC_DIR/src/AutoComplete.cpp \
        $$QSCINTILLA_SRC_DIR/src/CallTip.cpp \
        $$QSCINTILLA_SRC_DIR/src/CaseConvert.cpp \
        $$QSCINTILLA_SRC_DIR/src/CaseFolder.cpp \
        $$QSCINTILLA_SRC_DIR/src/Catalogue.cpp \
        $$QSCINTILLA_SRC_DIR/src/CellBuffer.cpp \
        $$QSCINTILLA_SRC_DIR/src/CharClassify.cpp \
        $$QSCINTILLA_SRC_DIR/src/ContractionState.cpp \
        $$QSCINTILLA_SRC_DIR/src/Decoration.cpp \
        $$QSCINTILLA_SRC_DIR/src/Document.cpp \
        $$QSCINTILLA_SRC_DIR/src/EditModel.cpp \
        $$QSCINTILLA_SRC_DIR/src/Editor.cpp \
        $$QSCINTILLA_SRC_DIR/src/EditView.cpp \
        $$QSCINTILLA_SRC_DIR/src/ExternalLexer.cpp \
        $$QSCINTILLA_SRC_DIR/src/Indicator.cpp \
        $$QSCINTILLA_SRC_DIR/src/KeyMap.cpp \
        $$QSCINTILLA_SRC_DIR/src/LineMarker.cpp \
        $$QSCINTILLA_SRC_DIR/src/MarginView.cpp \
        $$QSCINTILLA_SRC_DIR/src/PerLine.cpp \
        $$QSCINTILLA_SRC_DIR/src/PositionCache.cpp \
        $$QSCINTILLA_SRC_DIR/src/RESearch.cpp \
        $$QSCINTILLA_SRC_DIR/src/RunStyles.cpp \
        $$QSCINTILLA_SRC_DIR/src/ScintillaBase.cpp \
        $$QSCINTILLA_SRC_DIR/src/Selection.cpp \
        $$QSCINTILLA_SRC_DIR/src/Style.cpp \
        $$QSCINTILLA_SRC_DIR/src/UniConversion.cpp \
        $$QSCINTILLA_SRC_DIR/src/ViewStyle.cpp \
        $$QSCINTILLA_SRC_DIR/src/XPM.cpp

TRANSLATIONS += \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscintilla_cs.ts \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscintilla_de.ts \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscintilla_es.ts \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscintilla_fr.ts \
        $$QSCINTILLA_SRC_DIR/Qt4Qt5/qscintilla_pt_br.ts
