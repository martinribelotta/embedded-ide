#include "bannerwidget.h"
#include "ui_bannerwidget.h"

BannerWidget::BannerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BannerWidget)
{
    ui->setupUi(this);
}

BannerWidget::~BannerWidget()
{
    delete ui;
}
