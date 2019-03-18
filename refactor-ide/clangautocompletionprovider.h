#ifndef CLANGAUTOCOMPLETIONPROVIDER_H
#define CLANGAUTOCOMPLETIONPROVIDER_H

#include <QObject>
#include <icodemodelprovider.h>

class ProjectManager;

class ClangAutocompletionProvider: public QObject, public ICodeModelProvider
{
    Q_OBJECT
public:
    explicit ClangAutocompletionProvider(ProjectManager *proj, QObject *parent);
    virtual ~ClangAutocompletionProvider() override;

    void startIndexingProject(const QString& path) override;
    void startIndexingFile(const QString& path) override;

    void referenceOf(const QString& entity, FindReferenceCallback_t cb) override;
    void completionAt(const FileReference& ref, const QString& unsaved, CompletionCallback_t cb) override;

private:
    class Priv_t;
    Priv_t *priv;
};

#endif // CLANGAUTOCOMPLETIONPROVIDER_H
