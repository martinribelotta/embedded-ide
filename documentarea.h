#ifndef DOCUMENTAREA_H
#define DOCUMENTAREA_H

#include <QTabWidget>

class MakefileInfo;
class CodeEditor;

class DocumentArea : public QTabWidget
{
    Q_OBJECT
public:
    explicit DocumentArea(QWidget *parent = 0);

signals:

public slots:
    bool fileOpen(const QString& file, int row, int col, const MakefileInfo *mk);
    bool binOpen(const QString& file);
    void saveAll();

private slots:
    void documentToClose(int idx);
    void closeAll();
    void saveCurrent();
    void reloadCurrent();
    void modifyTab(bool isModify);
    void tabDestroy(QObject *obj);

private:
    int documentFind(const QString& file, CodeEditor **ww = nullptr);
};

#endif // DOCUMENTAREA_H
