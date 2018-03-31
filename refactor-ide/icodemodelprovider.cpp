#include "icodemodelprovider.h"

#include <QUrlQuery>

QUrl ICodeModelProvider::FileReference::encode() const
{
    auto url = QUrl::fromLocalFile(path);
    QUrlQuery q;
    q.addQueryItem("line", QString("%1").arg(line));
    q.addQueryItem("column", QString("%1").arg(column));
    q.addQueryItem("meta", meta);
    url.setQuery(q);
    return url;
}

ICodeModelProvider::FileReference ICodeModelProvider::FileReference::decode(const QUrl &url)
{
    QUrlQuery q(url.query());
    return { url.toLocalFile(), q.queryItemValue("line").toInt(), 0, q.queryItemValue("meta") };
}
