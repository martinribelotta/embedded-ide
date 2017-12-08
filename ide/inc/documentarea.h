#ifndef DOCUMENTAREA_H
#define DOCUMENTAREA_H

#include "combodocumentview.h"

class MakefileInfo;
class CodeEditor;

class QLabel;

class DocumentArea : public ComboDocumentView
{
    Q_OBJECT
public:
    explicit DocumentArea(QWidget *parent = 0);
    virtual ~DocumentArea();

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
    virtual void resizeEvent(QResizeEvent *e) override;

private slots:
    bool documentToClose(int idx);
    void modifyTab(bool isModify);
    void tabDestroy(QObject *obj);

private:
    int documentFind(const QString& file, QWidget **ww = nullptr);
    CodeEditor *lastIpEditor;
    QLabel *banner;
};

#endif // DOCUMENTAREA_H
