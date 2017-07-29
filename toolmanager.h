#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include <QDialog>

#include "projectview.h"

namespace Ui {
class ToolManager;
}

class QStandardItemModel;

class ToolManager : public QDialog
{
    Q_OBJECT

public:
    explicit ToolManager(QWidget *parent = 0);
    ~ToolManager();

    void setTools(const ProjectView::EntryList_t &toolList);

private slots:
    void on_toolButton_add_clicked();

    void on_toolButton_del_clicked();

    void on_ToolManager_accepted();

    void on_toolButton_itemUp_clicked();

    void on_toolButton_itemDown_clicked();

    void on_toolButton_fastHelp_clicked();

private:
    Ui::ToolManager *ui;
    QStandardItemModel *model;
};

#endif // TOOLMANAGER_H
