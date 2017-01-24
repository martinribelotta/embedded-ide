#ifndef DOCUMENTVIEW_H
#define DOCUMENTVIEW_H

#include <QWidget>
#include <QModelIndex>
#include <QDir>
#include <QFileInfo>

#include "makefileinfo.h"
#include "etags.h"

namespace Ui {
class DocumentView;
}

class QProcess;
class DebugInterface;
class TagList;

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

public slots:
    void closeProject();
    void openProject(const QString& projectFile);

#if 0
    void buildStart(const QString& target);
    void buildStop();
#endif
    void setDebugOn(bool on);

private slots:
    void on_treeView_activated(const QModelIndex &index);
    void updateMakefileInfo(const MakefileInfo &info);

    void on_targetList_doubleClicked(const QModelIndex &index);

#if 0
    void on_buildProc_readyReadStandardError();
    void on_buildProc_readyReadStandardOutput();
#endif
    void on_filterCombo_activated(int idx);

    void on_filterButton_clicked();

    void on_toolButton_documentNew_clicked();

    void on_toolButton_folderNew_clicked();

    void on_toolButton_elementDel_clicked();

signals:
    void projectOpened();
    void fileOpen(const QString& file);
    void startBuild(const QString& target);
#if 0
    void buildStdout(const QString& text);
    void buildStderr(const QString& text);
    void buildEnd(int status);
#endif

private:
    Ui::DocumentView *ui;
#if 0
    QProcess *buildProc;
#endif
    MakefileInfo mk_info;
    TagList *tagList;
};

#endif // DOCUMENTVIEW_H
