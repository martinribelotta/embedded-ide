#-------------------------------------------------
#
# Project created by QtCreator 2014-06-29T17:36:01
#
#-------------------------------------------------

QT       += core gui widgets concurrent network

CONFIG += c++11

DESTDIR = build

TARGET = embedded-ide
TEMPLATE = app

win32{
    LIBS += -LC:/OpenSSL-Win32/lib -lubsec
    INCLUDEPATH += C:/OpenSSL-Win32/include
}

QTSOURCEVIEW_SRC_DIR=qtsourceview/src
include(qtsourceview/src/qsvsh.pri)
#include(qgdb/qgdb.pri)

#QATE_SRC_DIR=qtsourceview/src
#include(qtsourceview/src/qate.pri)

SOURCES += main.cpp\
        mainwindow.cpp \
    documentarea.cpp \
    codeeditor.cpp \
    projectnewdialog.cpp \
    targetupdatediscover.cpp \
    projetfromtemplate.cpp \
    projectexporter.cpp \
    configdialog.cpp \
    aboutdialog.cpp \
    projecticonprovider.cpp \
    makefileinfo.cpp \
    clangcodecontext.cpp \
    debugmanager.cpp \
    debuginterface.cpp \
    qsvtextoperationswidget.cpp \
    etags.cpp \
    projectview.cpp \
    filedownloader.cpp \
    version.cpp

HEADERS  += mainwindow.h \
    documentarea.h \
    codeeditor.h \
    projectnewdialog.h \
    targetupdatediscover.h \
    projetfromtemplate.h \
    projectexporter.h \
    configdialog.h \
    aboutdialog.h \
    projecticonprovider.h \
    makefileinfo.h \
    clangcodecontext.h \
    debugmanager.h \
    debuginterface.h \
    qsvtextoperationswidget.h \
    etags.h \
    projectview.h \
    filedownloader.h \
    version.h

FORMS    += mainwindow.ui \
    editorwidget.ui \
    projectnewdialog.ui \
    configdialog.ui \
    aboutdialog.ui \
    debuginterface.ui \
    replaceform.ui \
    searchform.ui \
    projectview.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    templates/generic-make.template \
    templates/sAPI.template \
    project-filters.txt

DISTFILES += \
    reference-code-c.txt

RC_ICONS = icon-theme/icon.ico
