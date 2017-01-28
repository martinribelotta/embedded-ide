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
    int fileOpenAt(const QString& file, int row, int col, const MakefileInfo *mk);
    int fileOpen(const QString& file, const MakefileInfo *mk);
    int fileOpenAndSetIP(const QString& file, int line, const MakefileInfo *mk);
    int binOpen(const QString& file);
    int mapOpen(const QString& file);
    void saveAll();
    void closeAll();
    void saveCurrent();
    void reloadCurrent();

private slots:
    void documentToClose(int idx);
    void modifyTab(bool isModify);
    void tabDestroy(QObject *obj);

private:
    int documentFind(const QString& file, CodeEditor **ww = nullptr);

    CodeEditor *lastIpEditor;
};

#endif // DOCUMENTAREA_H
