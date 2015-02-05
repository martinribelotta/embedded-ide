/**
 * \file qsvcolordef.cpp
 * \brief Implementation of the color defintion
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see QsvColorDef
 */

#include <QString>
#include <QDomNode>
#include <QColor>
#include <QTextCharFormat>

#include "qsvcolordef.h"

/**
 * \class QsvColorDef 
 * \brief A color definition abstraction
 * 
 * This class is an abstraction of a color definition, which can be
 * loaded from XML file. It represents a single definition (for example
 * the color definition of a string).
 * 
 * The XML syntax of the color definition is inspired by the syntax highlight 
 * teh kate project (http://kate.kde.org/, http://www.kate-editor.org/)
 * 
 * The syntax of the color definition is simple
 * - a node with tag name of \b itemData
 * - all the properties of that node are rendered as properties for this color definition
 * 
 * For example:
 * <pre>
 * &gt; itemData defStyleNum="dsNormal" name="Normal" color="#000000" selColor="#ffffff" background="#ffffff"  bold="0" italic="0" / &lt;
 * </pre>
 * 
 * This class can also be converted to a QTextCharFormat on demand.
 * 
 * \see QsvColorDefFactory
 */
 
 
/**
 * \brief default constructor
 * 
 * Construct a default color definition. By default no color not background is defined
 * which makes this color definition "neutral".
 */
QsvColorDef::QsvColorDef()
{
// 	TODO is it safe to remove this code?
// 	attributes["color"] = "black";
// 	attributes["background"] = "white";
}

/**
 * \brief construct a color definition from a node 
 * \param node the DOM node from which to load the definitions
 * 
 * Construct a default color definitino from an XML dom node. The definition of the colors
 * will be read by calling load( QDomNode )
 * 
 * \see load( QDomNode )
 */
QsvColorDef::QsvColorDef( QDomNode node )
{
	load( node );
}

/**
 * \brief load the definition of the colors from a node
 * \param node the DOM node from which to load the definitions
 * \return true on succes, false on ay error
 * 
 * Load the definition of this color from a DOM node. An example of the
 * node which is loaded is:
 * 
 * <pre>
 * &gt; itemData defStyleNum="dsNormal" name="Normal" color="#000000" selColor="#ffffff" background="#ffffff"  bold="0" italic="0" / &lt;
 * </pre>
 *
 * This syntax is similar to the kate syntax definitions code, and initialy inspired by it.
 */
bool	QsvColorDef::load( QDomNode node )
{
	uint attrCount = node.attributes().count();
	for( uint i=0; i< attrCount; i++ )
	{
		attributes[node.attributes().item(i).nodeName()] =
			node.attributes().item(i).nodeValue();
	}

	return true;
}

/**
 * \brief save the definition of the colors into a node
 * \param node the DOM node to store the colors definitions
 * \return true on succes, false on ay error
 * 
 * Currently not implemented and returns false.
 * 
 * In future it will stire the definitions of this color into
 * the node.
 * 
 */
bool	QsvColorDef::save( QDomNode node )
{
//QStringMap attributes;
	return false;
}

/**
 * \brief this property represents if the color definition is "bold"
 * \return true if the color definition is bold
 * 
 * Will return true if the loaded value of the attribute \b bold by the load( QDomNode ) is "true"
 * "yes" or "1" (not case sensitive). All other values will return false.
 */
bool	QsvColorDef::isBold()
{
	if (!attributes.contains("bold"))
		return false;
	
	QString boldAttr = attributes["bold"].toLower();
	if ((boldAttr == "1") || (boldAttr == "true") || (boldAttr == "yes"))
		return true;
	else
		return false;
}

/**
 * \brief this property represents if the color definition is "underline"
 * \return true if the color definition is underline
 * 
 * Will return true if the loaded value of the attribute \b underline by the load( QDomNode ) is "true"
 * "yes" or "1" (not case sensitive). All other values will return false.
 */
bool	QsvColorDef::isUnderline()
{
	if (!attributes.contains("underline"))
		return false;
	
	QString boldAttr = attributes["underline"].toLower();
	if ((boldAttr == "1") || (boldAttr == "true") || (boldAttr == "yes"))
		return true;
	else
		return false;
}

/**
 * \brief this property represents if the color definition is "italic"
 * \return true if the color definition is italic
 * 
 * Will return true if the loaded value of the attribute \b italic by the load( QDomNode ) is "true"
 * "yes" or "1" (not case sensitive). All other values will return false.
 */
bool	QsvColorDef::isItalic()
{
	if (!attributes.contains("italic"))
		return false;
	
	QString boldAttr = attributes["italic"].toLower();
	if ((boldAttr == "1") || (boldAttr == "true") || (boldAttr == "yes"))
		return true;
	else
		return false;
}

/**
 * \brief the font color of this color definition
 * \return a QColor represnting the forground color 
 *
 * Use this function to retrieve the QColor representation
 * of this color definition. If no color defined, it will return
 * black (by constructing an empthy QColor)
 * 
 */
QColor	QsvColorDef::getColor()
{
	if (!attributes.contains("color"))
		return QColor();
	else 
		return QColor( attributes["color"] );
}

/**
 * \brief the font color for selected text of this color definition
 * \return a QColor represnting the selection forground color 
 *
 * Use this function to retrievethe QColor representation
 * of this color definition. If no color defined, it will return
 * black (by constructing an empthy QColor).
 *
 */
QColor	QsvColorDef::getSelColor()
{
	if (!attributes.contains("selColor"))
		return QColor();
	else 
		return QColor( attributes["selColor"] );
}

/**
 * \brief the background color of this color definition
 * \return a QColor represnting the background color 
 *
 * Use this function to retrievethe QColor representation
 * of this color definition. If no color defined, it will return
 * black (by constructing an empthy QColor).
 *
 */
QColor	QsvColorDef::getBackground()
{
	if (!attributes.contains("background"))
		return QColor();
	else 
		return QColor( attributes["background"] );
}

/**
 * \brief return the style number definition
 * \return a string represnting the style number definition
 * 
 * Each color definition should have an attribute called "defStyleNum"
 * which represents the style of this color. This way you can collect all 
 * the color definitions into a factory and styart quering it for a specific
 * role using this attribute.
 * 
 * \see QsvColorDefFactory
 */
QString	QsvColorDef::getStyleNum()
{
//	TODO is this needed? safe to remove?
//	if (!attributes.contains("defStyleNum"))
//		return QString();

	return attributes["defStyleNum"];
}

/**
 * \brief convert this color definition to a QTextCharFormat
 * \return a QTextCharFormat which contains the same defintino as this class.
 * 
 * This function converts this class into a QTextCharFormat which can 
 * be used in QSyntaxHighlighter class for example.
 */
QTextCharFormat QsvColorDef::toCharFormat()
{
	QTextCharFormat f;

	if (attributes.contains("color"))
		f.setForeground( QColor(attributes["color"]) );

	if (attributes.contains("background"))
		f.setBackground( QColor(attributes["background"]) );

	if (isBold()) 
		f.setFontWeight(QFont::Bold);
	
	if (isItalic())
		f.setFontItalic( true );

	if (isUnderline())
		f.setFontUnderline( true );

	return f;
}
