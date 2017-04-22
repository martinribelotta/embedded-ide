#include "taglist.h"

TagList::TagList(QWidget *parent) : QListWidget (parent)
{
    setAlternatingRowColors(true);
}

void TagList::setTagList(const QList<ETags::Tag> &tags)
{
    foreach(ETags::Tag t, tags) {
        auto item = new QListWidgetItem(this);
        item->setText(QString("%2 (%3):\n%1")
                      .arg(t.decl)
                      .arg(t.file)
                      .arg(t.line));
        item->setData(Qt::UserRole, QVariant::fromValue(t));
    }
    setMinimumWidth(sizeHintForColumn(0) + 2 + frameWidth());
}

ETags::Tag TagList::selectedTag() const
{
    QList<QListWidgetItem*> sel = selectedItems();
    if (sel.count() > 0) {
        return qvariant_cast<ETags::Tag>(sel.at(0)->data(Qt::UserRole));
    } else {
        return ETags::Tag();
    }
}
