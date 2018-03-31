#include "documentmanager.h"
#include "idocumenteditor.h"
#include "plaintexteditor.h"
#include "projectmanager.h"
#include "codetexteditor.h"
#include "binaryviewer.h"
#include "filesystemmanager.h"
#include "cpptexteditor.h"

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
    DocumentEditorFactory::instance()->registerDocumentInterface(CodeTextEditor::creator());
    DocumentEditorFactory::instance()->registerDocumentInterface(PlainTextEditor::creator());
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
    if (QFileInfo(path).isRelative())
        path = QDir(priv->projectManager->projectPath()).absoluteFilePath(path);
    QWidget *widget = nullptr;
    auto item = priv->mapedWidgets.value(path, nullptr);
    if (!item) {
        item = DocumentEditorFactory::instance()->create(path, this);
        if (item) {
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
                if (priv->projectManager)
                    item->setCodeModel(priv->projectManager->codeModel());
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

void DocumentManager::closeDocument(const QString &filePath)
{
    auto path = absoluteTo(priv->projectManager->projectPath(), filePath);
    if (path.isEmpty())
        return;
    auto iface = priv->mapedWidgets.value(path);
    if (!iface)
        return;
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
    }
}

void DocumentManager::closeAll()
{
    for(auto& path: priv->mapedWidgets.keys())
        closeDocument(path);
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
