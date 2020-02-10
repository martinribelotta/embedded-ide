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

        FileReference() {}
        FileReference(const QString& p, int l, int c, const QString& m);
        QUrl encode() const;
        bool isEmpty() const;
        static FileReference decode(const QUrl& url);

        bool operator ==(const FileReference& other) const;
    };
    struct Symbol {
        QString name;
        QString expression;
        QString lang;
        QString type;
        FileReference ref;

        bool operator ==(const Symbol& other) const { return toString() == other.toString(); }

        QString toString() const;
    };

    using SymbolList = QList<Symbol>;
    using FileReferenceList = QList<FileReference>;
    using SymbolSet = QSet<ICodeModelProvider::Symbol>;
    using SymbolSetMap = QHash<QString, SymbolSet>;

    using FindReferenceCallback_t = std::function<void (const FileReferenceList& ref)>;
    using CompletionCallback_t = std::function<void (const QStringList& completionList)>;
    using SymbolRequestCallback_t = std::function<void (const SymbolSetMap& completionList)>;

    virtual void startIndexingProject(const QString& path) = 0;
    virtual void startIndexingFile(const QString& path) = 0;

    virtual void referenceOf(const QString& entity, FindReferenceCallback_t cb) = 0;
    virtual void completionAt(const FileReference& ref, const QString& unsaved, CompletionCallback_t cb) = 0;
    virtual void requestSymbolForFile(const QString& path, SymbolRequestCallback_t cb) = 0;
};

Q_DECLARE_METATYPE(ICodeModelProvider::FileReference)
Q_DECLARE_METATYPE(ICodeModelProvider::Symbol)

inline uint qHash(const ICodeModelProvider::FileReference &t, uint seed = 0)
{
    return qHash(t.encode(), seed);
}

inline uint qHash(const ICodeModelProvider::Symbol &t, uint seed = 0)
{
    return qHash(t.toString(), seed);
}

#endif // ICPPCODEMODELPROVIDER_H
