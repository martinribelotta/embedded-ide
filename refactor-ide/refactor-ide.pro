#-------------------------------------------------
#
# Project created by QtCreator 2018-02-24T13:51:54
#
#-------------------------------------------------
DESTDIR  = ../build

QT       += core gui svg xml network

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

include(../mapview/mapview.pri)
include(3rdpart/qhexview/qhexview.pri)
include(3rdpart/astyle/astyle.pri)
!win32: include(3rdpart/backward/backward.pri)
#include(3rdpart/qt-promise/qt-promise.pri)

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
        configwidget.cpp \
        externaltoolmanager.cpp \
        version.cpp \
        newprojectdialog.cpp \
        findinfilesdialog.cpp \
        icodemodelprovider.cpp \
        templatemanager.cpp \
        templateitemwidget.cpp \
    clangautocompletionprovider.cpp \
    childprocess.cpp \
    filereferencesdialog.cpp \
    mapfileviewer.cpp \
    textmessagebrocker.cpp \
    regexhtmltranslator.cpp

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
        configwidget.h \
        externaltoolmanager.h \
        version.h \
        newprojectdialog.h \
        findinfilesdialog.h \
        icodemodelprovider.h \
        templatemanager.h \
        templateitemwidget.h \
    clangautocompletionprovider.h \
    childprocess.h \
    filereferencesdialog.h \
    mapfileviewer.h \
    textmessagebrocker.h \
    regexhtmltranslator.h

FORMS += \
        mainwindow.ui \
        unsavedfilesdialog.ui \
        formfindreplace.ui \
    configwidget.ui \
    externaltoolmanager.ui \
    newprojectdialog.ui \
    findinfilesdialog.ui \
    templatemanager.ui \
    templateitemwidget.ui \
    filereferencesdialog.ui

CONFIG += mobility
MOBILITY = 

RESOURCES += \
    resources/resources.qrc

unix {
    QMAKE_LFLAGS_RELEASE += -static-libstdc++ -static-libgcc
    QMAKE_LFLAGS_DEBUG += -static-libstdc++ -static-libgcc
    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    target.path = $$PREFIX/bin

    desktopfile.files = skeleton/embedded-ide.desktop
    desktopfile.path = $$PREFIX/share/applications

    iconfiles.files = resources/images/embedded-ide.svg resources/images/embedded-ide.png
    iconfiles.path = $$PREFIX/share/icons/default/256x256/apps/

    scripts.path = $$PREFIX/bin
    scripts.files = skeleton/desktop-integration.sh skeleton/ftdi-tools.sh

    hardconf.path = $$PREFIX/share/embedded-ide
    hardconf.files = skeleton/embedded-ide.hardconf

    INSTALLS += desktopfile
    INSTALLS += iconfiles
    INSTALLS += scripts
    INSTALLS += hardconf
    INSTALLS += target
}

DISTFILES += \
    skeleton/desktop-integration.sh \
    skeleton/embedded-ide.sh \
    skeleton/embedded-ide.sh.wrapper \
    skeleton/ftdi-tools.sh \
    skeleton/embedded-ide.hardconf \
    skeleton/embedded-ide.desktop
