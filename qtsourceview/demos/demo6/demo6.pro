# shared temp dir for all projects
TMP_DIR		=	../../tmp/
UI_DIR		=	../../tmp/
MOC_DIR		=	../../tmp/
OBJECTS_DIR	=	../../tmp/
DESTDIR		=	../../

DEFINES         +=      CORE_EXPORT=Q_DECL_EXPORT
LIBS		=	-L../../ -lqate
INCLUDEPATH	=	../../src ../../src/qate
TARGET		=	demo6-qate
QT		+=	gui core xml  network


CONFIG		+=	qt warn_on silent
TARGET		=	demo6
SOURCES		+=	main6.cpp \
    ../demo4/qsvtextedit.cpp \
    ../demo4/qsvsyntaxhighlighterbase.cpp \
    ../demo4/qsvtextoperationswidget.cpp

HEADERS += ../demo4/qsvtextedit.h \
    ../demo4/qsvsyntaxhighlighterbase.h \
    ../demo4/qsvtextoperationswidget.h
FORMS += ../demo4/searchform.ui \
    ../demo4/replaceform.ui \
    ../demo4/bannermessage.ui
INCLUDEPATH+=../demo4/
