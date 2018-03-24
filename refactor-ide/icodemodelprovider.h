#ifndef ICPPCODEMODELPROVIDER_H
#define ICPPCODEMODELPROVIDER_H

#include <QObject>
#include <QMimeType>

#include <functional>

class ICodeModelProvider
{
public:
    struct FileReference {
        QString path;
        int line;
        int column;
    };
    typedef QList<FileReference> FileReferenceList;

    typedef std::function<void (const FileReferenceList& ref)> FindReferenceCallback_t;
    typedef std::function<void (const QStringList& completionList)> CompletionCallback_t;

    virtual void declarationOf(const QString& entity, FindReferenceCallback_t cb) = 0;
    virtual void referenceOf(const QString& entity, FindReferenceCallback_t cb) = 0;
    virtual void completionAt(const FileReference& ref, const QString& unsaved, CompletionCallback_t cb) = 0;
};

#endif // ICPPCODEMODELPROVIDER_H
