#ifndef DEBUGTOOLBAR_H
#define DEBUGTOOLBAR_H

#include <QFrame>

class QShortcut;

namespace Ui {
class DebugToolBar;
}

class DebugToolBar : public QFrame
{
    Q_OBJECT

public:
    explicit DebugToolBar(QWidget *parent = 0);
    ~DebugToolBar();

signals:
    void debugStepOver();
    void debugStepInto();
    void debugStepOut();

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

private:
    Ui::DebugToolBar *ui;
    QList<QShortcut*> shortcuts;
};

#endif // DEBUGTOOLBAR_H
