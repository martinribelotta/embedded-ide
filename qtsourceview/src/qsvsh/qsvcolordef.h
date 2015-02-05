#ifndef __QSV_COLOR_DEF_H__
#define __QSV_COLOR_DEF_H__

/**
 * \file qsvcolordef.h
 * \brief Definition of the color defintion
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see QsvColorDef
 */

#include <QMap>

class QDomNode;
class QColor;
class QTextCharFormat;
class QString;

class QsvColorDef
{
public:
	QsvColorDef();
	QsvColorDef( QDomNode node );
	bool load( QDomNode node );
	bool save( QDomNode node );

	bool isBold();
	bool isUnderline();
	bool isItalic();
	QColor getColor();
	QColor getSelColor();
	QColor getBackground();
	QString getStyleNum();
	QTextCharFormat toCharFormat();
private:
	QMap<QString,QString> attributes;
};

#endif // __QSV_COLOR_DEF_H__
