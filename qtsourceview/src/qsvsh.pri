QT      += xml core gui widgets

RESOURCES	+=	$$QTSOURCEVIEW_SRC_DIR/qsvsh/qtsourceview.qrc

HEADERS += \
	$$QTSOURCEVIEW_SRC_DIR/qsvsh/qorderedmap.h \
	$$QTSOURCEVIEW_SRC_DIR/qsvsh/qsvcolordef.h \
	$$QTSOURCEVIEW_SRC_DIR/qsvsh/qsvcolordeffactory.h \
	$$QTSOURCEVIEW_SRC_DIR/qsvsh/qsvlangdef.h \
	$$QTSOURCEVIEW_SRC_DIR/qsvsh/qsvlangdeffactory.h \
	$$QTSOURCEVIEW_SRC_DIR/qsvsh/qsvsyntaxhighlighter.h \
	$$QTSOURCEVIEW_SRC_DIR/qsvsh/debug_info.h
	
SOURCES += \
	$$QTSOURCEVIEW_SRC_DIR/qsvsh/qsvcolordef.cpp \
	$$QTSOURCEVIEW_SRC_DIR/qsvsh/qsvcolordeffactory.cpp \
	$$QTSOURCEVIEW_SRC_DIR/qsvsh/qsvlangdef.cpp \
	$$QTSOURCEVIEW_SRC_DIR/qsvsh/qsvlangdeffactory.cpp \
	$$QTSOURCEVIEW_SRC_DIR/qsvsh/qsvsyntaxhighlighter.cpp

INCLUDEPATH += $$QTSOURCEVIEW_SRC_DIR/