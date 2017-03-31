#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}
class AppConfig;
class QSystemTrayIcon;

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

    void projectNew();

    void projectOpen();

    void openProject();

    void projectOpened();

    void helpShow();

    void projectClose();

    void on_projectView_projectOpened();

    void on_actionSave_All_triggered();

    void configureShow();

    void configChanged(AppConfig* config);

    void loggerOpenPath(const QString &path, int col, int row);

    void checkForUpdates();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;

    bool goToBuildStage();
    void setUpProxy();
};

#endif // MAINWINDOW_H
