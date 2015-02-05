TEMPLATE = app
SOURCES += main.cpp \
 samplepanel.cpp \
 lineseditor.cpp \
 transparentwidget.cpp \
 newlineedit.cpp \
 mainwindowimpl.cpp \
 editorconfig.cpp \
 colorsmodel.cpp
HEADERS += samplepanel.h \
 lineseditor.h \
 transparentwidget.h \
 newlineedit.h \
 mainwindowimpl.h \
 editorconfig.h \
 colorsmodel.h
CONFIG += warn_on debug_and_release
FORMS += findwidget.ui mainwindow.ui configdialog.ui
RESOURCES += textlines.qrc
INCLUDEPATH += ../../src src
QT += xml
