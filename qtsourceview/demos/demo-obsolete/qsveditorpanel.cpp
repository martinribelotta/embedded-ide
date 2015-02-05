#if QT_VERSION >= 0x040400
#	include <QPlainTextEdit>
#else
#	include <QTextEdit>
#endif

#include <QPainter>
#include <QPaintEvent>
#include <QTextBlock>
#include <QTextLayout>
#include <QScrollBar>

#include "qsvprivateblockdata.h"
#include "qsveditorpanel.h"
#include "qsveditor.h"

QsvEditorPanel::QsvEditorPanel(QTextEditorControl *a): QWidget(a)//, m_area(a)
{
	m_edit = a;
	m_modifiedColor = Qt::green;
	m_panelColor = QColor("#FFFFD0");
	m_bookMarkImage = QPixmap(":/images/emblem-important.png");

	setFixedWidth(50);
	connect(m_edit->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(update()));
}

void QsvEditorPanel::paintEvent(QPaintEvent*)
{
	QPainter p( this );
	QRect r = geometry();
	
	p.fillRect( r, m_panelColor );
	
	int contentsY = m_edit->verticalScrollBar()->value();
	qreal pageBottom = contentsY + m_edit->viewport()->height();
	int m_lineNumber = 1;
	const QFontMetrics fm = fontMetrics();
	const int ascent = fontMetrics().ascent() +1;
	QTextBlock currentBlock = m_edit->textCursor().block();
	
	for ( QTextBlock block = m_edit->document()->begin(); block.isValid(); block = block.next(), m_lineNumber++ )
	{
		QTextLayout* layout = block.layout();
		const QRectF boundingRect = layout->boundingRect();
		QPointF position = layout->position();
		QsvPrivateBlockData *data = dynamic_cast<QsvPrivateBlockData*>( block.userData() );

		if ( position.y() +boundingRect.height() < contentsY )
			continue;
		if ( position.y() > pageBottom )
			break;
		
		const QString txt = QString::number( m_lineNumber );
		
		if (block == currentBlock)
		{
			QFont f = p.font();
			f.setBold( true );
			p.setFont( f );
		}
		p.drawText( width() -fm.width( txt ) - 7, qRound( position.y() ) - contentsY +ascent, txt ); // -fm.width( "0" ) is an ampty place/indent 
		if (block == currentBlock)
		{
			QFont f = p.font();
			f.setBold( false );
			p.setFont( f );
		}
		
		if (data)
		{
			if (data->m_isBookmark)
				p.drawPixmap( 2, qRound(position.y() - contentsY + ascent - m_bookMarkImage.height()), m_bookMarkImage ); 
			if (data->m_isModified)
				p.fillRect( width()- 3, qRound(position.y()-contentsY), 2, qRound(boundingRect.height()), m_modifiedColor );
		}
	}
}
