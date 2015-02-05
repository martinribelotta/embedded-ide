# shared temp dir for all projects
TMP_DIR		=	../tmp/
UI_DIR		=	../tmp/
MOC_DIR		=	../tmp/
OBJECTS_DIR	=	../tmp/
DESTDIR		=	../

QTSOURCEVIEW_SRC_DIR = .
include(qsvsh.pri)
TARGET		=	qtsourceview
TEMPLATE	=	lib
CONFIG		+=	static lib warn_on silent
VERSION		=	0.0.3
