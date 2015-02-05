# demo1 - demo1 applicaiont

PROJECT(demo1)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.0)

# input files
SET( demo1_UIS		 )
SET( demo1_SRCS		main1.cpp )
SET( demo1_MOC_HDRS	 )

# verbose - on
# SET( CMAKE_VERBOSE_MAKEFILE  ON )


# definitions for the project
ADD_DEFINITIONS( -Wall )
SET( QT_USE_QTXML TRUE )
SET( QT_USE_QTGUI TRUE )
FIND_PACKAGE( Qt4 REQUIRED )
LINK_DIRECTORIES( ../../ )
INCLUDE(${QT_USE_FILE})

# qt4 definitinos
QT4_WRAP_UI		( demo1_UIS_H		${demo1_UIS}		)
QT4_WRAP_CPP		( demo1_MOC_SRCS	${demo1_MOC_HDRS}	)
QT4_AUTOMOC		( ${demo1_SRCS}					)

# build the file
INCLUDE_DIRECTORIES	( ${QT_INCLUDE_DIR} ${CMAKE_BINARY_DIR} ../../src  )
ADD_EXECUTABLE		( demo1	${demo1_SRCS} ${demo1_UIS_H} ${demo1_MOC_SRCS} ${demo1_UIS} )
TARGET_LINK_LIBRARIES	( demo1	${QT_LIBRARIES} qsvsh -L../../src/ )
#SET(RUNTIME_OUTPUT_DIRECTORY ../../)
