#ifndef MAPVIEWER_H
#define MAPVIEWER_H

#include <QTableView>

class MapViewer : public QTableView
{
    Q_OBJECT
public:
    explicit MapViewer(QWidget *parent = 0);

signals:

public slots:
    bool load(const QString& path);
};

#endif // MAPVIEWER_H
