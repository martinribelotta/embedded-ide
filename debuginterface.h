#ifndef DEBUGINTERFACE_H
#define DEBUGINTERFACE_H

#include <QWidget>

namespace Ui {
class DebugInterface;
}

class DebugInterface : public QWidget
{
    Q_OBJECT

public:
    explicit DebugInterface(QWidget *parent = 0);
    ~DebugInterface();

private:
    Ui::DebugInterface *ui;
};

#endif // DEBUGINTERFACE_H
