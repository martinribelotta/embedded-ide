# shared temp dir for all projects
TMP_DIR		=	../../tmp/
UI_DIR		=	../../tmp/
MOC_DIR		=	../../tmp/
OBJECTS_DIR	=	../../tmp/
DESTDIR		=	../../

QT		=	gui core xml widgets
CONFIG		+=	qt warn_on silent
TEMPLATE	=	app
SOURCES		=	main3.cpp   mainwindow3.cpp
FORMS		=	mainwindow3.ui
HEADERS		=	mainwindow3.h
INCLUDEPATH	=	../../src .
LIBS		=	-L../../ -lqtsourceview
