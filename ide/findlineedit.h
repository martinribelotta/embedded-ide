#ifndef FINDLINEEDIT_H
#define FINDLINEEDIT_H

#include <QLineEdit>

class QToolButton;

class FindLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    FindLineEdit(QWidget *parent = 0l);

    void addMenuActions(const QHash<QString, QString>& actionList);

    void setPropertyChecked(const QString &propertyName, bool state);
    bool isPropertyChecked(const char *name) const { return property(name).toBool(); }

signals:
    void menuActionClicked(const QString& prop, bool status);

private:
    QToolButton *optionsButton;
    QHash<QString, QAction*> actionMap;
};

#endif // FINDLINEEDIT_H
