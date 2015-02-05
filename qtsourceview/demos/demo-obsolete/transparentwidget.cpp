#include <QPainter>
#include "transparentwidget.h"

QsvTransparentWidget::QsvTransparentWidget( QWidget *parent, qreal t )
	:QWidget(parent)
{
	widgetTransparency = t;
}

qreal	QsvTransparentWidget::getWidgetTransparency()
{
	return widgetTransparency;
}

void	QsvTransparentWidget::setWidgetTransparency( qreal t )
{
	widgetTransparency = t;
	update();
}

void	QsvTransparentWidget::paintEvent(QPaintEvent*)
{
	QPainter p( this );
	p.setOpacity( widgetTransparency );
	p.fillRect(rect(), palette().background() );
}
