#ifndef MAINMENUWIDGET_H
#define MAINMENUWIDGET_H

#include <QWidget>
#include <QFileInfo>
#include <QList>

class QStandardItemModel;

namespace Ui {
class MainMenuWidget;
}

class MainMenuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainMenuWidget(QWidget *parent = 0);
    ~MainMenuWidget();

    void setProjectList(const QFileInfoList& list);
    QFileInfoList projectList() const { return m_projectList; }

signals:
    void projectNew();
    void projectOpen();
    void projectOpenAs(const QFileInfo& path);
    void projectClose();
    void configure();
    void help();
    void exit();

private slots:
    void on_listView_activated(const QModelIndex &index);

private:
    Ui::MainMenuWidget *ui;
    QStandardItemModel *model;
    QFileInfoList m_projectList;
};

#endif // MAINMENUWIDGET_H
