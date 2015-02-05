# shared temp dir for all projects
TMP_DIR		=	../../tmp/
UI_DIR		=	../../tmp/
MOC_DIR		=	../../tmp/
OBJECTS_DIR	=	../../tmp/
DESTDIR		=	../../

QT		=	gui core xml
CONFIG		+=	qt warn_on silent
TEMPLATE	=	app

SOURCES = main4.cpp qsvtextedit.cpp qsvsyntaxhighlighterbase.cpp qsvtextoperationswidget.cpp 
HEADERS += qsvtextedit.h qsvsyntaxhighlighterbase.h qsvtextoperationswidget.h
FORMS += searchform.ui replaceform.ui bannermessage.ui
RESOURCES += demo4.qrc
OTHER_FILES += readme.txt
