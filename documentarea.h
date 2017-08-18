#ifndef DOCUMENTAREA_H
#define DOCUMENTAREA_H

#include <QWidget>
#include <QTabWidget>
#include <QTabBar>

class MakefileInfo;
class CodeEditor;
class DebugToolBar;

class QStandardItemModel;

class TabWidget: public QTabWidget {
    Q_OBJECT
public:
    TabWidget(QWidget *parent) :
        QTabWidget(parent) { tabBar()->hide(); }

    void setTabTitle(int idx, const QString& text) {
        setTabText(idx, text);
    }

signals:
    void refresh();

protected:
    void tabInserted(int) Q_DECL_OVERRIDE;
    void tabRemoved(int) Q_DECL_OVERRIDE;
};


class DocumentArea : public QWidget
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
    void setDebugToolBarVisible(bool visible);

protected:
    void windowListUpdate();
    void resizeEvent(QResizeEvent *event);


private slots:
    bool documentToClose(int idx);
    void modifyTab(bool isModify);
    void tabDestroy(QObject *obj);

private:
    int documentFind(const QString& file, QWidget **ww = nullptr);

    TabWidget *tab;
    CodeEditor *lastIpEditor;
    QStandardItemModel *windowListModel;
    DebugToolBar *dbgToolbar;
};

#endif // DOCUMENTAREA_H
