#-------------------------------------------------
#
# Project created by QtCreator 2014-06-29T17:36:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DESTDIR = build

TARGET = embedded-ide
TEMPLATE = app

QATE_SRC_DIR=qtsourceview/src
QTSOURCEVIEW_SRC_DIR=qtsourceview/src
include(qtsourceview/src/qsvsh.pri)

SOURCES += main.cpp\
        mainwindow.cpp \
    documentarea.cpp \
    documentview.cpp \
    editorwidget.cpp \
    codeeditor.cpp \
    projectnewdialog.cpp \
    targetupdatediscover.cpp \
    projetfromtemplate.cpp \
    projectexporter.cpp \
    configdialog.cpp

HEADERS  += mainwindow.h \
    documentarea.h \
    documentview.h \
    editorwidget.h \
    codeeditor.h \
    projectnewdialog.h \
    targetupdatediscover.h \
    projetfromtemplate.h \
    projectexporter.h \
    configdialog.h

FORMS    += mainwindow.ui \
    documentview.ui \
    editorwidget.ui \
    projectnewdialog.ui \
    configdialog.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    about.txt

DISTFILES += \
    reference-code-c.txt
