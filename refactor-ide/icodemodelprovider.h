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

    virtual bool canHandleMimeType(const QMimeType& type) = 0;
    virtual bool canHandleSuffix(const QString& suffix) = 0;
};

class CodeModelFactory : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(CodeModelFactory)

private:
    explicit CodeModelFactory();
    virtual ~CodeModelFactory();

public:
    static CodeModelFactory &instance();

    void registerProvider(ICodeModelProvider *iface) { providerList.append(iface); }

    ICodeModelProvider *modelForSuffix(const QString& suffix);
    ICodeModelProvider *modelForMime(const QMimeType& type);
    ICodeModelProvider *modelForFile(const QString& filePath);

private:
    QList<ICodeModelProvider*> providerList;
};

#endif // ICPPCODEMODELPROVIDER_H
