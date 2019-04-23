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

ICodeModelProvider::~ICodeModelProvider()
= default;
