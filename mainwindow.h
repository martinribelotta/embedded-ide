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

private slots:
    void openProject();

private:
    Ui::MainWindow *ui;
    class Priv_t;
    Priv_t *priv;
};

#endif // MAINWINDOW_H
