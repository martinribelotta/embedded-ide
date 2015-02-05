#ifndef __SAMPLEPANEL_H__
#define __SAMPLEPANEL_H__

/*
#if QT_VERSION >= 0x040400
#	define	QTextEditorControl	QPlainTextEdit
#	warning	"Using QPlainTextEdit as the text editor control"
#else
#	define	QTextEditorControl	QTextEdit
#endif
*/

#define	QTextEditorControl	QTextEdit

#include <QWidget>
class QTextEdit;
class QTextEditorControl;

class QsvEditor;

class QsvEditorPanel : public QWidget
{
	Q_OBJECT
public:
	QsvEditorPanel(QTextEditorControl *editor);
private:
	void paintEvent(QPaintEvent*);
	QTextEditorControl *m_edit;
	QColor m_panelColor;
	QColor m_modifiedColor;
	QPixmap m_bookMarkImage;
friend class QsvEditor;
};

#endif // __SAMPLEPANEL_H__
