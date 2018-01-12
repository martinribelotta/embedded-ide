#include "documentarea.h"

#include "codeeditor.h"
#include "qhexedit.h"
#include "mapviewer.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QTabWidget>
#include <QTabBar>
#include <QtDebug>
#include <QSvgRenderer>

static QPixmap loadImage(const QSize& size = QSize(256,256))
{
    QString url = ":/images/documentarea-bg.svg";
    QImage img(size, QImage::Format_ARGB32);
    img.fill(QColor(0, 0, 0, 0));
    QPainter p(&img);
    QSvgRenderer r(url);
    if (r.isValid())
        r.render(&p, QRect(QPoint(), size));
    return QPixmap::fromImage(img);
}

DocumentArea::DocumentArea(QWidget *parent) :
    ComboDocumentView(parent),
    lastIpEditor(nullptr)
{
    banner = new QLabel(this);
    banner->setPixmap(loadImage());
    banner->setAlignment (Qt::AlignCenter);
    banner->setBackgroundRole(QPalette::Base);
    banner->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    QList<QAction*> actions
            ({
#if 0
                 addActionToLeftCorner(QIcon(":/images/actions/go-next.svg"), tr("Next Position"), this, SLOT(goNext())),
                 addActionToLeftCorner(QIcon(":/images/actions/go-previous.svg"), tr("Prev Position"), this, SLOT(goPrev())),
#endif
                 addActionToRightCorner(QIcon(":/images/actions/view-refresh.svg"), tr("Reload File"), this, SLOT(reloadCurrent())),
                 addActionToRightCorner(QIcon(":/images/document-save.svg"), tr("Save File"), this, SLOT(saveCurrent())),
                 addActionToRightCorner(QIcon(":/images/document-save-all.svg"), tr("Save All"), this, SLOT(saveAll())),
                 addActionToRightCorner(QIcon(":/images/document-close.svg"), tr("Close Document"), this, SLOT(closeCurrent())),
                 addActionToRightCorner(QIcon(":/images/document-close-all.svg"), tr("Close All"), this, SLOT(closeAll()))
             });
    auto actionsEnable = [actions](bool en) { for(auto a: actions) a->setEnabled(en); };
#if 0
    auto *prev = actions[0];
    auto *next = actions[1];
    prev->setEnabled(false);
    next->setEnabled(false);
    connect(this, &ComboDocumentView::widgetCurrentChanged,
            [this, prev, next](int idx, QWidget *w) {
        QString documentPath = w->windowFilePath();
    });
#endif

    connect(this, &ComboDocumentView::widgetAdded,
            [this, actionsEnable](int idx, QWidget *w) {
        Q_UNUSED(idx);
        Q_UNUSED(w);
        actionsEnable(true);
        banner->setVisible(false);
    });
    connect(this, &ComboDocumentView::widgetRemoved,
            [this, actionsEnable](int idx, QWidget *w) {
        Q_UNUSED(idx);
        Q_UNUSED(w);
        bool noDocs = widgetCount() == 0;
        actionsEnable(!noDocs);
        banner->setVisible(noDocs);
    });
    actionsEnable(false);
    banner->setVisible(true);
}

DocumentArea::~DocumentArea()
{
}

QList<CodeEditor *> DocumentArea::documentsDirty() const
{
    QList<CodeEditor*> list;

    for(int i=0; i<widgetCount(); i++) {
        CodeEditor *e = qobject_cast<CodeEditor*>(widget(i));
        if (e) {
            if (e->isModified())
                list.append(e);
        }
    }

    return list;
}

bool DocumentArea::hasUnsavedChanges() {
    return !documentsDirty().isEmpty();
}

int DocumentArea::fileOpenAt(const QString &file, int row, int col, const MakefileInfo *mk)
{
    int idx = fileOpen(file, mk);
    if (idx == -1)
        return -1;
    CodeEditor *w = qobject_cast<CodeEditor*>(widget(idx));
    if (w) {
        w->moveTextCursor(row, col);
        w->setFocus();
    }
    return idx;
}

int DocumentArea::fileOpen(const QString &file, const MakefileInfo *mk)
{
    int idx = documentFind(file);
    if (idx == -1) {
        auto editor = new CodeEditor(this);
        connect(editor, &CodeEditor::requireOpen, this, &DocumentArea::fileOpenAt);
        editor->setMakefileInfo(mk);
        if (!editor->load(file))
            return -1;
        idx = addWidget(editor, editor->windowTitle());
        connect(editor, SIGNAL(modificationChanged(bool)), this, SLOT(modifyTab(bool)));
        connect(editor, &CodeEditor::destroyed, this, &DocumentArea::tabDestroy);
    }
    setCurrentIndex(idx);
    return idx;
}

void DocumentArea::clearIp()
{
    if (lastIpEditor)
        lastIpEditor->clearDebugPointer();
}

int DocumentArea::fileOpenAndSetIP(const QString &file, int line, const MakefileInfo *mk)
{
    int idx = fileOpenAt(file, line, 0, mk);
    if (idx == -1)
        return -1;
    CodeEditor *w = qobject_cast<CodeEditor*>(widget(idx));
    if (w) {
        if (lastIpEditor)
            lastIpEditor->clearDebugPointer();
        lastIpEditor = w;
        lastIpEditor->setDebugPointer(line);
    }
    return idx;
}

int DocumentArea::binOpen(const QString &filePath)
{
    int idx = documentFind(filePath);
    if (idx == -1) {
        auto file = new QFile(filePath);
        if (file)
            file->open(QFile::ReadOnly);
        QHexEditData *data = QHexEditData::fromDevice(file);
        if (!data) {
            delete file;
            return -1;
        }
        auto editor = new QHexEdit(this);
        file->setParent(editor);
        editor->setData(data);
        editor->setReadOnly(true);
        editor->setWindowTitle(QFileInfo(filePath).fileName());
        editor->setWindowFilePath(QFileInfo(filePath).absoluteFilePath());
        idx = addWidget(editor, editor->windowTitle());
        connect(editor, &CodeEditor::destroyed, this, &DocumentArea::tabDestroy);
    }
    setCurrentIndex(idx);
    return idx;
}

int DocumentArea::mapOpen(const QString &file)
{
    int idx = documentFind(file);
    if (idx == -1) {
        auto  editor = new MapViewer(this);
        editor->load(file);
        editor->setWindowTitle(QFileInfo(file).fileName());
        editor->setWindowFilePath(QFileInfo(file).absoluteFilePath());
        idx = addWidget(editor, editor->windowTitle());
        connect(editor, &CodeEditor::destroyed, this, &DocumentArea::tabDestroy);
    }
    setCurrentIndex(idx);
    return idx;
}

void DocumentArea::saveAll()
{
    for(int i=0; i<widgetCount(); i++) {
        CodeEditor *e = qobject_cast<CodeEditor*>(widget(i));
        if (e)
            e->save();
    }
}

bool DocumentArea::documentToClose(int idx)
{
    QWidget *w = widget(idx);
    if (w) {
        if (w->close()) {
            if (w == lastIpEditor)
                lastIpEditor = nullptr;
            w->deleteLater();
            removeWidget(idx);
            return true;
        }
    }
    return false;
}

void DocumentArea::closeAll()
{
    while(widgetCount() > 0)
        if (!documentToClose(0))
            break;
}

void DocumentArea::saveCurrent()
{
    CodeEditor *w = qobject_cast<CodeEditor*>(currentWidget());
    if (w)
        w->save();
}

void DocumentArea::reloadCurrent()
{
    CodeEditor *w = qobject_cast<CodeEditor*>(currentWidget());
    if (w)
        w->reload();
}

void DocumentArea::closeCurrent()
{
    documentToClose(currentIndex());
}

void DocumentArea::setTopBarHeight(int h)
{
    QSize z(h, h);
    for(auto b: findChildren<QToolButton*>()) {
        b->setIconSize(z);
    }
    // z.scale(h*1.25, h*1.25, Qt::IgnoreAspectRatio);
    for(auto c: findChildren<QComboBox*>()) {
        c->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        c->setMaximumHeight(z.height());
    }
}

void DocumentArea::goNext()
{
    // TODO
}

void DocumentArea::goPrev()
{
    // TODO
}

void DocumentArea::resizeEvent(QResizeEvent *e)
{
    ComboDocumentView::resizeEvent(e);
    QWidget *stack = findChild<QStackedWidget*>();
    banner->setGeometry(stack->geometry());
    banner->setVisible(widgetCount() == 0);
}

void DocumentArea::modifyTab(bool isModify)
{
    QWidget *w = qobject_cast<QWidget*>(sender());
    if (w) {
        QString title = w->windowTitle();
        if (isModify)
            title += tr(" [*]");
        setWidgetTitle(w, title);
    } else
        qWarning("sender of modifyTab is not a CodeEditor");
}

void DocumentArea::tabDestroy(QObject *obj)
{
    Q_UNUSED(obj);
}

int DocumentArea::documentFind(const QString &file, QWidget **ww)
{
    QFileInfo fileInfo(file);
    for(int i=0; i<widgetCount(); i++) {
        QWidget *w = widget(i);
        if (w) {
            if (QFileInfo(w->windowFilePath()) == fileInfo) {
                if (ww)
                    *ww = w;
                return i;
            }
        }
    }
    return -1;
}
