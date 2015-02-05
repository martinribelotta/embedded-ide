# demo3 - demo3 applicaiont

PROJECT(demo3)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.0)

# input files
SET( demo3_UIS		mainwindow3.ui )
SET( demo3_SRCS		mainwindow3.cpp main3.cpp )
SET( demo3_MOC_HDRS	mainwindow3.h )

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
QT4_WRAP_UI		( demo3_UIS_H		${demo3_UIS}		)
QT4_WRAP_CPP		( demo3_MOC_SRCS	${demo3_MOC_HDRS}	)
QT4_AUTOMOC		( ${demo3_SRCS}					)

# build the file
INCLUDE_DIRECTORIES	( ${QT_INCLUDE_DIR} ${CMAKE_BINARY_DIR} ../../src  )
ADD_EXECUTABLE		( demo3	${demo3_SRCS} ${demo3_UIS_H} ${demo3_MOC_SRCS} ${demo3_UIS} )
TARGET_LINK_LIBRARIES	( demo3 ${QT_LIBRARIES} qtsourceview )
