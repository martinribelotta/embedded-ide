# shared temp dir for all projects
TMP_DIR		=	../../tmp/
UI_DIR		=	../../tmp/
MOC_DIR		=	../../tmp/
OBJECTS_DIR	=	../../tmp/
DESTDIR		=	../../

# set the Qate source dir, and include the pri file
# then add your sources and that's it
INCLUDEPATH	=	../../src .
LIBS		=	-L../../ -lqtsourceview

QT		=	gui core xml widgets


CONFIG		+=	qt warn_on silent
TARGET		=	demo5
RESOURCES	+=	demo5.qrc
SOURCES		+=	main5.cpp \
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
