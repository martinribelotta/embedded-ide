#include "projecticonprovider.h"

#include <QMimeDatabase>

QIcon ProjectIconProvider::icon(const QFileInfo &info) const
{
    auto t = QMimeDatabase().mimeTypeForFile(info);
    if (t.isValid()) {
        auto resName = QString(":/images/mimetypes/%1.svg").arg(t.iconName());
        if (QFile(resName).exists()) {
            return QIcon(resName);
        }
        resName = QString(":/images/mimetypes/%1.svg").arg(t.genericIconName());
        if (QFile(resName).exists()) {
            return QIcon(resName);
        }
    }
    return QFileIconProvider::icon(info);
}
