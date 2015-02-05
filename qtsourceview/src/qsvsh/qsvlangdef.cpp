/**
 * \file qsvlangdef.cpp
 * \brief Implementation of the language definition, and support structs
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see QsvLangDef
 */

#include <QString>
#include <QStringList>
#include <QDomDocument>
#include <QDomNode>
#include <QFile>

#include "qsvlangdef.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// not documented classes, yet

/**
 * \class QsvEntityDef 
 * \brief A container for a syntax entity definition
 *
 */

/**
 * \class QsvEntityString
 * \brief 
 *
 */

/**
 * \class QsvEntityLineComment
 * \brief 
 *
 */

/**
 * \class QsvEntityBlockComment
 * \brief 
 *
 */

/**
 * \class QsvEntityPatternItem
 * \brief 
 *
 */

/**
 * \class QsvEntityKeywordList
 * \brief 
 *
 */
#endif // DOXYGEN_SHOULD_SKIP_THIS

/**
 * \class QsvLangDef
 * \brief A container class for GtkSourceView syntax definitions
 * 
 * GtkSourceView is a text widget that extends the standard gtk+ 2.x text widget:
 * http://gtksourceview.sourceforge.net/
 * 
 * The syntax definitions used by that library are stored as XML files.
 * This class provides an object oriented, Qt4 based way of loading those 
 * definitions, and later on use them for syntax highlighter classes.
 */


/**
 * \brief Constructor for the QsvLangDef class
 * \param fileName the file from which the definitions should be loaded
 *
 * This constructor builds the language definition by loading the definitions
 * from a file.
 */
QsvLangDef::QsvLangDef( QString fileName )
{
	load( fileName );
};

/**
 * \brief Constructor for the QsvLangDef class
 * \param doc the dom document from which the definitions should be loaded
 *
 * This constructor builds the language definition by loading the definitions
 * from a QDomDocument class.
 */
QsvLangDef::QsvLangDef( QDomDocument doc )
{
	load( doc );
};

/**
 * \brief Destructor for the QsvLangDef class
 * 
 * Destructs the language definition
 */
QsvLangDef::~QsvLangDef()
{
};

/**
 * \brief load a language definition from an XML file stored on the disk
 * \param fileName The file from which the synatx should be loads
 * \return true on sucess, false on any error
 * 
 * Loads the language definitions from a file by constructing a QDomDocument
 * class with \b fileName and then calling load() with the new constructed
 * QDomDocument.
 *
 * \see load( QDomDocument )
 */
bool	QsvLangDef::load( QString fileName )
{
	QDomDocument doc("language");
	QFile file(fileName);
	QString s;

	if (!file.open(QIODevice::ReadOnly))
	{
#ifdef	__DEBUG_LANG_DEF_LOAD__
		qDebug( "%s %d : Could not open %s", __FILE__, __LINE__, qPrintable(fileName) );
#endif		
		return false;
	}

	if (!doc.setContent(&file))
	{
		file.close();
#ifdef	__DEBUG_LANG_DEG_LOAD__
		qDebug( "%s %d : Could not open %s", __FILE__, __LINE__, qPrintable(fileName) );
#endif
		return false;
	}
	file.close();
	
	return load( doc );
}

/**
 * \brief load a language definition from an XML file stored on the disk
 * \param doc the dom document from which the language should be loaded.
 * \return true on sucess, false on any error
 *
 * Loads the language definition from the XML document \b doc.
 * Basic checks are done on the synatx, the error checking is very
 * minimal, so having a good valid XML is a good idea with this
 * implementation.
 */
bool	QsvLangDef::load( QDomDocument doc )
{
	QDomNodeList list, l;
	QDomNode n,m;
	uint attrCount; 
	
	// read information about this syntax highlight
	list = doc.elementsByTagName("language");
	n = list.item(0);

	// we assume that if the fist line is well decalred, everything else is cool
	if (! n.hasAttributes() ) 
		return false;

	attrCount = n.attributes().count();
	
	for( uint i=0; i<attrCount; i++ )
	{
		QString name = n.attributes().item(i).nodeName();
		QString value = n.attributes().item(i).nodeValue();
		
		if (name=="mimetypes")
			mimeTypes = value.split(QRegExp("[;,]"));
		else if (name=="extensions")
			extensions = value.split(";");
		else
			attributes[name] = value;
	}

	// read the entities which define this language/syntax
	list = doc.elementsByTagName("escape-char");
	escapeChar = list.item(0).nodeValue();

	if (!loadLineComments( doc.elementsByTagName("line-comment") ))	
		return false;

	if (!loadStrings( doc.elementsByTagName("string") ))
		return false;

	if (!loadPatternItems( doc.elementsByTagName("pattern-item")))
		return false;

	if (!loadBlockComments( doc.elementsByTagName("block-comment"), blockCommentsDefs ))
		return false;

	if (!loadKeywordList( doc.elementsByTagName("keyword-list") ))
		return false;

	if (!loadBlockComments( doc.elementsByTagName("syntax-item"), syntaxItemDefs ))
		return false;

	return true;
};

/**
 * \brief returns the version of this syntax
 * 
 * This return the version of the syntax definition, as defined in the XML 
 * file for this syntax. You can use it for displaying the in GUIs instead 
 * of the file name.
 */
QString QsvLangDef::getVersion()
{
	return attributes["_version"];
}

/**
 * \brief returns the name of this syntax
 * 
 * This return a descriptive name for this syntax. You can use it for displaying 
 * the in GUIs instead of the file name.
 */
QString	QsvLangDef::getName()
{
	return attributes["_name"];
}

/**
 * \brief returns the section of this syntax
 * 
 * This return the section to which this syntyax definition belongs to.
 *
 * \see getMimeTypes()
 */
QString QsvLangDef::getSection()
{
	return attributes["_section"];
}

/**
 * \brief Get the supported mime types of this syntax definition
 * \return a string list, which represents the all the mime types supported by this syntax
 * 
 * Each syntax definition includes a list of mime types supported by this
 * syntax definition. For more information about mime types look in RFC 2045, 2046, 2047,
 * 2048, and 2077.
 * 
 * It's up to an upper level API to match file names into mime types, and
 * this class does not take 
 */
QStringList QsvLangDef::getMimeTypes()
{
	return mimeTypes;
}

/**
 * \brief helper function for checking the boolean value of a node
 * \param s the value to check
 * \return true if the string is a valid "true"
 *
 * This is a helper function to check the value of a string. The
 * syntax of GTK source view demands that the value "TRUE" will be
 * logical true, but this function extends that definition by returning "true"
 * on these conditions:
 *  - the string equals "true" (non case sensitive, "True" and "TrUe" also valid)
 *  - the string equals "yes" (non case sensitive, "yeS" and "YES" also valid)
 *  - the string equals "1"
 */
bool	QsvLangDef::isTrue( QString s )
{
	bool b = false;

	s  = s.toLower();
	if (s == "true")	
		b = true;
	else if (s == "yes")
		b = true;
	else if (s == "1")
		b = true;
	
	return b;
}

/**
 * \brief load the value of an entity from a XML node
 * \param node the dom node from which the value should be read
 * \return true on sucess, false on any error
 *
 * Load an entity from the dom node passed on as a parameter.
 * The value of the entity will be filled from the attributes of the node.
 */
bool	QsvLangDef::loadEntity( QDomNode node, QsvEntityDef &entity )
{
	try
	{
		entity.name	= node.attributes().namedItem("_name").nodeValue();
		entity.style	= node.attributes().namedItem("style").nodeValue();
		entity.type	= node.toElement().tagName();
	}
	catch( ... )
	{
		// does this shite even works on gcc?
		return false;
	}
	
	return true;
}

/**
 * \brief load the definition of the line commends from the node list
 * \param nodes the node list from which the definition of the line comments should be loaded
 * \return true on sucess, false on any error
 *
 * This function loads the definition of the line comments on this syntax from the list of
 * node list passed as a parameter.
 */
bool	QsvLangDef::loadLineComments( QDomNodeList nodes )
{
	QDomNode node;
	int i, size = nodes.size();

	for( i=0; i<size; i++ )
	{
		QsvEntityLineComment e;
		node = nodes.at( i );

		if (!loadEntity( node, e )) 
			return false;
		e.start = node.toElement().elementsByTagName("start-regex").item(0).toElement().text();

		// WTF???
		e.start.replace( "\\\\", "\\" );
		
		lineCommentsDefs << e;
	}

	return true;
}

/**
 * \brief load the definition of the strings from the node list
 * \param nodes the node list from which the definition of the line comments should be loaded
 * \return true on sucess, false on any error
 *
 * This function loads the definition of the strings on this syntax from the node list passed as a parameter.
 */
bool	QsvLangDef::loadStrings( QDomNodeList nodes )
{
	QDomNode node;
	QString s;
	int i, size = nodes.size();

	for( i=0; i<size; i++ )
	{
		QsvEntityString e;
		node = nodes.at( i );
		
		if (!loadEntity( node, e )) 
			return false;
		
		e.atEOL      = isTrue( node.attributes().namedItem("end-at-line-end").nodeValue() );
		e.startRegex = node.toElement().elementsByTagName("start-regex").item(0).toElement().text();
		e.endRegex   = node.toElement().elementsByTagName("end-regex").item(0).toElement().text();
		// WTF???
		e.startRegex.replace( "\\\\", "\\" );
		e.endRegex.replace( "\\\\", "\\" );
		stringsDefs << e;
	}
	
	return true;
}

/**
 * \brief load the definition of the patterns in this syntax
 * \param nodes the node list from which the definition of the patterns should be loaded
 * \return true on sucess, false on any error
 *
 * This function loads the definition of the patterns on this syntax from the node list passed as a parameter.
 *
 * TODO
 *   define what are patterns
 */
bool	QsvLangDef::loadPatternItems( QDomNodeList nodes )
{
	QDomNode node;
	int i, size = nodes.size();

	i = patternItems.size();

	for( i=0; i<size; i++ )
	{
		node = nodes.at( i );

		QsvEntityPatternItem e;
		if (!loadEntity( node, e )) return false;
		e.regex = node.toElement().elementsByTagName("regex").item(0).toElement().text();

		// WTF???
// 		e.regex.replace( "\\\\", "\\" );
		e.regex.replace( "\\n", "$" );
		
		patternItems << e;
	}

	return true;
}

/**
 * \brief load the definition of the block comments in this syntax
 * \param nodes the node list from which the definition of the block comments should be loaded
 * \param list the links of block comment definitions
 * \return true on sucess, false on any error
 *
 * This function loads the definition of the block comments on this syntax from the node list passed as a parameter
 * into the list of block comments passed as a parameter.
 */
bool	QsvLangDef::loadBlockComments( QDomNodeList nodes, QList<QsvEntityBlockComment> &list )
{
	QDomNode node;
	int i, size = nodes.size();

	i = list.size();

	for( i=0; i<size; i++ )
	{
		node = nodes.at( i );

		QsvEntityBlockComment e;
		if (!loadEntity( node, e ))
			return false;
		
		e.startRegex	= node.toElement().elementsByTagName("start-regex").item(0).toElement().text();
		e.endRegex	= node.toElement().elementsByTagName("end-regex").item(0).toElement().text();
		
		// WTF???
		e.startRegex.replace( "\\\\", "\\" );
		e.endRegex.replace( "\\\\", "\\" );
		
		list << e;
	}

	return true;
}

/**
 * \brief load the keyword definition list from
 * \param nodes the node list from which the definition of the block comments should be loaded
 * \return true on sucess, false on any error
 *
 * This function loads the definition of the keywords list on this syntax from the node list passed as a parameter.
 */
bool	QsvLangDef::loadKeywordList( QDomNodeList nodes )
{
	QDomNodeList strs;
	QDomNode str;
	QString s;

	int i, size = nodes.size();
	int j;
	
	for( i=0; i<size; i++ )
	{
		QsvEntityKeywordList e;
		QDomNode node = nodes.at( i );
		
		if (!loadEntity( node, e ))
			return false;
		e.list.clear();

		e.caseSensitive			= isTrue( node.attributes().namedItem("case-sensitive").nodeValue() );
		e.matchEmptyStringAtBeginning	= isTrue( node.attributes().namedItem("match-empty-string-at-beginning").nodeValue() );
		e.matchEmptyStringAtEnd		= isTrue( node.attributes().namedItem("match-empty-string-at-end").nodeValue() );
		e.startRegex			= node.attributes().namedItem("beginning-regex").nodeValue();
		e.endRegex			= node.attributes().namedItem("end-regex").nodeValue();

		// WTF???
		e.startRegex.replace( "\\\\", "\\" );
		e.endRegex.replace( "\\\\", "\\" );
		
		// read strings
		strs = node.toElement().elementsByTagName("keyword");
		for( j=0; j<strs.size(); j++ )
		{
			str = strs.item( j );
			e.list << str.toElement().text();
		}
		keywordListDefs << e;
	}
	
	return true;
}
