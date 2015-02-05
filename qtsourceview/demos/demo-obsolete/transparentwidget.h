#ifndef __QSV_TRANSPARENTWIDGET_H__
#define __QSV_TRANSPARENTWIDGET_H__

#include <QWidget>

class QsvTransparentWidget: public QWidget
{
public:
	QsvTransparentWidget( QWidget *parent, qreal t=1 );
	qreal getWidgetTransparency();
	void setWidgetTransparency( qreal );
	
protected:
	void paintEvent(QPaintEvent*);
	
private:
	qreal widgetTransparency;
};

#endif // __QSV_TRANSPARENTWIDGET_H__
