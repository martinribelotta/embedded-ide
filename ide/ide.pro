DESTDIR  = ../build

QT += core gui widgets svg xml network concurrent uitools

CONFIG += qscintilla2
CONFIG += c++14

greaterThan(QT_MAJOR_VERSION, 4) {
    greaterThan(QT_MINOR_VERSION, 11) {
        CONFIG += lrelease embed_translations
    }
}

TARGET = embedded-ide
TEMPLATE = app

TRANSLATIONS = translations/es.ts

DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include($$PWD/../mapview/mapview.pri)
include($$PWD/../3rdpart/qhexview/qhexview.pri)
include($$PWD/../3rdpart/astyle/astyle.pri)
include($$PWD/../3rdpart/qdarkstyle/qdarkstype.pri)
include($$PWD/../3rdpart/hoedown/hoedown.pri)

#linux:!android: include(3rdpart/backward/backward.pri)
#include(3rdpart/qt-promise/qt-promise.pri)

INCLUDEPATH += $$PWD/../3rdpart

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    markdowneditor.cpp \
    markdownview.cpp \
        projectmanager.cpp \
        documentmanager.cpp \
        idocumenteditor.cpp \
        plaintexteditor.cpp \
        filesystemmanager.cpp \
    templatefile.cpp \
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
    regexhtmltranslator.cpp \
    imageviewer.cpp

HEADERS += \
    buttoneditoritemdelegate.h \
        mainwindow.h \
    markdowneditor.h \
    markdownview.h \
        projectmanager.h \
        documentmanager.h \
        idocumenteditor.h \
        plaintexteditor.h \
        filesystemmanager.h \
    tar.h \
    templatefile.h \
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
    regexhtmltranslator.h \
    imageviewer.h

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
    resources/fonts.qrc \
    resources/iconactions.qrc \
    resources/mimetypes.qrc \
    resources/resources.qrc \
    resources/styles.qrc

QMAKE_LFLAGS += -lqscintilla2_qt5

win32 {
    QMAKE_CXXFLAGS += -g3
    QMAKE_CFLAGS += -g3
}

unix {
    QMAKE_LFLAGS_RELEASE += -static-libstdc++ -static-libgcc
    QMAKE_LFLAGS_DEBUG += -static-libstdc++ -static-libgcc
    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    target.path = $$PREFIX/bin

    desktopfile.files = skeleton/embedded-ide.desktop
    desktopfile.path = $$PREFIX/share/applications

    iconfiles.files = resources/images/light/embedded-ide.svg resources/light/images/embedded-ide.png
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
    skeleton/embedded-ide.desktop \
    translations/es.ts
