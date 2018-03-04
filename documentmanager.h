#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include <QWidget>

class IDocumentEditor;

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

signals:
    void documentFocushed(const QString& path);
    void documentNotFound(const QString& path);
    void documentClosed(const QString& path);

public slots:
    void openDocument(const QString& path);
    void closeDocument(const QString& path);
    void closeCurrent() { closeDocument(documentCurrent()); }
    void closeAll();
    void saveDocument(const QString& path);
    void saveDocuments(const QStringList& list) { for(const auto& a: list) saveDocument(a); }
    void saveCurrent() { saveDocument(documentCurrent()); }
    void saveAll();

private:
    class Priv_t;
    Priv_t *priv;
};

#endif // DOCUMENTMANAGER_H
