#!exists(qtcreator-src.pri){
#	message("Please create a file called qtcreator-src.pri, containing:")
#	message("QTCREATOR_DIR=...")
#	error("configuration failed")
#}
#include(qtcreator-src.pri)

EDITOR_DIR = $$QTCREATOR_DIR/src/plugins/texteditor/generichighlighter
COREPLUGIN_DIR = $$QTCREATOR_DIR/src/plugins/coreplugin

EDITOR_DIR	=	$$QATE_SRC_DIR/qate
QT		+=	widgets xml network concurrent
DEFINES		+=	CORE_EXPORT=Q_DECL_EXPORT QTCONCURRENT_EXPORT=

SOURCES +=  \
	$$EDITOR_DIR/context.cpp \
	$$EDITOR_DIR/definitiondownloader.cpp \
	$$EDITOR_DIR/dynamicrule.cpp \
	$$EDITOR_DIR/highlightdefinition.cpp \
	$$EDITOR_DIR/highlightdefinitionmetadata.cpp \
	$$EDITOR_DIR/highlighter.cpp \
	$$EDITOR_DIR/itemdata.cpp \
	$$EDITOR_DIR/includerulesinstruction.cpp \
	$$EDITOR_DIR/keywordlist.cpp \
	$$EDITOR_DIR/progressdata.cpp \
	$$EDITOR_DIR/rule.cpp \
	$$EDITOR_DIR/specificrules.cpp \
	$$QATE_SRC_DIR/qate/mimedatabase.cpp \
	$$QATE_SRC_DIR/qate/highlightdefinitionhandler.cpp \
	$$QATE_SRC_DIR/qate/highlightdefinitionmanager.cpp \
	$$QATE_SRC_DIR/qate/defaultcolors.cpp \

HEADERS += \
	$$EDITOR_DIR/context.h \
	$$EDITOR_DIR/definitiondownloader.h \
	$$EDITOR_DIR/dynamicrule.h \
	$$EDITOR_DIR/highlighter.h \
	$$EDITOR_DIR/highlightdefinition.h \
	$$EDITOR_DIR/highlightdefinitionmetadata.h \
	$$EDITOR_DIR/includerulesinstruction.h \
	$$EDITOR_DIR/itemdata.h \
	$$EDITOR_DIR/keywordlist.h \
	$$EDITOR_DIR/progressdata.h \
	$$EDITOR_DIR/rule.h \
	$$EDITOR_DIR/specificrules.h \
	$$QATE_SRC_DIR/qate/mimedatabase.h \
	$$QATE_SRC_DIR/qate/highlightdefinitionhandler.h \
	$$QATE_SRC_DIR/qate/highlightdefinitionmanager.h \
	$$QATE_SRC_DIR/qate/syntaxhighlighter.h \
	$$QATE_SRC_DIR/qate/basetextdocumentlayout.h \
	$$QATE_SRC_DIR/qate/tabsettings.h \
	$$QATE_SRC_DIR/qate/defaultcolors.h 

#INCLUDEPATH += $$QATE_SRC_DIR $$QATE_SRC_DIR/qate $$EDITOR_DIR $$QTCREATOR_DIR/src/libs/
INCLUDEPATH += $$QATE_SRC_DIR $$QATE_SRC_DIR/qate 


#	qate/highlightdefinitionhanlder-v2.h \
