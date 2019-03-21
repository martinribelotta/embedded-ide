#ifndef TAGLIST_H
#define TAGLIST_H

#include <QListWidget>
#include <QList>

#include "etags.h"

class TagList : public QListWidget
{
public:
    TagList(QWidget *parent = 0l);
    void setTagList(const QList<ETags::Tag> &tags);

    ETags::Tag selectedTag() const;
};

#endif // TAGLIST_H
