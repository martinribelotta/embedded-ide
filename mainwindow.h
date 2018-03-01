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
    virtual ~MainWindow();

public slots:
    void openProject(const QString& path);

private:
    Ui::MainWindow *ui;
    class Priv_t;
    Priv_t *priv;
};

#endif // MAINWINDOW_H
