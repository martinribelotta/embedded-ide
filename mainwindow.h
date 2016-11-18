#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void closeEvent(QCloseEvent *e);

private slots:
    void actionNewFromTemplateEnd(const QString& project, const QString& error);

    void actionExportFinish(const QString& s);

    void on_projectView_fileOpen(const QString &);

    void on_actionProjectNew_triggered();

    void on_actionProjectOpen_triggered();

    void openProject();

    void projectOpened();

    void on_actionHelp_triggered();

    void on_actionProjectExport_triggered();

    void on_projectView_startBuild(const QString &target);

    void on_actionProjectClose_triggered();


#if 0
    void on_buildStop_clicked();

    void on_projectView_buildStdout(const QString& text);

    void on_projectView_buildStderr(const QString& text);

    void on_projectView_buildEnd(int status);
#endif
    void on_projectView_projectOpened();

    void on_actionSave_All_triggered();

    void on_actionConfigure_triggered();

    void loggerOpenPath(const QString &path, int col, int row);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
