#include "mainmenuwidget.h"
#include "ui_mainmenuwidget.h"

#include <QStandardItemModel>
#include <QDir>

MainMenuWidget::MainMenuWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainMenuWidget)
{
    ui->setupUi(this);
    ui->listView->setModel(model = new QStandardItemModel(this));
    connect(ui->toolButton_newProject, SIGNAL(clicked()), this, SIGNAL(projectNew()));
    connect(ui->toolButton_openProject, SIGNAL(clicked()), this, SIGNAL(projectOpen()));
    connect(ui->toolButton_closeProject, SIGNAL(clicked()), this, SIGNAL(projectClose()));
    connect(ui->toolButton_configure, SIGNAL(clicked()), this, SIGNAL(configure()));
    connect(ui->toolButton_help, SIGNAL(clicked()), this, SIGNAL(help()));
    // connect(ui->toolButton_exit, SIGNAL(clicked()), this, SIGNAL(exit()));
}

MainMenuWidget::~MainMenuWidget()
{
    delete ui;
}

void MainMenuWidget::setProjectList(const QFileInfoList &list)
{
    model->clear();
    m_projectList = list;
    foreach(QFileInfo info, m_projectList) {
        if (info.exists()) {
            QString path = info.path();
            QStringList pathPart = path.split(QDir::separator());
            QString name = pathPart.last();
            QStandardItem *item = new QStandardItem(name);
            item->setToolTip(info.absoluteFilePath());
            item->setData(QVariant::fromValue(info), Qt::UserRole);
            item->setIcon(QIcon("://images/mimetypes/text-x-makefile.svg"));
            model->appendRow(item);
        }
    }
}

void MainMenuWidget::on_listView_activated(const QModelIndex &index)
{
    QStandardItem *item = model->itemFromIndex(index);
    QFileInfo info = item->data(Qt::UserRole).value<QFileInfo>();
    emit projectOpenAs(info);
}
