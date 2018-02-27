#ifndef APPMENU_H
#define APPMENU_H

#include <QWidget>

namespace Ui {
class AppMenu;
}

class QMenu;

class AppMenu : public QWidget
{
    Q_OBJECT

public:
    explicit AppMenu(QWidget *parent = 0);
    ~AppMenu();

    QMenu *menu(QWidget *parent);

public slots:
    QStringList recentProjects() const;
    void setRecentProjects(const QStringList& pathList);

signals:
    void actionActivated();
    void newAction();
    void openAction();
    void closeAction();
    void configAction();
    void aboutAction();

private:
    Ui::AppMenu *ui;
};

#endif // APPMENU_H
