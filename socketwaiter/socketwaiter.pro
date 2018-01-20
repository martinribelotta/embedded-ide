DESTDIR = ../build

QT += core network
QT -= gui

TARGET = socketwaiter
TEMPLATE = app
INSTALLS += target

SOURCES += \
    main.cpp

unix {
    isEmpty(PREFIX) {
        PREFIX = /usr
    }
    target.path = $$PREFIX/bin
}
