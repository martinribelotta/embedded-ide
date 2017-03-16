#-------------------------------------------------
#
# Project created by QtCreator 2014-06-29T17:36:01
#
#-------------------------------------------------

QT       += core gui widgets concurrent network svg

CONFIG += c++11

DESTDIR = build

TARGET = embedded-ide
TEMPLATE = app

#win32{
#    LIBS += -LC:/OpenSSL-Win32/lib -lubsec
#    INCLUDEPATH += C:/OpenSSL-Win32/include
#}

INCLUDEPATH += QHexEdit

DEFINES += CIAA_IDE

QTSOURCEVIEW_SRC_DIR=qtsourceview/src
include(qtsourceview/src/qsvsh.pri)
include(qgdb/qgdb.pri)

QSCINTILLA_SRC_DIR=qscintilla/
include(qscintilla/qscintilla.pri)
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
    taglist.cpp \
    mapviewer.cpp \
    dialogconfigworkspace.cpp \
    mainmenuwidget.cpp \
    formfindreplace.cpp \
    findlineedit.cpp

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
    debuginterface.h \
    qsvtextoperationswidget.h \
    etags.h \
    projectview.h \
    filedownloader.h \
    version.h \
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
    taglist.h \
    mapviewer.h \
    dialogconfigworkspace.h \
    mainmenuwidget.h \
    formfindreplace.h \
    findlineedit.h

FORMS    += mainwindow.ui \
    editorwidget.ui \
    projectnewdialog.ui \
    configdialog.ui \
    aboutdialog.ui \
    debuginterface.ui \
    replaceform.ui \
    searchform.ui \
    projectview.ui \
    mapviewer.ui \
    dialogconfigworkspace.ui \
    mainmenuwidget.ui \
    formfindreplace.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    templates/generic-make.template \
    templates/sAPI.template \
    project-filters.txt

DISTFILES += \
    reference-code-c.txt \
    style.css \
    QHexEdit/LICENSE \
    QHexEdit/README.md \
    i18n/es.ts \
    i18n/zh.ts

RC_ICONS = images/embedded-ide.ico

TRANSLATIONS = i18n/es.ts \
               i18n/zh.ts
