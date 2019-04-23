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
#ifndef ICPPCODEMODELPROVIDER_H
#define ICPPCODEMODELPROVIDER_H

#include <QObject>

#include <QMimeType>

#include <QUrl>
#include <functional>

class ICodeModelProvider
{
public:
    virtual ~ICodeModelProvider();
    struct FileReference {
        QString path;
        int line = -1;
        int column = -1;
        QString meta;

        FileReference(const QString& p, int l, int c, const QString& m):  path(p),
                                                                          line(l),
                                                                          column(c),
                                                                          meta(m) {}

        QUrl encode() const;

        bool isEmpty() const { return path.isEmpty() && meta.isEmpty() && line == -1 && column == -1; }

        static FileReference decode(const QUrl& url);
    };
    typedef QList<FileReference> FileReferenceList;

    typedef std::function<void (const FileReferenceList& ref)> FindReferenceCallback_t;
    typedef std::function<void (const QStringList& completionList)> CompletionCallback_t;

    virtual void startIndexingProject(const QString& path) = 0;
    virtual void startIndexingFile(const QString& path) = 0;

    virtual void referenceOf(const QString& entity, FindReferenceCallback_t cb) = 0;
    virtual void completionAt(const FileReference& ref, const QString& unsaved, CompletionCallback_t cb) = 0;
};

#endif // ICPPCODEMODELPROVIDER_H
