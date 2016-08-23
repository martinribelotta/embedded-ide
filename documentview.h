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

class DocumentView : public QWidget
{
    Q_OBJECT

public:
    explicit DocumentView(QWidget *parent = 0);
    ~DocumentView();

    QString project() const;
    QDir projectPath() const;
    const MakefileInfo &makeInfo() const { return mk_info; }
    const ETags &tags() const { return mk_info.tags; }

public slots:
    void closeProject();
    void openProject(const QString& projectFile);
    QString makeTemplate(const QString& diffFile);
    void buildStart(const QString& target);
    void buildStop();
    void setDebugOn(bool on);
    void setETags(ETags &tags);

private slots:
    void on_treeView_activated(const QModelIndex &index);
    void updateMakefileInfo(const MakefileInfo &info);

    void on_targetList_doubleClicked(const QModelIndex &index);

    void on_buildProc_readyReadStandardError();
    void on_buildProc_readyReadStandardOutput();

    void on_filterCombo_activated(int idx);

    void on_filterButton_clicked();

    void on_toolButton_documentNew_clicked();

    void on_toolButton_folderNew_clicked();

    void on_toolButton_elementDel_clicked();

signals:
    void projectOpened();
    void fileOpen(const QString& file);
    void startBuild(const QString& target);
    void buildStdout(const QString& text);
    void buildStderr(const QString& text);
    void buildEnd(int status);

private:
    Ui::DocumentView *ui;
    QProcess *buildProc;
    MakefileInfo mk_info;
};

#endif // DOCUMENTVIEW_H
