#-------------------------------------------------
#
# Project created by QtCreator 2018-02-24T13:51:54
#
#-------------------------------------------------
DESTDIR  = ../build

QT       += core gui svg xml

CONFIG += qscintilla2

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = embedded_ide
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

include(3rdpart/qhexview.pri)

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        projectmanager.cpp \
        documentmanager.cpp \
        idocumenteditor.cpp \
        plaintexteditor.cpp \
        filesystemmanager.cpp \
        unsavedfilesdialog.cpp \
        processmanager.cpp \
        consoleinterceptor.cpp \
        buildmanager.cpp \
        binaryviewer.cpp \
        codetexteditor.cpp \
        findlineedit.cpp \
        formfindreplace.cpp \
    cpptexteditor.cpp \
    appconfig.cpp \
    configwidget.cpp

HEADERS += \
        mainwindow.h \
        projectmanager.h \
        documentmanager.h \
        idocumenteditor.h \
        plaintexteditor.h \
        filesystemmanager.h \
        unsavedfilesdialog.h \
        processmanager.h \
        consoleinterceptor.h \
        buildmanager.h \
        binaryviewer.h \
        codetexteditor.h \
        findlineedit.h \
        formfindreplace.h \
    cpptexteditor.h \
    appconfig.h \
    configwidget.h

FORMS += \
        mainwindow.ui \
        unsavedfilesdialog.ui \
        formfindreplace.ui \
    configwidget.ui

CONFIG += mobility
MOBILITY = 

RESOURCES += \
    resources/resources.qrc
