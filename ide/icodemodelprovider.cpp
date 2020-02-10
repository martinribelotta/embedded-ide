/*
 * This file is part of Embedded-IDE
 * 
 * Copyright 2019 Martin Ribelotta <martinribelotta@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "icodemodelprovider.h"

#include <QUrlQuery>

ICodeModelProvider::FileReference::FileReference(const QString &p, int l, int c, const QString &m):
    path(p), line(l), column(c), meta(m)
{
}

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

bool ICodeModelProvider::FileReference::isEmpty() const
{
    return path.isEmpty() &&
           meta.isEmpty() &&
           line == -1 &&
           column == -1;
}

ICodeModelProvider::FileReference ICodeModelProvider::FileReference::decode(const QUrl &url)
{
    QUrlQuery q(url.query());
    return { url.toLocalFile(), q.queryItemValue("line").toInt(), 0, q.queryItemValue("meta") };
}

bool ICodeModelProvider::FileReference::operator ==(const ICodeModelProvider::FileReference &other) const
{
    return path == other.path && meta == other.meta && line == other.line && column == other.column;
}

ICodeModelProvider::~ICodeModelProvider() {}

QString ICodeModelProvider::Symbol::toString() const
{
    return QString{"%1, %2, %3, %4, %5"}.arg(name, expression, lang, type, ref.encode().toString());
}
