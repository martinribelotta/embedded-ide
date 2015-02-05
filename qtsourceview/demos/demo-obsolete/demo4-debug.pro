TEMPLATE = app
SOURCES += main.cpp \
 transparentwidget.cpp \
 mainwindowimpl.cpp \
 editorconfig.cpp \
 colorsmodel.cpp \
 ../../src/qsvcolordef.cpp \
 ../../src/qsvcolordeffactory.cpp \
 ../../src/qsvlangdef.cpp \
 ../../src/qsvlangdeffactory.cpp \
 ../../src/qsvsyntaxhighlighter.cpp \
 qsveditorpanel.cpp \
 qsveditor.cpp \
 qsvprivateblockdata.cpp \
 qsvlineedit.cpp
HEADERS += transparentwidget.h \
 mainwindowimpl.h \
 editorconfig.h \
 colorsmodel.h \
 ../../src/debug_info.h \
 ../../src/qorderedmap.h \
 ../../src/qsvcolordef.h \
 ../../src/qsvcolordeffactory.h \
 ../../src/qsvlangdef.h \
 ../../src/qsvlangdeffactory.h \
 ../../src/qsvsyntaxhighlighter.h \
 qsveditorpanel.h \
 qsveditor.h \
 qsvprivateblockdata.h \
 qsvlineedit.h
CONFIG += warn_on console debug_and_release
FORMS += findwidget.ui \
 mainwindow.ui \
 configdialog.ui \
 filemessage.ui \
 replacewidget.ui \
 gotolinewidget.ui
RESOURCES += textlines.qrc
INCLUDEPATH += ../../src src
LIBS +=
QT += xml
