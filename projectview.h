#ifndef DOCUMENTVIEW_H
#define DOCUMENTVIEW_H

#include <QWidget>
#include <QModelIndex>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemModel>

#include "makefileinfo.h"
#include "etags.h"

namespace Ui {
    class ProjectView;
}

class QLabel;
class QToolButton;
class QMenu;
class QProcess;
class TagList;

class MyFileSystemModel: public QFileSystemModel
{
    Q_OBJECT
public:
    MyFileSystemModel(QObject *parent = 0l);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QStringList sectionName;
};

class ProjectView : public QWidget
{
    Q_OBJECT

public:
    typedef QPair<QString, QVariant> Entry_t;
    typedef QList<Entry_t> EntryList_t;

    explicit ProjectView(QWidget *parent = 0);
    ~ProjectView();

    QString project() const;
    QString projectName() const;
    QDir projectPath() const;
    const MakefileInfo &makeInfo() const { return mk_info; }
    const ETags &tags() const { return mk_info.tags; }
    void setMainMenu(QMenu *m);

public slots:
    void closeProject();
    void openProject(const QString& projectFile);
    void setTargetsViewOn(bool on);
    void debugStarted();
    void debugStoped();
    void doTarget(const QString& target) {
        emit startBuild(target);
    }

private slots:
    void on_treeView_activated(const QModelIndex &index);

    void updateMakefileInfo(const MakefileInfo &info);

    void on_targetList_clicked(const QModelIndex &index);

    void onDocumentNew();

    void onFolderNew();

    void onLinkNew();

    void onElementDel();

    void on_toolButton_export_clicked();

    void toolAction();

    void fileProperties(const QFileInfo& info);

    void on_treeView_pressed(const QModelIndex &index);

    void on_toolButton_find_clicked();

    void on_toolButton_startDebug_clicked();

    void on_treeView_customContextMenuRequested(const QPoint &pos);

signals:
    void projectOpened();
    void fileOpen(const QString& file);
    void startBuild(const QString& target);
    void execTool(const QString& command);
    void openFindDialog();
    void debugChange(bool enable);

private:
    Ui::ProjectView *ui;
    MakefileInfo mk_info;
    TagList *tagList;
    QList<QToolButton*> projectButtons;
    QLabel *labelStatus;

    QMenu *createExternalToolsMenu();
};

#endif // DOCUMENTVIEW_H
