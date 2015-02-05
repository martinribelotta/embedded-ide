#include <QVariant>
#include <QModelIndex>

#include "colorsmodel.h"
#include "qsvcolordeffactory.h"

ColorsModel::ColorsModel( QList<QsvColorDefFactory*> *the_colors, QObject *parent )
	: QAbstractItemModel(parent)
{
	colors = the_colors;
}

QModelIndex ColorsModel::index( int row, int col, const QModelIndex &parent ) const
{
	return createIndex( row, col, 0 );
	
	Q_UNUSED( parent );
}

QModelIndex ColorsModel::parent( const QModelIndex & ) const
{
	return QModelIndex();
}

int ColorsModel::rowCount( const QModelIndex &parent ) const
{
	int i;
	if (colors == NULL)
		i = 0;
	else
		i = colors->count();
	
	return i;

	Q_UNUSED( parent );
}

int ColorsModel::columnCount( const QModelIndex &parent ) const
{
	return 1;
	Q_UNUSED( parent );
}

QVariant ColorsModel::data( const QModelIndex &index, int role ) const
{
	if (!colors)
		return QVariant();
		
	if (!index.isValid())
		return QVariant();

	if (index.row() >= colors->count())
		return QVariant();

	QsvColorDefFactory *c = colors->at(index.row());
	if (!c)
		return QVariant();
	
	if (role == Qt::DisplayRole)
		return c->name;
	else if (role == Qt::StatusTipRole)
		return c->fileName;
	else if (role == Qt::ToolTipRole)
		return c->fileName;
	else
		return QVariant();
}
