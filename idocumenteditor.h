#ifndef IDOCUMENTEDITOR_H
#define IDOCUMENTEDITOR_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QPoint>
#include <QMimeType>

#include <functional>

class IDocumentEditor
{
public:
    typedef std::function<void (IDocumentEditor*, bool)> ModifyObserver_t;

    virtual const QWidget *widget() const = 0;
    virtual QWidget *widget() = 0;
    virtual bool load(const QString& path) = 0;
    virtual bool save(const QString& path) = 0;
    virtual QString path() const { return widget()->windowFilePath(); }
    virtual void setPath(const QString& path) { widget()->setWindowFilePath(path); }
    virtual bool isReadonly() const = 0;
    virtual void setReadonly(bool rdOnly) = 0;
    virtual bool isModified() const = 0;
    virtual void setModified(bool m) = 0;
    virtual QPoint cursor() const = 0;
    virtual void setCursor(const QPoint& pos) = 0;

    void addModifyObserver(ModifyObserver_t fptr) {
        modifyObserverList.append(fptr);
    }

protected:
    void notifyModifyObservers() {
        for(auto& a: modifyObserverList)
            a(this, isModified());
    }

private:
    QList<ModifyObserver_t> modifyObserverList;
};

class IDocumentEditorCreator
{
public:
    virtual bool canHandleExtentions(const QStringList&) { return false; }
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
