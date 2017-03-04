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
class DocumentView;
}

class QToolButton;
class QMenu;
class QProcess;
class DebugInterface;
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
    explicit ProjectView(QWidget *parent = 0);
    ~ProjectView();

    QString project() const;
    QDir projectPath() const;
    const MakefileInfo &makeInfo() const { return mk_info; }
    const ETags &tags() const { return mk_info.tags; }
    DebugInterface *getDebugInterface() const;
    void setMainMenu(QMenu *m);

public slots:
    void closeProject();
    void openProject(const QString& projectFile);
    void setDebugOn(bool on);

private slots:
    void on_treeView_activated(const QModelIndex &index);
    void updateMakefileInfo(const MakefileInfo &info);

    void on_targetList_doubleClicked(const QModelIndex &index);

    void on_filterCombo_activated(int idx);

    void on_filterButton_clicked();

    void on_toolButton_documentNew_clicked();

    void on_toolButton_folderNew_clicked();

    void on_toolButton_elementDel_clicked();

    void on_toolButton_export_clicked();

signals:
    void projectOpened();
    void fileOpen(const QString& file);
    void startBuild(const QString& target);

private:
    Ui::DocumentView *ui;
    MakefileInfo mk_info;
    TagList *tagList;
    QList<QToolButton*> projectButtons;
};

#endif // DOCUMENTVIEW_H
