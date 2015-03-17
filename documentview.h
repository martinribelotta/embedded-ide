#ifndef DOCUMENTVIEW_H
#define DOCUMENTVIEW_H

#include <QWidget>
#include <QModelIndex>
#include <QDir>
#include <QFileInfo>

#include "makefileinfo.h"

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

public slots:
    void closeProject();
    void openProject(const QString& projectFile);
    QString makeTemplate(const QString& diffFile);
    void buildStart(const QString& target);
    void buildStop();

private slots:
    void on_treeView_activated(const QModelIndex &index);
    void updateMakefileInfo(const MakefileInfo &info);

    void on_targetList_doubleClicked(const QModelIndex &index);

    void on_buildProc_readyReadStandardError();
    void on_buildProc_readyReadStandardOutput();

    void on_filterCombo_activated(int idx);

    void on_filterButton_clicked();

signals:
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
