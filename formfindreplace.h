#ifndef FORMFINDREPLACE_H
#define FORMFINDREPLACE_H

#include <QWidget>

namespace Ui {
class FormFindReplace;
}

class FormFindReplace : public QWidget
{
    Q_OBJECT

public:
    explicit FormFindReplace(QWidget *parent = 0);
    ~FormFindReplace();

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::FormFindReplace *ui;
};

#endif // FORMFINDREPLACE_H
