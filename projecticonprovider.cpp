#include "projecticonprovider.h"

#include <QMimeDatabase>

#include <QtDebug>

ProjectIconProvider::ProjectIconProvider(QObject *parent) : QObject(parent)
{
    db = new QMimeDatabase();
}

ProjectIconProvider::~ProjectIconProvider()
{
    delete db;
}

class StaticMimeHelper {
public:
    QIcon icon(const QMimeType& m) const {
        QString resName = QString(":/icon-theme/icon-theme/%1.png").arg(m.iconName());
        if (QFile(resName).exists())
            return QIcon(resName);
        else
            return QIcon();
    }
};

static StaticMimeHelper staticIconDb;

QIcon ProjectIconProvider::icon(const QFileInfo &info) const
{
    QMimeType t = db->mimeTypeForFile(info);
    if (t.isValid()) {
        // qDebug() << "Required mime for" << info.fileName() << t << t.iconName();
        QIcon c = QIcon::fromTheme(t.iconName(), staticIconDb.icon(t));
        if (!c.isNull())
            return c;
    }
    return QFileIconProvider::icon(info);
}
