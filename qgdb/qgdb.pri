QT += gui core widgets

SOURCES+=qgdb/core.cpp \
    $$PWD/opendialog.cpp
HEADERS+=qgdb/core.h \
    $$PWD/opendialog.h

SOURCES+=qgdb/com.cpp
HEADERS+=qgdb/com.h

SOURCES+=qgdb/util.cpp
HEADERS+=qgdb/util.h

SOURCES+=qgdb/tree.cpp
HEADERS+=qgdb/tree.h

SOURCES+=qgdb/ini.cpp
HEADERS+=qgdb/ini.h

SOURCES+=qgdb/log.cpp
HEADERS+=qgdb/log.h

SOURCES+=qgdb/settings.cpp
HEADERS+=qgdb/settings.h

HEADERS+=qgdb/config.h
HEADERS+=qgdb/version.h

FORMS += \
    $$PWD/opendialog.ui

DISTFILES += \
    $$PWD/gdb_default_profile.ini
