PROJECT(qtsourceview-demos)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
set(QATE_VERSION "0.0.3")
set(QATE_SOVERSION "0")

SET( QT_USE_QTXML TRUE )
SET( QT_USE_QTGUI TRUE )
SET( QT_USE_QTNETWORK TRUE )
FIND_PACKAGE( Qt4 REQUIRED )
INCLUDE(${QT_USE_FILE})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../")

add_definitions(-DCORE_EXPORT=)
INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/../src/ .
	${CMAKE_CURRENT_SOURCE_DIR}/../src/qate
	${CMAKE_CURRENT_SOURCE_DIR}/demo4
	${CMAKE_CURRENT_BINARY_DIR}
)

# demo 1 - a simple qate window
ADD_EXECUTABLE(DEMO_QATE demo-qate/demo-qate.cpp)
TARGET_LINK_LIBRARIES(DEMO_QATE QATE_STATIC ${QT_LIBRARIES} )
SET_TARGET_PROPERTIES(DEMO_QATE PROPERTIES OUTPUT_NAME qate-demo)

# demo 2 - a main window using qate
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/demo-qate2/demo-qate2.h  ${CMAKE_CURRENT_BINARY_DIR}/demo-qate2.h COPYONLY)
QT4_WRAP_CPP(DEMO_QATE2_MOC demo-qate2/demo-qate2.h )
ADD_EXECUTABLE(DEMO_QATE2 demo-qate2/demo-qate2.cpp ${DEMO_QATE2_MOC})
TARGET_LINK_LIBRARIES(DEMO_QATE2 QATE_STATIC ${QT_LIBRARIES} )
SET_TARGET_PROPERTIES(DEMO_QATE2 PROPERTIES OUTPUT_NAME qate-demo2)

# demo 3 - the text editor control
#INCLUDE_DIRECTORIES( ../ . ${CMAKE_BINARY_DIR}/../../demos/demo4/)
QT4_WRAP_UI( DEMO_QATE3_UI
	demo4/bannermessage.ui
	demo4/replaceform.ui
	demo4/searchform.ui
)
QT4_WRAP_CPP( DEMO_QATE3_MOC
	demo4/qsvtextedit.h
	demo4/qsvtextoperationswidget.h
	demo4/mainwindow4.h
)
ADD_EXECUTABLE( DEMO_QATE3
	demo4/demo4.cpp
	demo4/qsvsyntaxhighlighterbase.cpp
	demo4/qsvdefaulthighlighter.cpp
	demo4/qsvtextedit.cpp
	demo4/qsvtextoperationswidget.cpp
	${DEMO_QATE3_MOC}
	${DEMO_QATE3_UI}
)
TARGET_LINK_LIBRARIES(DEMO_QATE3 QATE_STATIC ${QT_LIBRARIES} )
SET_TARGET_PROPERTIES(DEMO_QATE3 PROPERTIES OUTPUT_NAME qate-demo3)

# demo 4 - text editor using the qate syntax highlighter
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/demo-qate4/demo-qate4.h  ${CMAKE_CURRENT_BINARY_DIR}/demo-qate4.h COPYONLY)
QT4_WRAP_CPP( DEMO_QATE4_MOC
	demo4/qsvtextedit.h
	demo4/qsvtextoperationswidget.h
	demo-qate4/demo-qate4.h
)
	#demo4/qatehighlighterbase.cpp
ADD_EXECUTABLE( DEMO_QATE4
	demo-qate4/demo-qate4.cpp
	demo-qate4/qatehighlighter.cpp
	demo4/qsvsyntaxhighlighterbase.cpp
	demo4/qsvtextedit.cpp
	demo4/qsvtextoperationswidget.cpp
	${DEMO_QATE4_MOC}
)
QT4_WRAP_CPP( DEMO_QATE4_MOC
	demo-qate4/demo-qate4.h
)
TARGET_LINK_LIBRARIES(DEMO_QATE4 QATE_STATIC ${QT_LIBRARIES} )
SET_TARGET_PROPERTIES(DEMO_QATE4 PROPERTIES OUTPUT_NAME qate-demo4)

if (fasle)
endif()
