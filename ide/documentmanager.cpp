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
#include "documentmanager.h"
#include "idocumenteditor.h"
#include "plaintexteditor.h"
#include "projectmanager.h"
#include "codetexteditor.h"
#include "binaryviewer.h"
#include "filesystemmanager.h"
#include "cpptexteditor.h"
#include "mapfileviewer.h"
#include "unsavedfilesdialog.h"
#include "textmessagebrocker.h"
#include "imageviewer.h"

#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QMimeDatabase>
#include <QSortFilterProxyModel>
#include <QStackedLayout>

#include <QtDebug>

static QString absoluteTo(const QString& path, const QString& file) {
    return QFileInfo(file).isAbsolute()? file : QDir(path).absoluteFilePath(file);
}

class DocumentManager::Priv_t {
public:
    QComboBox *combo = nullptr;
    QStackedLayout *stack = nullptr;
    QHash<QString, IDocumentEditor*> mapedWidgets;
    const ProjectManager *projectManager = nullptr;
};

DocumentManager::DocumentManager(QWidget *parent) :
    QWidget(parent),
    priv(new Priv_t)
{
    priv->stack = new QStackedLayout(this);
    priv->stack->setMargin(0);

    DocumentEditorFactory::instance()->registerDocumentInterface(CPPTextEditor::creator());
    DocumentEditorFactory::instance()->registerDocumentInterface(ImageViewer::creator());
    DocumentEditorFactory::instance()->registerDocumentInterface(CodeTextEditor::creator());
    DocumentEditorFactory::instance()->registerDocumentInterface(PlainTextEditor::creator());
    DocumentEditorFactory::instance()->registerDocumentInterface(MapFileViewer::creator());
    DocumentEditorFactory::instance()->registerDocumentInterface(BinaryViewer::creator());

    auto label = new QLabel(this);
    label->setPixmap(QPixmap(":/images/screens/EmbeddedIDE_02.png"));
    label->setAlignment(Qt::AlignHCenter | Qt::AlignCenter);
    priv->stack->addWidget(label);
}

DocumentManager::~DocumentManager()
{
    delete priv;
}

void DocumentManager::setComboBox(QComboBox *cb)
{
    if (priv->combo)
        priv->combo->disconnect(this);

    priv->combo = cb;
    auto model = priv->combo->model();
    auto proxy = new QSortFilterProxyModel(priv->combo);
    model->setParent(proxy);
    proxy->setSourceModel(model);
    proxy->setSortRole(Qt::DisplayRole);
    proxy->setDynamicSortFilter(false);
    priv->combo->setModel(proxy);
    connect(priv->combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        auto path = priv->combo->itemData(idx).toString();
        if (!path.isEmpty())
            openDocument(path);
    });
    connect(priv->stack, &QStackedLayout::currentChanged, [this](int idx) {
        auto widget = priv->stack->widget(idx);
        if (widget) {
            auto path = widget->windowFilePath();
            auto iface = priv->mapedWidgets.value(path, nullptr);
            if (iface) {
                int comboIdx = priv->combo->findData(path);
                if (comboIdx == -1) {
                    priv->combo->addItem(QFileInfo(path).fileName(), path);
                    priv->combo->model()->sort(0);
                    comboIdx = priv->combo->findData(path);
                    priv->combo->setItemIcon(comboIdx, FileSystemManager::iconForFile(QFileInfo(path)));
                }
                priv->combo->setCurrentIndex(comboIdx);
            }
        }
    });
}

QStringList DocumentManager::unsavedDocuments() const
{
    QStringList unsaved;
    for(auto d: priv->mapedWidgets.values())
        if (d->isModified())
            unsaved.append(d->path());
    return unsaved;
}

QStringList DocumentManager::documents() const
{
    return priv->mapedWidgets.keys();
}

int DocumentManager::documentCount() const
{
    return priv->mapedWidgets.count();
}

QString DocumentManager::documentCurrent() const
{
    if (priv->stack->currentWidget()) {
        auto path = priv->stack->currentWidget()->windowFilePath();
        auto iface = priv->mapedWidgets.value(path, nullptr);
        if (iface)
            return iface->path();
    }
    return QString();
}

IDocumentEditor *DocumentManager::documentEditor(const QString& path) const
{
    return priv->mapedWidgets.value(absoluteTo(priv->projectManager->projectPath(), path), nullptr);
}

void DocumentManager::setProjectManager(const ProjectManager *projectManager)
{
    priv->projectManager = projectManager;
}

void DocumentManager::openDocument(const QString &filePath)
{
    QString path = absoluteTo(priv->projectManager->projectPath(), filePath);
    if (QFileInfo(path).isDir())
        return;
    if (!QFileInfo(path).exists()) {
        TextMessageBrocker::instance().publish("stderrLog", tr("%1 not exist").arg(filePath));
        return;
    }
    if (QFileInfo(path).isRelative())
        path = QDir(priv->projectManager->projectPath()).absoluteFilePath(path);
    QWidget *widget = nullptr;
    auto item = priv->mapedWidgets.value(path, nullptr);
    if (!item) {
        item = DocumentEditorFactory::instance()->create(path, this);
        if (item) {
            if (priv->projectManager)
                item->setCodeModel(priv->projectManager->codeModel());
            widget = item->widget();
            if (!item->load(path)) {
                item->widget()->deleteLater();
                item = nullptr;
                widget = nullptr;
            } else {
                item->setModified(false);
                priv->mapedWidgets.insert(path, item);
                priv->stack->addWidget(widget);
                item->addModifyObserver([this](IDocumentEditor *ed, bool m) {
                    auto path = ed->path();
                    auto idx = priv->combo->findData(path);
                    if (idx != -1) {
                        priv->combo->setItemIcon(idx, m? QIcon(":/images/actions/document-close.svg") :
                                                         FileSystemManager::iconForFile(QFileInfo(path)));
                    }
                    emit documentModified(path, ed, m);
                });
                item->setDocumentManager(this);
            }
        }
    } else
        widget = item->widget();
    if (widget) {
        priv->stack->setCurrentWidget(widget);
        emit documentFocushed(path);
    } else
        emit documentNotFound(path);
}

void DocumentManager::openDocumentHere(const QString &path, int line, int col)
{
    qDebug() << "open document here" << path << line << col;
    openDocument(path);
    auto ed = documentEditor(path);
    if (ed)
        ed->setCursor(QPoint(col, line));
}

bool DocumentManager::closeDocument(const QString &filePath)
{
    auto path = absoluteTo(priv->projectManager->projectPath(), filePath);
    // Cannot save due not name on path
    if (path.isEmpty())
        return true;
    // Cannot save due not in map (not widget interface registered)
    auto iface = priv->mapedWidgets.value(path);
    if (!iface)
        return true;
    if (iface->widget()->close()) {
        priv->stack->removeWidget(iface->widget());
        if (priv->combo) {
            int idx = priv->combo->findData(iface->path());
            if (idx != -1)
                priv->combo->removeItem(idx);
        }
        iface->widget()->deleteLater();
        priv->mapedWidgets.remove(path);
        emit documentClosed(path);
        return true;
    }
    return false;
}

bool DocumentManager::closeAll()
{
    for(auto& path: priv->mapedWidgets.keys())
        if (!closeDocument(path))
            return false;
    return true;
}

bool DocumentManager::aboutToCloseAll()
{
    auto unsaved = unsavedDocuments();
    if (unsaved.isEmpty()) {
        return closeAll();
    }
    UnsavedFilesDialog d(unsaved, this);
    if (d.exec() == QDialog::Accepted) {
        saveDocuments(d.checkedForSave());
        for(const auto& doc: unsavedDocuments()) {
            auto iface = documentEditor(doc);
            if (iface)
                iface->setModified(false);
        }
        return closeAll();
   }
    return false;
}

void DocumentManager::saveDocument(const QString &path)
{
    if (path.isEmpty())
        return;
    auto iface = priv->mapedWidgets.value(absoluteTo(priv->projectManager->projectPath(), path));
    if (!iface)
        return;
    iface->save(iface->path());
}

void DocumentManager::saveAll()
{
    for(auto& path: priv->mapedWidgets.keys())
        saveDocument(path);
}

void DocumentManager::reloadDocument(const QString &path)
{
    if (path.isEmpty())
        return;
    auto iface = priv->mapedWidgets.value(absoluteTo(priv->projectManager->projectPath(), path));
    if (!iface)
        return;
    iface->reload();
}

void DocumentManager::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    auto w = priv->stack->currentWidget();
    if (w)
        w->setFocus();
}
