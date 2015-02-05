# shared temp dir for all projects
TMP_DIR		=	../../tmp/
UI_DIR		=	../../tmp/
MOC_DIR		=	../../tmp/
OBJECTS_DIR	=	../../tmp/
DESTDIR		=	../../

QT		=	gui core xml widgets
CONFIG		+=	qt warn_on silent
TEMPLATE	=	app
SOURCES		=	main1.cpp  
INCLUDEPATH	=	../../src .
LIBS		=	-L../../ -lqtsourceview
