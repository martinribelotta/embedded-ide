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

#win32{
#    LIBS += -LC:/OpenSSL-Win32/lib -lubsec
#    INCLUDEPATH += C:/OpenSSL-Win32/include
#}

INCLUDEPATH += QHexEdit

QTSOURCEVIEW_SRC_DIR=qtsourceview/src
include(qtsourceview/src/qsvsh.pri)
include(qgdb/qgdb.pri)

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
    version.cpp \
    waitingspinnerwidget.cpp \
    QHexEdit/qhexedit.cpp \
    QHexEdit/qhexeditcomments.cpp \
    QHexEdit/qhexeditdata.cpp \
    QHexEdit/qhexeditdatadevice.cpp \
    QHexEdit/qhexeditdatareader.cpp \
    QHexEdit/qhexeditdatawriter.cpp \
    QHexEdit/qhexedithighlighter.cpp \
    QHexEdit/qhexeditprivate.cpp \
    QHexEdit/sparserangemap.cpp \
    loggerwidget.cpp \
    taglist.cpp

HEADERS  += mainwindow.h \
    documentarea.h \
    codeeditor.h \
    projectnewdialog.h \
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
    version.h \
    templates/ciaa-lpcopen.template \
    targetupdatediscover.h \
    waitingspinnerwidget.h \
    QHexEdit/qhexedit.h \
    QHexEdit/qhexeditcomments.h \
    QHexEdit/qhexeditdata.h \
    QHexEdit/qhexeditdatadevice.h \
    QHexEdit/qhexeditdatareader.h \
    QHexEdit/qhexeditdatawriter.h \
    QHexEdit/qhexedithighlighter.h \
    QHexEdit/qhexeditprivate.h \
    QHexEdit/sparserangemap.h \
    loggerwidget.h \
    taglist.h

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
    templates/ciaa-lpcopen.template \
    templates/sAPI.template \
    project-filters.txt

DISTFILES += \
    reference-code-c.txt \
    style.css \
    QHexEdit/LICENSE \
    QHexEdit/README.md

RC_ICONS = icon-theme/icon.ico
