#ifndef DOCUMENTAREA_H
#define DOCUMENTAREA_H

#include <QTabWidget>

class DocumentArea : public QTabWidget
{
    Q_OBJECT
public:
    explicit DocumentArea(QWidget *parent = 0);

signals:

public slots:
    bool fileOpen(const QString& file);
    void saveAll();

private slots:
    void documentToClose(int idx);

private:
    int documentFind(const QString& file);
};

#endif // DOCUMENTAREA_H
