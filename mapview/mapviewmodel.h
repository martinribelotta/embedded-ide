#ifndef MAPVIEWMODEL_H
#define MAPVIEWMODEL_H

#include <QObject>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

class BarItemDelegate: public QStyledItemDelegate {
public:
    BarItemDelegate(QObject *parent = Q_NULLPTR);
    ~BarItemDelegate() override;

    QStyleOptionProgressBar options(const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const;

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

class MapViewModel : public QStandardItemModel
{
    Q_OBJECT
public:
    static const int PERCENT_COLUMN;

    explicit MapViewModel(QObject *parent = nullptr);
    virtual ~MapViewModel();

public slots:
    bool load(const QString& path);
};

#endif // MAPVIEWMODEL_H
