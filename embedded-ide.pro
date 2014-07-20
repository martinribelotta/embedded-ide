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


SOURCES += main.cpp\
        mainwindow.cpp \
    documentarea.cpp \
    documentview.cpp \
    editorwidget.cpp \
    codeeditor.cpp \
    highlighter.cpp \
    projectnewdialog.cpp \
    targetupdatediscover.cpp \
    projetfromtemplate.cpp \
    projectexporter.cpp

HEADERS  += mainwindow.h \
    documentarea.h \
    documentview.h \
    editorwidget.h \
    codeeditor.h \
    highlighter.h \
    projectnewdialog.h \
    targetupdatediscover.h \
    projetfromtemplate.h \
    projectexporter.h

FORMS    += mainwindow.ui \
    documentview.ui \
    editorwidget.ui \
    projectnewdialog.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    about.txt
