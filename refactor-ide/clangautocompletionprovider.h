#ifndef CLANGAUTOCOMPLETIONPROVIDER_H
#define CLANGAUTOCOMPLETIONPROVIDER_H

#include <QObject>
#include <icodemodelprovider.h>

class ProcessManager;

class ClangAutocompletionProvider: public QObject, public ICodeModelProvider
{
    Q_OBJECT
public:
    explicit ClangAutocompletionProvider(ProcessManager *_pman, QObject *parent);
    virtual ~ClangAutocompletionProvider();

    void startIndexingProject(const QString& path, const QHash<QString, QString>& targetMap) override;
    void startIndexingFile(const QString& path) override;

    void declarationOf(const QString& entity, FindReferenceCallback_t cb) override;
    void referenceOf(const QString& entity, FindReferenceCallback_t cb) override;
    void completionAt(const FileReference& ref, const QString& unsaved, CompletionCallback_t cb) override;

private:
    ProcessManager *pman;
};

#endif // CLANGAUTOCOMPLETIONPROVIDER_H
