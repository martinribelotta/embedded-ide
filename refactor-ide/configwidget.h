#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <QDialog>

namespace Ui {
class ConfigWidget;
}

class ConfigWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigWidget(QWidget *parent = nullptr);
    virtual ~ConfigWidget();

public slots:
    void save();
    void load();

protected:
    void showEvent(QShowEvent *) { load(); }

private:
    Ui::ConfigWidget *ui;
};

#endif // CONFIGWIDGET_H
