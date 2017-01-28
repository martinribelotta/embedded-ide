#ifndef MAPVIEWER_H
#define MAPVIEWER_H

#include <QGraphicsView>

class MapViewer : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MapViewer(QWidget *parent = 0);

signals:

public slots:
    bool load(const QString& path);
};

#endif // MAPVIEWER_H
