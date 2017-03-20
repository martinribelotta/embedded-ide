#ifndef FINDLINEEDIT_H
#define FINDLINEEDIT_H

#include <QLineEdit>

class QToolButton;

class FindLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    FindLineEdit(QWidget *parent = 0l);

    void addMenuAction(const QHash<QString, QString>& actionList);

    bool isPropertyChecked(const QString& name) const;

private:
    QToolButton *optionsButton;
};

#endif // FINDLINEEDIT_H
