#ifndef DOCUMENTAREA_H
#define DOCUMENTAREA_H

#include <QWidget>

class MakefileInfo;
class CodeEditor;

class QStandardItemModel;
class QTabWidget;

class DocumentArea : public QWidget
{
    Q_OBJECT
public:
    explicit DocumentArea(QWidget *parent = 0);

    QList<CodeEditor*> documentsDirty() const;
    bool hasUnsavedChanges();

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
    void closeCurrent();

protected:
    void windowListUpdate();

private slots:
    bool documentToClose(int idx);
    void modifyTab(bool isModify);
    void tabDestroy(QObject *obj);

private:
    int documentFind(const QString& file, CodeEditor **ww = nullptr);

    QTabWidget *tab;
    CodeEditor *lastIpEditor;
    QStandardItemModel *windowListModel;
};

#endif // DOCUMENTAREA_H
