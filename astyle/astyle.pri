BASE=$$PWD
SOURCES += $$BASE/ASBeautifier.cpp
SOURCES += $$BASE/ASEnhancer.cpp
SOURCES += $$BASE/ASFormatter.cpp
SOURCES += $$BASE/ASLocalizer.cpp
SOURCES += $$BASE/ASResource.cpp
SOURCES += $$BASE/astyle_main.cpp

HEADERS += $$BASE/ASLocalizer.h
HEADERS += $$BASE/astyle.h
HEADERS += $$BASE/astyle_main.h

DEFINES += ASTYLE_LIB
