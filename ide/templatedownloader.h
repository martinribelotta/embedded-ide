#ifndef TEMPLATEDOWNLOADER_H
#define TEMPLATEDOWNLOADER_H

#include <vector>

#include "templatesdownloadselector.h"

class TemplateDownloader : public QObject
{
    Q_OBJECT
public:
    TemplateDownloader();

    bool isSilent() const { return silent_; }
    void setSilent(bool en) { silent_ = en; }

public slots:
    void requestPendantDownloads();
    void download();

signals:
    void newUpdatesAvailables();
    void finished(bool ok);

private:
    void downloadMetadata();

    std::vector<Template> tmpls_;
    bool silent_;
};

#endif // TEMPLATEDOWNLOADER_H
