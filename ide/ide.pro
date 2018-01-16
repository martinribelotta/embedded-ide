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
    src/main.cpp \
    src/mainwindow.cpp \
    src/documentarea.cpp \
    src/codeeditor.cpp \
    src/projectnewdialog.cpp \
    src/targetupdatediscover.cpp \
    src/projetfromtemplate.cpp \
    src/projectexporter.cpp \
    src/configdialog.cpp \
    src/aboutdialog.cpp \
    src/projecticonprovider.cpp \
    src/makefileinfo.cpp \
    src/clangcodecontext.cpp \
    src/etags.cpp \
    src/projectview.cpp \
    src/filedownloader.cpp \
    src/version.cpp \
    src/loggerwidget.cpp \
    src/taglist.cpp \
    src/mapviewer.cpp \
    src/dialogconfigworkspace.cpp \
    src/mainmenuwidget.cpp \
    src/formfindreplace.cpp \
    src/findlineedit.cpp \
    src/toolmanager.cpp \
    src/appconfig.cpp \
    src/passwordpromtdialog.cpp \
    src/templatedownloader.cpp \
    src/templatesdownloadselector.cpp \
    src/filepropertiesdialog.cpp \
    src/findinfilesdialog.cpp \
    src/debugui.cpp \
    src/gdbstartdialog.cpp \
    src/codetemplate.cpp \
    src/componentsdialog.cpp \
    src/componentitemwidget.cpp \
    src/combodocumentview.cpp

INCLUDEPATH += inc

HEADERS  += \
    inc/mainwindow.h \
    inc/documentarea.h \
    inc/codeeditor.h \
    inc/projectnewdialog.h \
    inc/projetfromtemplate.h \
    inc/projectexporter.h \
    inc/configdialog.h \
    inc/aboutdialog.h \
    inc/projecticonprovider.h \
    inc/makefileinfo.h \
    inc/clangcodecontext.h \
    inc/etags.h \
    inc/projectview.h \
    inc/filedownloader.h \
    inc/version.h \
    inc/targetupdatediscover.h \
    inc/loggerwidget.h \
    inc/taglist.h \
    inc/mapviewer.h \
    inc/dialogconfigworkspace.h \
    inc/mainmenuwidget.h \
    inc/formfindreplace.h \
    inc/findlineedit.h \
    inc/toolmanager.h \
    inc/appconfig.h \
    inc/passwordpromtdialog.h \
    inc/templatedownloader.h \
    inc/templatesdownloadselector.h \
    inc/filepropertiesdialog.h \
    inc/findinfilesdialog.h \
    inc/debugui.h \
    inc/gdbstartdialog.h \
    inc/codetemplate.h \
    inc/componentsdialog.h \
    inc/componentitemwidget.h \
    inc/combodocumentview.h

FORMS += \
    ui/mainwindow.ui \
    ui/projectnewdialog.ui \
    ui/configdialog.ui \
    ui/aboutdialog.ui \
    ui/projectview.ui \
    ui/mapviewer.ui \
    ui/dialogconfigworkspace.ui \
    ui/mainmenuwidget.ui \
    ui/formfindreplace.ui \
    ui/toolmanager.ui \
    ui/passwordpromtdialog.ui \
    ui/templatesdownloadselector.ui \
    ui/filepropertiesdialog.ui \
    ui/findinfilesdialog.ui \
    ui/debugui.ui \
    ui/gdbstartdialog.ui \
    ui/componentsdialog.ui \
    ui/componentitemwidget.ui

RESOURCES += resources/resources.qrc

RC_ICONS = resources/images/embedded-ide.ico

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

    INSTALLS += desktopfile
    INSTALLS += iconfiles
}
