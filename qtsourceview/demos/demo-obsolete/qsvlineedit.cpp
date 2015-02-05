#include <QPixmap>
#include <QToolButton>
#include <QStyle>
#include "qsvlineedit.h"

QsvLineEdit::QsvLineEdit( QWidget *parent ) 
	: QLineEdit(parent)
{
	clearButton = new QToolButton( this );
	clearButton->setCursor(Qt::ArrowCursor);
	clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
	clearButton->setAutoRaise( true );
	clearButton->hide();

	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	QSize msz = minimumSizeHint();
	setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(clearButton->sizeHint().width() + frameWidth + 1));
	setMinimumWidth( qMax(msz.width(), width() + frameWidth * 2 + 2) );
	//setMinimumHeight( qMax(msz.height(), height() + frameWidth * 2 + 2) );

	connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
	connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateEditLine()));
}

void	QsvLineEdit::setIcon(QPixmap i)
{
	clearButton->setIcon( i );
	//clearButton->setIconSize(i.size());
}

void	QsvLineEdit::resizeEvent(QResizeEvent *e)
{
	QLineEdit::resizeEvent( e );

	QSize sz = clearButton->sizeHint();
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	clearButton->move( rect().right() - frameWidth - sz.width(), (rect().bottom() + 1 - sz.height())/2 );	
}

void	QsvLineEdit::updateEditLine()
{
	clearButton->setVisible( ! displayText().isEmpty() );
}
