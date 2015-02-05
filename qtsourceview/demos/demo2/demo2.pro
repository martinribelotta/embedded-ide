# shared temp dir for all projects
TMP_DIR		=	../../tmp/
UI_DIR		=	../../tmp/
MOC_DIR		=	../../tmp/
OBJECTS_DIR	=	../../tmp/
DESTDIR		=	../../

QT		=	gui core xml widgets
CONFIG		+=	qt warn_on silent  gui
TEMPLATE	=	app
SOURCES		=	main2.cpp   mainwindow2.cpp
FORMS		=	mainwindow2.ui
HEADERS		=	mainwindow2.h
INCLUDEPATH	=	../../src .
LIBS		=	-L../../ -lqtsourceview
