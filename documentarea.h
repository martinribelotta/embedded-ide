#ifndef DOCUMENTAREA_H
#define DOCUMENTAREA_H

#include <QTabWidget>

class MakefileInfo;

class DocumentArea : public QTabWidget
{
    Q_OBJECT
public:
    explicit DocumentArea(QWidget *parent = 0);

signals:

public slots:
    bool fileOpen(const QString& file, int row, int col, const MakefileInfo *mk);
    void saveAll();

private slots:
    void documentToClose(int idx);
    void closeAll();
    void saveCurrent();
    void modifyTab(bool isModify);

private:
    int documentFind(const QString& file);
};

#endif // DOCUMENTAREA_H
