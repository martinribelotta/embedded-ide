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
#ifndef IDOCUMENTEDITOR_H
#define IDOCUMENTEDITOR_H

#include "documentmanager.h"

#include <QObject>
#include <QWidget>
#include <QString>
#include <QPoint>
#include <QMimeType>

#include <functional>

class ICodeModelProvider;

class IDocumentEditor
{
public:
    typedef std::function<void (IDocumentEditor*, bool)> ModifyObserver_t;

    virtual ~IDocumentEditor();

    virtual const QWidget *widget() const = 0;
    virtual QWidget *widget() = 0;
    virtual bool load(const QString& path) = 0;
    virtual bool save(const QString& path) = 0;
    virtual void reload() = 0;
    virtual QString path() const { return widget()->windowFilePath(); }
    virtual void setPath(const QString& path) { widget()->setWindowFilePath(path); }
    virtual bool isReadonly() const = 0;
    virtual void setReadonly(bool rdOnly) = 0;
    virtual bool isModified() const = 0;
    virtual void setModified(bool m) = 0;
    virtual QPoint cursor() const = 0;
    virtual void setCursor(const QPoint& pos) = 0;

    void setDocumentManager(DocumentManager *man) { this->man = man; }
    DocumentManager *documentManager() const { return this->man; }
    void addModifyObserver(ModifyObserver_t fptr) {
        modifyObserverList.append(fptr);
    }

    void setCodeModel(ICodeModelProvider *m) { _codeModel = m; }
    ICodeModelProvider *codeModel() const { return _codeModel; }

protected:
    void notifyModifyObservers() {
        for(auto& a: modifyObserverList)
            a(this, isModified());
    }

private:
    QList<ModifyObserver_t> modifyObserverList;
    ICodeModelProvider *_codeModel = nullptr;
    DocumentManager *man = nullptr;
};

class IDocumentEditorCreator
{
public:
    virtual ~IDocumentEditorCreator();

    virtual bool canHandleExtentions(const QStringList&) const { return false; }
    virtual bool canHandleMime(const QMimeType&) const { return false; }
    virtual IDocumentEditor *create(QWidget *parent = nullptr) const = 0;

    template<typename T>
    static IDocumentEditorCreator *staticCreator() {
        static IDocumentEditorCreator *singleton = nullptr;
        if (!singleton)
            singleton = new T;
        return singleton;
    }
};

class DocumentEditorFactory: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DocumentEditorFactory)

private:
    DocumentEditorFactory();
    QList<IDocumentEditorCreator*> creators;

public:
    static DocumentEditorFactory* instance();

    void registerDocumentInterface(IDocumentEditorCreator *creator);
    IDocumentEditor *create(const QString& path, QWidget *parent = nullptr);
};

#endif // IDOCUMENTEDITOR_H
