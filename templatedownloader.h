#ifndef TEMPLATEDOWNLOADER_H
#define TEMPLATEDOWNLOADER_H

#include <vector>

#include "templatesdownloadselector.h"

class TemplateDownloader : public QObject
{
    Q_OBJECT
public:
    TemplateDownloader();

public slots:
    void requestPendantDownloads();
    void download();

signals:
    void newUpdatesAvailables();

private:
    void downloadMetadata();

    std::vector<Template> tmpls_;
};

#endif // TEMPLATEDOWNLOADER_H
