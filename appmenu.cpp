#include "appmenu.h"
#include "ui_appmenu.h"

#include <QFileInfo>
#include <QDir>
#include <QMenu>
#include <QStandardItemModel>
#include <QWidgetAction>

AppMenu::AppMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AppMenu)
{
    ui->setupUi(this);
    ui->recentProjectList->setModel(new QStandardItemModel(this));
    for(auto a: findChildren<QToolButton*>())
        connect(a, &QToolButton::clicked, this, &AppMenu::actionActivated);
    connect(ui->recentProjectList, &QListView::activated, this, &AppMenu::actionActivated);
    connect(ui->buttonNew, &QToolButton::clicked, this, &AppMenu::newAction);
    connect(ui->buttonOpen, &QToolButton::clicked, this, &AppMenu::openAction);
    connect(ui->buttonClose, &QToolButton::clicked, this, &AppMenu::closeAction);
    connect(ui->buttonConfig, &QToolButton::clicked, this, &AppMenu::configAction);
    connect(ui->buttonAbout, &QToolButton::clicked, this, &AppMenu::aboutAction);
}

AppMenu::~AppMenu()
{
    delete ui;
}

QMenu *AppMenu::menu(QWidget *parent)
{
    auto menu = new QMenu(parent);
    auto widgetAction = new QWidgetAction(menu);
    widgetAction->setDefaultWidget(this);
    menu->addAction(widgetAction);
    connect(this, &AppMenu::actionActivated, menu, &QMenu::close);
    return menu;
}

QStringList AppMenu::recentProjects() const
{
    QStringList items;
    auto model = qobject_cast<QStandardItemModel*>(ui->recentProjectList->model());
    for(int row = 0; row < model->rowCount(); row++) {
        auto item = model->item(row);
        items << item->data().toString();
    }
    return items;
}

void AppMenu::setRecentProjects(const QStringList &pathList)
{
    auto icon = QIcon(":/images/mimetypes/text-x-makefile.svg");
    auto model = qobject_cast<QStandardItemModel*>(ui->recentProjectList->model());
    for(auto& path: pathList) {
        auto info = QFileInfo(path);
        auto item = new QStandardItem(icon, info.absoluteDir().dirName());
        item->setData(info.absoluteFilePath());
        item->setToolTip(info.absolutePath());
        model->appendRow({ item });
    }
}
