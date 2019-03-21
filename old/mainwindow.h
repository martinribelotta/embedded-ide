#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}
class AppConfig;
class TemplateDownloader;
class QSystemTrayIcon;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void openProject(const QString& makefilePath);

protected:
    virtual void closeEvent(QCloseEvent *e);

private slots:
    void actionNewFromTemplateEnd(const QString& project, const QString& error);

    void actionExportFinish(const QString& s);

    void on_projectView_fileOpen(const QString &);

    void on_projectView_openFindDialog();

    void projectNew();

    void projectOpen();

    void openProject();

    void projectExport();

    void helpShow();

    void projectClose();

    void on_projectView_projectOpened();

    void configureShow();

    void configChanged(AppConfig* config);

    void loggerOpenPath(const QString &path, int col, int row);

    void checkForUpdates();

    bool event(QEvent *event);

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;
    TemplateDownloader *templateDownloader;

    bool goToBuildStage();
    void setUpProxy();
};

#endif // MAINWINDOW_H
