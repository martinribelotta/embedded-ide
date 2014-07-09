#ifndef DOCUMENTVIEW_H
#define DOCUMENTVIEW_H

#include <QWidget>
#include <QModelIndex>

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

public slots:
    void closeProject();
    void openProject(const QString& projectFile);
    QString newTemplateProject(const QString &projectPath, const QString& patchText);
    QString makeTemplate(const QString& diffFile);
    void buildStart(const QString& target);
    void buildStop();

private slots:
    void on_treeView_activated(const QModelIndex &index);
    void updateTargets();

    void on_targetList_doubleClicked(const QModelIndex &index);

    void on_buildProc_readyReadStandardError();
    void on_buildProc_readyReadStandardOutput();

signals:
    void fileOpen(const QString& file);
    void startBuild(const QString& target);
    void buildStdout(const QString& text);
    void buildStderr(const QString& text);
    void buildEnd(int status);

private:
    Ui::DocumentView *ui;
    QProcess *buildProc;
};

#endif // DOCUMENTVIEW_H
