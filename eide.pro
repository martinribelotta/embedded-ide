#-------------------------------------------------
#
# Project created by QtCreator 2014-05-11T19:01:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = eide
TEMPLATE = app

CONFIG += qscintilla2

LIBS += -lqscintilla2

SOURCES += main.cpp\
        mainwindow.cpp \
    codeeditor.cpp \
    documentmanager.cpp \
    configuredialog.cpp \
    buildmanager.cpp \
    buildconfig.cpp

HEADERS  += mainwindow.h \
    codeeditor.h \
    documentmanager.h \
    configuredialog.h \
    buildmanager.h \
    buildconfig.h \
    programmsettings.h

FORMS    += mainwindow.ui \
    configuredialog.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    Makefile.template \
    keywords \
    prepro
