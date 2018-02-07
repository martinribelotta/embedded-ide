#-------------------------------------------------
#
# Project created by QtCreator 2014-06-29T17:36:01
#
#-------------------------------------------------

QT       += core gui widgets concurrent network svg xml

CONFIG += c++11

DESTDIR = ../build

TARGET = embedded-ide
TEMPLATE = app

target.path = /tmp/$${TARGET}/bin
INSTALLS += target

#linux:QMAKE_LFLAGS += -fuse-ld=gold -fno-lto
#linux:QMAKE_CXXFLAGS += -fsanitize=address -fno-lto
#linux:LIBS += -lasan -lubsan

#win32{
#    LIBS += -LC:/OpenSSL-Win32/lib -lubsec
#    INCLUDEPATH += C:/OpenSSL-Win32/include
#}

QSCINTILLA_SRC_DIR=3rdpart/qscintilla/
include(3rdpart/qscintilla/qscintilla.pri)
include(3rdpart/astyle/astyle.pri)
include(3rdpart/gdbdebugger/gdbdebugger.pri)
include(3rdpart/qtc_gdbmi/qtc_gdbmi.pri)
include(3rdpart/QHexEdit/qhexedit.pri)

SOURCES += \
    main.cpp \
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
    etags.cpp \
    projectview.cpp \
    filedownloader.cpp \
    version.cpp \
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
    debugui.cpp \
    gdbstartdialog.cpp \
    codetemplate.cpp \
    componentsdialog.cpp \
    componentitemwidget.cpp \
    combodocumentview.cpp \
    bannerwidget.cpp \
    clangdprovider.cpp

INCLUDEPATH += inc

HEADERS  += \
    mainwindow.h \
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
    etags.h \
    projectview.h \
    filedownloader.h \
    version.h \
    targetupdatediscover.h \
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
    debugui.h \
    gdbstartdialog.h \
    codetemplate.h \
    componentsdialog.h \
    componentitemwidget.h \
    combodocumentview.h \
    bannerwidget.h \
    clangdprovider.h

FORMS += \
    mainwindow.ui \
    projectnewdialog.ui \
    configdialog.ui \
    aboutdialog.ui \
    projectview.ui \
    mapviewer.ui \
    dialogconfigworkspace.ui \
    mainmenuwidget.ui \
    formfindreplace.ui \
    toolmanager.ui \
    passwordpromtdialog.ui \
    templatesdownloadselector.ui \
    filepropertiesdialog.ui \
    findinfilesdialog.ui \
    debugui.ui \
    gdbstartdialog.ui \
    componentsdialog.ui \
    componentitemwidget.ui \
    bannerwidget.ui

RESOURCES += resources/resources.qrc

RC_ICONS = resources/images/embedded-ide.ico

#######################################
#i18n
#######################################
qtPrepareTool(LUPDATE, lupdate)
qtPrepareTool(LRELEASE, lrelease)

### FIXIT:
### LANGUAGES = es zh
### defineReplace(prependAll) {
###  for(a,$$1):result = $$result $$2$${a}$$3
###  return($$result)
### }
### TRANSLATIONS = $$prependAll(LANGUAGES, $$PWD/i18n/, .ts)
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

unix {
    QMAKE_LFLAGS_RELEASE += -static-libstdc++ -static-libgcc
    QMAKE_LFLAGS_DEBUG += -static-libstdc++ -static-libgcc
    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    target.path = $$PREFIX/bin

    desktopfile.files = embedded-ide.desktop
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
}

DISTFILES += \
    i18n/zh.ts \
    i18n/es.ts
