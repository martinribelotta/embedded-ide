PROJECT(qtsourceview)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
set(QATE_VERSION "0.0.3")
set(QATE_SOVERSION "0")

# $Id$
# definitions for the project
# SET( CMAKE_VERBOSE_MAKEFILE  ON )

# http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_change_the_default_build_mode_and_see_it_reflected_in_the_GUI.3F
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

SET( QT_USE_QTXML TRUE )
SET( QT_USE_QTGUI TRUE )
SET( QT_USE_QTNETWORK TRUE )
FIND_PACKAGE( Qt4 REQUIRED )
INCLUDE(${QT_USE_FILE})

add_definitions(-DCORE_EXPORT=)
# input files
#	highlightersettings.cpp
SET( QATE_SRCS
	context.cpp
	defaultcolors.cpp
	definitiondownloader.cpp
	dynamicrule.cpp
	highlightdefinition.cpp
	highlightdefinitionhandler.cpp
	highlightdefinitionmanager.cpp
	highlightdefinitionmetadata.cpp
	highlighter.cpp
	includerulesinstruction.cpp
	itemdata.cpp
	keywordlist.cpp
	mimedatabase.cpp
	progressdata.cpp
	rule.cpp
	specificrules.cpp
)
SET( QATE_MOC_HDRS
	definitiondownloader.h
	highlightdefinitionmanager.h
	highlighter.h
)
#SET( QATE_RCS		qtsourceview.qrc )

# qt4 definitinos
QT4_AUTOMOC       ( ${QATE_SRCS} )
QT4_WRAP_CPP      ( QATE_MOC_SRCS ${QATE_MOC_HDRS} )
#QT4_ADD_RESOURCES ( QATE_RC_SRCS ${QATE_RCS} )

# build the libraries
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/../../)

INCLUDE_DIRECTORIES( ../ . ${CMAKE_BINARY_DIR} )
ADD_LIBRARY( QATE_STATIC STATIC ${QATE_SRCS} ${QATE_MOC_SRCS}  )
target_link_libraries(QATE_STATIC ${QT_LIBRARIES} )
set_target_properties(QATE_STATIC PROPERTIES OUTPUT_NAME qate)

IF(false)
ADD_LIBRARY( QATE_SHARED SHARED ${QATE_SRCS} ${QATE_MOC_SRCS} ${QATE_RC_SRCS} )
target_link_libraries(QATE_SHARED ${QT_LIBRARIES} )
SET_TARGET_PROPERTIES(
  QATE_SHARED
    PROPERTIES
      VERSION     ${QATE_VERSION}
      SOVERSION   ${QATE_SOVERSION}
)

if(MSVC)
	set_target_properties(QATE_STATIC PROPERTIES OUTPUT_NAME qate_static)
 	set_target_properties(QATE_SHARED PROPERTIES OUTPUT_NAME qate)
else()
	set_target_properties(QATE_STATIC PROPERTIES OUTPUT_NAME qate)
 	set_target_properties(QATE_SHARED PROPERTIES OUTPUT_NAME qate)
endif()
endif()

