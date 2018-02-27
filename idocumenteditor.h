#ifndef IDOCUMENTEDITOR_H
#define IDOCUMENTEDITOR_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QPoint>
#include <QMimeType>

class IDocumentEditor
{
public:
    virtual QWidget *widget() = 0;
    virtual bool load(const QString& path) = 0;
    virtual bool save(const QString& path) = 0;
    virtual QString path() const = 0;
    virtual bool isReadonly() const = 0;
    virtual void setReadonly(bool rdOnly) = 0;
    virtual bool isModified() const = 0;
    virtual void setModified(bool m) = 0;
    virtual QPoint cursor() const = 0;
    virtual void setCursor(const QPoint& pos) = 0;
};

class IDocumentEditorCreator
{
public:
    virtual bool canHandleMime(const QMimeType& mime) const = 0;
    virtual IDocumentEditor *create(QWidget *parent = nullptr) const = 0;
};

class DocumentEditorFactory: public QObject
{
    Q_OBJECT

private:
    DocumentEditorFactory();
    QList<IDocumentEditorCreator*> creators;

public:
    static DocumentEditorFactory* instance();

    void registerDocumentInterface(IDocumentEditorCreator *creator);
    IDocumentEditor *create(const QString& path, QWidget *parent = nullptr);
};

#endif // IDOCUMENTEDITOR_H
