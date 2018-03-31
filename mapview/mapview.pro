#-------------------------------------------------
#
# Project created by QtCreator 2018-01-24T21:04:40
#
#-------------------------------------------------
DESTDIR  = ../build
QT       += core gui widgets
TARGET   = mapview
TEMPLATE = app
INSTALLS += target
DEFINES  += QT_DEPRECATED_WARNINGS
SOURCES  += main.cpp mapviewmodel.cpp

unix {
    isEmpty(PREFIX): PREFIX = /usr
    target.path = $$PREFIX/bin
}

FORMS +=

HEADERS += \
    mapviewmodel.h

DISTFILES += \
    mapview.pri
