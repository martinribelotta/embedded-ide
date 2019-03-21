#ifndef BANNERWIDGET_H
#define BANNERWIDGET_H

#include <QWidget>

namespace Ui {
class BannerWidget;
}

class BannerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BannerWidget(QWidget *parent = 0);
    ~BannerWidget();

private:
    Ui::BannerWidget *ui;
};

#endif // BANNERWIDGET_H
