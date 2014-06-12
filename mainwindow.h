#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class BuildManager;

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
    virtual QMenu *createPopupMenu();

private slots:
    void on_centralWidget_tabCloseRequested(int n);

    void on_actionNew_triggered();

    void on_actionOpen_triggered();

    void on_actionExit_triggered();

    void on_actionSave_triggered();

    void on_actionBuild_triggered();

    void on_actionConfigure_triggered();

    void on_textBrowser_customContextMenuRequested(const QPoint &pos);

    void toggle_lockWidgets(bool lock);
private:
    Ui::MainWindow *ui;
    BuildManager *runner;
};

#endif // MAINWINDOW_H
