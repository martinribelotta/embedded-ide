#ifndef DOCUMENTAREA_H
#define DOCUMENTAREA_H

#include "combodocumentview.h"

#include <QMutableLinkedListIterator>

class MakefileInfo;
class CodeEditor;
class ProjectView;

class QLabel;
class QUndoStack;

class DocumentArea : public ComboDocumentView
{
    Q_OBJECT
public:
    explicit DocumentArea(QWidget *parent = 0);
    virtual ~DocumentArea();

    QList<CodeEditor*> documentsDirty() const;
    bool hasUnsavedChanges();
    void setProjectView(ProjectView *pView) { this->pView = pView; }

signals:

public slots:
    int fileOpen(const QString& file, int row=-1, int col=-1);
    void clearIp();
    int fileOpenAndSetIP(const QString& file, int line);
    int binOpen(const QString& file);
    int mapOpen(const QString& file);
    void saveAll();
    void closeAll();
    void saveCurrent();
    void reloadCurrent();
    void closeCurrent();
    void setTopBarHeight(int h);

protected:
    virtual void resizeEvent(QResizeEvent *e) override;

private slots:
    bool documentToClose(int idx);
    void modifyTab(bool isModify);
    void tabDestroy(QObject *obj);

private:
    int documentFind(const QString& file, QWidget **ww = nullptr);
    CodeEditor *lastIpEditor;
    QWidget *banner;
    ProjectView *pView;
};

#endif // DOCUMENTAREA_H
