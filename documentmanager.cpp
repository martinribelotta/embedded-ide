#include "documentmanager.h"
#include "idocumenteditor.h"
#include "plaintexteditor.h"

#include <QComboBox>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QStackedLayout>

class DocumentManager::Priv_t {
public:
    QComboBox *combo = nullptr;
    QStackedLayout *stack = nullptr;
    QHash<QString, IDocumentEditor*> mapedWidgets;
};

DocumentManager::DocumentManager(QWidget *parent) :
    QWidget(parent),
    priv(new Priv_t)
{
    priv->stack = new QStackedLayout(this);
    priv->stack->setMargin(0);
    DocumentEditorFactory::instance()->registerDocumentInterface(PlainTextEditor::creator());
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
    connect(priv->combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        auto path = priv->combo->itemData(idx).toString();
        openDocument(path);
    });
    connect(priv->stack, &QStackedLayout::currentChanged, [this](int idx) {
        auto widget = priv->stack->widget(idx);
        if (widget) {
            auto path = widget->windowFilePath();
            auto iface = priv->mapedWidgets.value(path, nullptr);
            if (iface) {
                int comboIdx = priv->combo->findData(path);
                if (comboIdx != -1) {
                    priv->combo->setCurrentIndex(comboIdx);
                } else {
                    priv->combo->addItem(QFileInfo(path).fileName(), path);
                    priv->combo->setCurrentIndex(priv->combo->count() - 1);
                    priv->combo->model()->sort(0);
                }
            }
        }
    });
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
    auto path = priv->stack->currentWidget()->windowFilePath();
    auto iface = priv->mapedWidgets.value(path, nullptr);
    return iface? iface->path() : QString();
}

IDocumentEditor *DocumentManager::documentEditor(const QString& path) const
{
    return priv->mapedWidgets.value(path, nullptr);
}

void DocumentManager::openDocument(const QString &path)
{
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

void DocumentManager::closeDocument(const QString &path)
{
    if (path.isEmpty())
        return;
    auto iface = priv->mapedWidgets.value(path);
    if (!iface)
        return;
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

void DocumentManager::closeAll()
{
    for(auto& path: priv->mapedWidgets.keys())
        closeDocument(path);
}
