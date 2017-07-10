#-------------------------------------------------
#
# Project created by QtCreator 2014-06-29T17:36:01
#
#-------------------------------------------------

QT       += core gui widgets concurrent network svg xml

CONFIG += c++11

DESTDIR = build

TARGET = embedded-ide
TEMPLATE = app

target.path = /tmp/$${TARGET}/bin
INSTALLS += target

linux:QMAKE_LFLAGS += -fuse-ld=gold

DEFINES += DISABLE_DEBUG_UI

#win32{
#    LIBS += -LC:/OpenSSL-Win32/lib -lubsec
#    INCLUDEPATH += C:/OpenSSL-Win32/include
#}

INCLUDEPATH += QHexEdit

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
    findlineedit.cpp \
    toolmanager.cpp \
    appconfig.cpp \
    passwordpromtdialog.cpp \
    templatedownloader.cpp \
    templatesdownloadselector.cpp \
    filepropertiesdialog.cpp \
    findinfilesdialog.cpp \
    gdbinterface.cpp \
    qtdesigner-gdb/gdbmi.cpp \
    qtdesigner-gdb/elfreader.cpp \
    qtdesigner-gdb/breakhandler.cpp \
    qtdesigner-gdb/breakpoint.cpp \
    gdbwire.c

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
    findlineedit.h \
    toolmanager.h \
    appconfig.h \
    passwordpromtdialog.h \
    templatedownloader.h \
    templatesdownloadselector.h \
    filepropertiesdialog.h \
    findinfilesdialog.h \
    gdbinterface.h \
    qtdesigner-gdb/gdbmi.h \
    qtdesigner-gdb/elfreader.h \
    qtdesigner-gdb/breakhandler.h \
    qtdesigner-gdb/breakpoint.h \
    qtdesigner-gdb/debuggerinternalconstants.h \
    gdbwire.h

FORMS    += mainwindow.ui \
    projectnewdialog.ui \
    configdialog.ui \
    aboutdialog.ui \
    debuginterface.ui \
    projectview.ui \
    mapviewer.ui \
    dialogconfigworkspace.ui \
    mainmenuwidget.ui \
    formfindreplace.ui \
    toolmanager.ui \
    passwordpromtdialog.ui \
    templatesdownloadselector.ui \
    filepropertiesdialog.ui \
    findinfilesdialog.ui

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

#######################################
#i18n
#######################################
### FIXIT:
### LANGUAGES = es zh
### defineReplace(prependAll) {
###  for(a,$$1):result = $$result $$2$${a}$$3
###  return($$result)
### }
### TRANSLATIONS = $$prependAll(LANGUAGES, $$PWD/i18n/, .ts)
### qtPrepareTool(LUPDATE, lupdate)
### qtPrepareTool(LRELEASE, lrelease)
### ts-all.commands = cd $$PWD && $$LUPDATE $$PWD/embedded-ide.pro && $$LRELEASE $$PWD/embedded-ide.pro
### QMAKE_EXTRA_TARGETS ''= ts-all
### # FIXME(denisacostaq@gmail.com): this invoke the build always becuase the qm
### # files are included as resouces, make it depend on "all" target
### # ts-all.deppends = all not work
### PRE_TARGETDEPS += ts-all

TRANSLATIONS = i18n/es.ts \
               i18n/zh.ts

TRANSLATIONS_FILES =
qtPrepareTool(LRELEASE, lrelease)
for(tsfile, TRANSLATIONS) {
    qmfile = $$tsfile #$$shadowed($$tsfile)
    qmfile ~= s,.ts$,.qm,
    qmdir = $$dirname(qmfile)
    !exists($$qmdir) {
        mkpath($$qmdir)|error("Aborting.")
    }
    command = $$LRELEASE -removeidentical $$tsfile -qm $$qmfile
    system($$command)|error("Failed to run: $$command")
    TRANSLATIONS_FILES''= $$qmfile
}
