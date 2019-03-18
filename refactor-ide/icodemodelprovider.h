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
        int line;
        int column;
        QString meta;

        QUrl encode() const;

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
