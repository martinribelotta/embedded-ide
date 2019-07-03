#ifndef BUTTONEDITORITEMDELEGATE_H
#define BUTTONEDITORITEMDELEGATE_H

#include "appconfig.h"

#include <QItemDelegate>
#include <QWidget>
#include <QLineEdit>
#include <QAction>

template<typename Functor>
class ButtonEditorItemDelegate: public QItemDelegate
{
public:
    ButtonEditorItemDelegate(const QString& ict, const Functor& f) : iconToolTip(ict), func(f) {}
    virtual QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        auto w = QItemDelegate::createEditor(parent, option, index);
        QLineEdit* e = qobject_cast<QLineEdit*>(w);
        if (e) {
            auto a = e->addAction(QIcon(AppConfig::resourceImage({ "actions", "document-open" })),
                QLineEdit::TrailingPosition);
            a->setToolTip(iconToolTip);
            connect(a, &QAction::triggered, [index, this]() {
                func(index);
            });
        }
        return w;
    }
private:
    QString iconToolTip;
    Functor func;
};


#endif // BUTTONEDITORITEMDELEGATE_H
