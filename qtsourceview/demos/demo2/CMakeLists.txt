# demo2 - demo2 applicaiont

PROJECT(demo2)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.0)

# input files
SET( demo2_UIS		mainwindow2.ui )
SET( demo2_SRCS		mainwindow2.cpp main2.cpp )
SET( demo2_MOC_HDRS	mainwindow2.h )

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
QT4_WRAP_UI		( demo2_UIS_H		${demo2_UIS}		)
QT4_WRAP_CPP		( demo2_MOC_SRCS	${demo2_MOC_HDRS}	)
QT4_AUTOMOC		( ${demo2_SRCS}					)

# build the file
INCLUDE_DIRECTORIES	( ${QT_INCLUDE_DIR} ${CMAKE_BINARY_DIR} ../../src  )
ADD_EXECUTABLE		( demo2	${demo2_SRCS} ${demo2_UIS_H} ${demo2_MOC_SRCS} ${demo2_UIS} )
TARGET_LINK_LIBRARIES	( demo2	${QT_LIBRARIES} qtsourceview )
