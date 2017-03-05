#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void closeEvent(QCloseEvent *e);

private slots:

    void openProject();

    void on_projectView_fileOpen(const QString &);

    void on_projectView_projectOpened();

    void actionExportFinish(const QString& s);

#if 0
    void actionNewFromTemplateEnd(const QString& project, const QString& error);

    void actionProjectNew_triggered();

    void actionProjectOpen_triggered();

    void actionHelp_triggered();

    void actionProjectExport_triggered();

    void actionProjectClose_triggered();

    void actionSave_All_triggered();

    void actionConfigure_triggered();

#endif
    void loggerOpenPath(const QString &path, int col, int row);
    void on_toolButton_projectExport_clicked();

    void on_toolButton_newProject_clicked();

    void on_toolButton_projectOpen_clicked();

    void on_toolButton_projectClose_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
