#ifndef COLORSMODEL_H
#define COLORSMODEL_H

#include <QAbstractItemModel>
#include <QList>

class QsvColorDefFactory;

class ColorsModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	ColorsModel(QList<QsvColorDefFactory*> *the_colors, QObject *parent = 0 );
	
	QModelIndex index( int row, int col, const QModelIndex &parent = QModelIndex() ) const;
	QModelIndex parent( const QModelIndex & ) const;
	int rowCount( const QModelIndex &parent = QModelIndex() ) const;
	int columnCount( const QModelIndex &parent = QModelIndex() ) const;
	QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
	
private:
	QList<QsvColorDefFactory*> *colors;
};

#endif
