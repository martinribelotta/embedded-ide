#ifndef PROJECTICONPROVIDER_H
#define PROJECTICONPROVIDER_H

#include <QFileIconProvider>

class ProjectIconProvider : public QFileIconProvider
{
public:
    virtual QIcon icon(const QFileInfo &info) const;
};

#endif // PROJECTICONPROVIDER_H
