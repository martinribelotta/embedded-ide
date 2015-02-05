#ifndef __QSV_LANGDEF_FACTORY_H__
#define __QSV_LANGDEF_FACTORY_H__

/**
 * \file qsvlangdeffactory.h
 * \brief Definition of the language definition factory
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see QsvLangDefFactory
 */

#include <QMap>

class QsvLangDef;
class QString;

class QsvLangDefFactory
{
public:
	static QsvLangDefFactory *getInstanse();
	QsvLangDef*	getHighlight( QString fileName );
	void		loadDirectory( QString directory );
	void		clearMimeTypes();
	bool		addDefaultMimeTypes();
	bool		addMimeTypes( QString fileName );

private:	
	QsvLangDefFactory(void);
	~QsvLangDefFactory(void);
	
	static	QsvLangDefFactory	*LangFactory;
	QList<QsvLangDef*>		langList;
	QMap<QString,QStringList>	mimeTypes;
};

#endif // __QSV_LANGDEF_FACTORY_H__
