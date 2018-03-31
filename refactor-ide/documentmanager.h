#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include <QWidget>

class IDocumentEditor;
class ProjectManager;

class QComboBox;

class DocumentManager : public QWidget
{
    Q_OBJECT
public:
    explicit DocumentManager(QWidget *parent = nullptr);
    virtual ~DocumentManager();

    void setComboBox(QComboBox *cb);

    QStringList unsavedDocuments() const;
    QStringList documents() const;
    int documentCount() const;
    QString documentCurrent() const;
    IDocumentEditor *documentEditor(const QString &path) const;
    IDocumentEditor *documentEditorCurrent() { return documentEditor(documentCurrent()); }

    void setProjectManager(const ProjectManager *projectManager);

signals:
    void documentFocushed(const QString& path);
    void documentNotFound(const QString& path);
    void documentClosed(const QString& path);
    void documentModified(const QString& path, IDocumentEditor *iface, bool modify);

public slots:
    void openDocument(const QString& filePath);
    void openDocumentHere(const QString& path, int line, int col);
    void closeDocument(const QString& filePath);
    void closeCurrent() { closeDocument(documentCurrent()); }
    void closeAll();
    void saveDocument(const QString& path);
    void saveDocuments(const QStringList& list) { for(const auto& a: list) saveDocument(a); }
    void saveCurrent() { saveDocument(documentCurrent()); }
    void saveAll();
    void reloadDocument(const QString& path);
    void reloadDocumentCurrent() { reloadDocument(documentCurrent()); }

protected:
    void focusInEvent(QFocusEvent *event) override;

private:
    class Priv_t;
    Priv_t *priv;
};

#endif // DOCUMENTMANAGER_H
