#ifndef FINDLINEEDIT_H
#define FINDLINEEDIT_H

#include <QLineEdit>

class FindLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    FindLineEdit(QWidget *parent = 0l);

    bool isRegularExpression() const { return property("regex").toBool(); }
    bool isCanseSensitive() const { return property("case").toBool(); }
    bool isWoleWords() const { return property("wword").toBool(); }
    bool isSelectionOnly() const { return property("selonly").toBool(); }
};

#endif // FINDLINEEDIT_H
