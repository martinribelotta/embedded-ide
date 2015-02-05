/**
 * \file qsvlangdeffactory.cpp
 * \brief Implementation of the language definition factory
 * \date 2006-07-21 23:15:55
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see QsvLangDefFactory
 */
 
#include <QMessageBox>
#include <QRegExp>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include "qsvlangdeffactory.h"
#include "qsvlangdef.h"

#include "debug_info.h"

/**
 * \class QsvLangDefFactory
 * \brief An abstract factory for languages definitions
 *
 * When making an editor (for example), you will like to load different file types, for
 * every different one the use might expect a different highlight defintion.
 * This class is a factory for those defintions: you load the defintions
 * from a directory, and then you start quering for hightlighs for specific
 * file names.
 *
 * The class is defined as a singleton class, which means you do not
 * need to make an instanse, and not cary about memory allocations.
 *
 * \see QsvLangDef
 */


/**
 * \var QsvLangDefFactory::LangFactory
 * \brief internal pointer of the singleton
 * 
 * The pointer to the single instanse of this class. Since this
 * is private, you shuld generally use getInstanse() instead.
 * 
 * \see getInstanse()
 */
QsvLangDefFactory *QsvLangDefFactory::LangFactory = NULL;


// public stuff....


/**
 * \brief return the instace of the object
 * \return the instanse of the factory
 * 
 * This class is defined as a singleton class, with a private constructor
 * the only way to interact with this class is by calling this function
 * is by getting the current instanse and then using the retured pointer.
 * 
 * If this is the first time you call this function and 
 * QsvLangDefFactory::LangFactory is not allocated, the constructor will
 * be called for you.
 * 
 */
QsvLangDefFactory *QsvLangDefFactory::getInstanse()
{
	if (LangFactory == NULL)
	{
		LangFactory = new QsvLangDefFactory();
	}

#ifdef __DEBUG_FOUND_LANG_DEF__	
	qDebug( "%s %d - gettng instanse %p",
		__FILE__, __LINE__,
	       LangFactory
	);
#endif	
	return LangFactory;
}

/**
 * \brief query the factory for a specific syntax definition
 * \return a pointer to the correct syntax definition or NULL if none found
 * 
 * The next stage after loading a group of syntax definitions if to ask
 * this factory for a syntax definition which should be used to represent 
 * a file. This function look for the syntax definition according to the
 * list of mime types defined internally and return a pointer to the corresponding
 * syntax definition, or NULL if none fonud.
 * 
 * The search is done by quering each of the languages for it's mime types, then
 * looking up in an internal list for the correct mime type and compering the globs
 * of that mime type to the file name passed, if there is a match the language is
 * returned as a pointer.
 * 
 * The represented pointer should not be unallocated by the user, as it's unallocated
 * by this factory.
 * 
 * \see addDefaultMimeTypes()
 * \see addMimeTypes()
 * \see clearMimeTypes()
 */
QsvLangDef* QsvLangDefFactory::getHighlight( QString fileName )
{
	QsvLangDef *langDef;
	QString langMimeType;
	QString trimmedFileName = fileName;
	
	// try to find the file name, without directory
	int i = fileName.lastIndexOf( '/' );
	if (i == -1 )
		i = fileName.lastIndexOf( '\\' );	
	if (i != -1 )
		trimmedFileName = fileName.right( fileName.length() - i - 1 );
	
	foreach( langDef, langList )
	{
		foreach( langMimeType, langDef->getMimeTypes() )
		{
			// if do we recognize the mime type defined in that
			// syntax highligh definition, check if matches this file

			if ( !mimeTypes.contains(langMimeType) )
			{
				// TODO
				//qDebug( "%s %d - Unknown mimetype [%s] at highlight file %s", __FILE__, __LINE__, qPrintable(langMimeType), qPrintable(langDef->getName()) );
				continue;
			}
			
			for ( int j=0; j<mimeTypes[langMimeType].count(); j ++ ) 
			{
				QString s = "*." + mimeTypes[langMimeType][j];
				
				if	(
						// match full names like Makefile, Doyxgen, Changelog, etc
						(mimeTypes[langMimeType][j] == trimmedFileName) ||
						// otherwise match by extensions
						QDir::match( s, fileName) 
					) // still the "if"
				{
#ifdef __DEBUG_FOUND_LANG_DEF__
					qDebug( "%s %d - Found language definition %s [%s,%s]",
						__FILE__, __LINE__,
						qPrintable(langDef->getName()),
						qPrintable(fileName), qPrintable( "*" + mimeTypes[langMimeType][j] )
					);
#endif
					return langDef;
				}
			}
		}
	}

#ifdef __DEBUG_FOUND_LANG_DEF__
	qDebug( "%s %d - Error: Not found any highlighter for [%s]", __FILE__, __LINE__, qPrintable(fileName) );
#endif
	return NULL;
}

/**
 * \brief scan a directory for language definitions
 * \param directory the directory to be scanned
 *
 * The easiest way for populating the list of syntax definitinos found 
 * in this class is to let it scan in a directory for languages definitions
 * and add them to it's internal list.
 *
 * The directory passed as a parameter will be scanned for *.lang files. Each
 * \b lang file must be a valid XML to be loaded by a QsvLangDef file (must be
 * a valid GTK Source View language definition.
 *
 * \see QsvLangDef
 * \see QsvLangDefFactory::getHighlight( QString )
 */
void QsvLangDefFactory::loadDirectory( QString directory )
{
	if (directory.isEmpty())
		directory = QDir::currentPath();
	QDir dir(directory, "*.lang");

	QStringList files = dir.entryList(QDir::Files | QDir::NoSymLinks);
	int fileCount =	files.count();

	if (fileCount == 0)
	{
		qDebug( "%s %d - Error: no highlight definitions found at directory: %s", __FILE__, __LINE__, qPrintable(directory) );
		QMessageBox::information(0, "Application name", 
			"no highlight definitions found at directory " + directory
		);
		return;
	}

	for (int i = 0; i < fileCount; ++i)
	{
		QsvLangDef *langDef = new QsvLangDef ( directory + "/" + files[i] );
		langList << langDef;
		QString langMimeType;

#ifdef __DEBUG_LANGS_MIMES_
		foreach( langMimeType, langDef->getMimeTypes() )
		{
			if ( mimeTypes.find(langMimeType) == mimeTypes.end() )
			{
				qDebug("%s %d - Warning: highlight file %s - unknown mimetype [%s]",
					__FILE__, __LINE__,
					qPrintable(langDef->getName()), qPrintable(langMimeType)
				);
			}
		}
#endif		
	}
}

/**
 * \brief clear the internal list of suported mime types
 * 
 * This function cleans the internal list of supported mime types. 
 * 
 * \see addDefaultMimeTypes()
 * \see addMimeTypes()
 */
void	QsvLangDefFactory::clearMimeTypes()
{
	mimeTypes.clear();
}

/**
 * \brief install a default set of mime types
 * \return true on success, false on any error
 * 
 * In oder to recognize a syntax highlight by the file name extension,
 * one needs to install a set of mime types. By default this factory installs
 * a pre defined internal mime types set, which is tested for a a minimal set
 * of languages.
 * 
 * It is possible to add more mime types later on using addMimeTypes().
 * 
 * \see addMimeTypes( QString )
 * \see clearMimeTypes()
 * \see getHighlight( QString )
 */
bool	QsvLangDefFactory::addDefaultMimeTypes()
{
	Q_INIT_RESOURCE(qtsourceview);
	return addMimeTypes( ":/mime.types" );
}

/**
 * \brief install a set of mime types
 * \return true on success, false on any error
 * 
 * If you expect you application to have more extensions added by the end users
 * they will need to add a language definition and a mime type. One way of implementing 
 * this is to have a directory in which users can put *.lang files, and on it's root make 
 * an empty "mime.types" for them to add their own mime types.
 * 
 * Calling this function will add mime types to the internal list. If you 
 * wish you can call clearMimeTypes() before to use only your mime types list
 * 
 * \see clearMimeTypes()
 * \see getHighlight( QString )
 */
 
bool	QsvLangDefFactory::addMimeTypes( QString fileName )
{
	// load mime types
	QFile file( fileName );

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
#ifdef __DEBUG_LANGS_MIMES__
		qDebug("%s %d - Error: could not load mime.types [%s]",
			__FILE__, __LINE__,
			qPrintable(fileName)
		);
#endif
		return false;
	}
		
	// parse built in mime types definitions
	QTextStream in(&file);
	while (!in.atEnd())
	{
		QString line = in.readLine();
		
		if (line.startsWith("#"))
			continue;

		QStringList l = line.split( QRegExp("\\s+") );
		QString     name = l[0];
		l.removeAt( 0 );

		if (!l.empty())
		{
#ifdef __DEBUG_LANGS_MIMES__
			QString s;
			for ( int j=0; j<l.count(); j ++ )
				s = s + "*." + l[j] + ",";
			qDebug( "%s %d - Info: loaded mime type %s -> %s", __FILE__, __LINE__, qPrintable(name), qPrintable(s) );
#endif
			mimeTypes[name] = l;
		}
	}
	file.close();
	
#ifdef __DEBUG_LANGS_MIMES__
	qDebug("%s %d - Info: loaded mime.types [%s]",
		__FILE__, __LINE__,
		qPrintable(fileName)
	);
#endif
	return true;
}

// private...

/**
 * \brief default constructor
 * 
 * This will constuct a language factory and install a pre defined
 * set of mime types automatically.
 * 
 * You can later on clear the list of mime types and add your own list.
 * 
 * \see clearMimeTypes()
 * \see addMimeTypes( QString )
 */
QsvLangDefFactory::QsvLangDefFactory(void)
{
	// TODO: fix the code to use
	// /usr/share/mime/globs

	addDefaultMimeTypes();
}

/**
 * \brief default destructor
 * 
 * Destructs the language factory.
 */
QsvLangDefFactory::~QsvLangDefFactory(void)
{
}
