#-------------------------------------------------
#
# Project created by QtCreator 2018-02-24T13:51:54
#
#-------------------------------------------------

QT       += core gui svg xml

CONFIG += qscintilla2

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = embedded_ide-ng
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    appmenu.cpp \
    projectmanager.cpp \
    projecticonprovider.cpp \
    documentmanager.cpp \
    idocumenteditor.cpp \
    plaintexteditor.cpp

HEADERS += \
        mainwindow.h \
    appmenu.h \
    projectmanager.h \
    projecticonprovider.h \
    documentmanager.h \
    idocumenteditor.h \
    plaintexteditor.h

FORMS += \
        mainwindow.ui \
    appmenu.ui

CONFIG += mobility
MOBILITY = 

RESOURCES += \
    resources/resources.qrc
