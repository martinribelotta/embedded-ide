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
    ~DocumentManager() override;

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
    void documentPositionModified(const QString& path, int line, int col);

public slots:
    IDocumentEditor *openDocument(const QString& filePath);
    void openDocumentHere(const QString& path, int line, int col);
    bool closeDocument(const QString& filePath);
    bool closeCurrent() { return closeDocument(documentCurrent()); }
    bool closeAll();
    bool aboutToCloseAll();
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
