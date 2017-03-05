#ifndef DOCUMENTAREA_H
#define DOCUMENTAREA_H

#include <QWidget>

class QTabWidget;
class MakefileInfo;
class CodeEditor;

class DocumentArea : public QWidget
{
    Q_OBJECT
public:
    explicit DocumentArea(QWidget *parent = 0);

    QList<CodeEditor*> documentsDirty() const;

signals:

public slots:
    int fileOpenAt(const QString& file, int row, int col, const MakefileInfo *mk);
    int fileOpen(const QString& file, const MakefileInfo *mk);
    void clearIp();
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
    QTabWidget *tab;
};

#endif // DOCUMENTAREA_H
