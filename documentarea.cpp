#include "documentarea.h"
#include "codeeditor.h"
#include "qhexedit.h"
#include "mapviewer.h"

#include <QTabWidget>
#include <QHBoxLayout>
#include <QToolButton>
#include <QTabBar>
#include <QtDebug>

DocumentArea::DocumentArea(QWidget *parent) :
    QWidget(parent),
    lastIpEditor(0l)
{
    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);
    tab = new QTabWidget(this);
    QWidget *buttonBox = new QWidget(this);
    l->addWidget(buttonBox);
    l->addWidget(tab);
    QHBoxLayout *buttonBoxLayout = new QHBoxLayout(buttonBox);
    buttonBoxLayout->setMargin(2);
    QToolButton *closeAll = new QToolButton(buttonBox);
    QToolButton *saveCurrent = new QToolButton(buttonBox);
    QToolButton *saveAll = new QToolButton(buttonBox);
    QToolButton *reloadCurrent = new QToolButton(buttonBox);
    buttonBoxLayout->addWidget(reloadCurrent);
    buttonBoxLayout->addWidget(saveCurrent);
    buttonBoxLayout->addWidget(saveAll);
    buttonBoxLayout->addWidget(closeAll);

    reloadCurrent->setIcon(QIcon("://images/actions/view-refresh.svg"));
    reloadCurrent->setToolTip(tr("Reload File"));
    closeAll->setIcon(QIcon("://images/actions/window-close.svg"));
    closeAll->setToolTip(tr("Close All"));
    saveAll->setIcon(QIcon("://images/document-save-all.svg"));
    saveAll->setToolTip(tr("Save All"));
    saveCurrent->setIcon(QIcon("://images/document-save.svg"));
    saveCurrent->setToolTip(tr("Save File"));
    connect(closeAll, SIGNAL(clicked()), this, SLOT(closeAll()));
    connect(saveAll, SIGNAL(clicked()), this, SLOT(saveAll()));
    connect(saveCurrent, SIGNAL(clicked()), this, SLOT(saveCurrent()));
    connect(reloadCurrent, SIGNAL(clicked()), this, SLOT(reloadCurrent()));
    // tab->setCornerWidget(buttonBox, Qt::TopRightCorner);
    tab->setDocumentMode(false);
    tab->setTabsClosable(true);
    tab->setMovable(true);
    connect(tab, SIGNAL(tabCloseRequested(int)), this, SLOT(documentToClose(int)));
}

QList<CodeEditor *> DocumentArea::documentsDirty() const
{
    QList<CodeEditor*> list;
    for(int i=0; i<tab->count(); i++) {
        CodeEditor *e = qobject_cast<CodeEditor*>(tab->widget(i));
        if (e) {
            if (e->document()->isModified())
                list.append(e);
        }
    }
    return list;
}

int DocumentArea::fileOpenAt(const QString &file, int row, int col, const MakefileInfo *mk)
{
    int idx = fileOpen(file, mk);
    if (idx == -1)
        return -1;
    CodeEditor *w = qobject_cast<CodeEditor*>(tab->widget(idx));
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
        CodeEditor *editor = new CodeEditor(this);
        connect(editor, &CodeEditor::requireOpen, this, &DocumentArea::fileOpenAt);
        editor->setMakefileInfo(mk);
        if (!editor->load(file))
            return -1;
        idx = tab->addTab(editor, editor->windowTitle());
        tab->setTabToolTip(idx, file);
        connect(editor, SIGNAL(modificationChanged(bool)), this, SLOT(modifyTab(bool)));
        connect(editor, &CodeEditor::destroyed, this, &DocumentArea::tabDestroy);
    }
    tab->setCurrentIndex(idx);
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
    CodeEditor *w = qobject_cast<CodeEditor*>(tab->widget(idx));
    if (w) {
        if (lastIpEditor)
            lastIpEditor->clearDebugPointer();
        lastIpEditor = w;
        lastIpEditor->setDebugPointer(line, Qt::blue);
    }
    return idx;
}

int DocumentArea::binOpen(const QString &file)
{
    int idx = documentFind(file);
    if (idx == -1) {
        QHexEditData *data = QHexEditData::fromFile(file);
        if (!data)
            return -1;
        QHexEdit *editor = new QHexEdit(this);
        editor->setData(data);
        editor->setReadOnly(true);
        editor->setWindowTitle(QFileInfo(file).fileName());
        editor->setWindowFilePath(QFileInfo(file).absoluteFilePath());
        idx = tab->addTab(editor, editor->windowTitle());
        tab->setTabToolTip(idx, file);
        connect(editor, &CodeEditor::destroyed, this, &DocumentArea::tabDestroy);
    }
    tab->setCurrentIndex(idx);
    return idx;
}

int DocumentArea::mapOpen(const QString &file)
{
    int idx = documentFind(file);
    if (idx == -1) {
        MapViewer * editor = new MapViewer(this);
        editor->load(file);
        editor->setWindowTitle(QFileInfo(file).fileName());
        editor->setWindowFilePath(QFileInfo(file).absoluteFilePath());
        idx = tab->addTab(editor, editor->windowTitle());
        tab->setTabToolTip(idx, file);
        connect(editor, &CodeEditor::destroyed, this, &DocumentArea::tabDestroy);
    }
    tab->setCurrentIndex(idx);
    return idx;
}

void DocumentArea::saveAll()
{
    for(int i=0; i<tab->count(); i++) {
        CodeEditor *e = qobject_cast<CodeEditor*>(tab->widget(i));
        if (e)
            e->save();
    }
}

void DocumentArea::documentToClose(int idx)
{
    QWidget *w = tab->widget(idx);
    if (w) {
        if (w->close()) {
            w->deleteLater();
            tab->removeTab(idx);
        }
    }
}

void DocumentArea::closeAll()
{
    while(tab->count() > 0)
        documentToClose(0);
}

void DocumentArea::saveCurrent()
{
    CodeEditor *w = qobject_cast<CodeEditor*>(tab->currentWidget());
    if (w)
        w->save();
}

void DocumentArea::reloadCurrent()
{
    CodeEditor *w = qobject_cast<CodeEditor*>(tab->currentWidget());
    if (w)
        w->reload();
}

void DocumentArea::modifyTab(bool isModify)
{
    CodeEditor *w = qobject_cast<CodeEditor*>(sender());
    if (w) {
        int idx = tab->indexOf(w);
        if (idx != -1) {
            QString title = w->windowTitle();
            if (isModify)
                title += tr(" [*]");
            tab->setTabText(idx, title);
        } else
            qWarning("sender is not a tab");
    } else
        qWarning("sender of modifyTab is not a CodeEditor");
}

void DocumentArea::tabDestroy(QObject *obj)
{
    Q_UNUSED(obj);
}

int DocumentArea::documentFind(const QString &file, CodeEditor **ww)
{
    for(int i=0; i<tab->count(); i++) {
        CodeEditor *w = qobject_cast<CodeEditor*>(tab->widget(i));
        if (w) {
            if (w->windowFilePath() == file) {
                if (ww)
                    *ww = w;
                return i;
            }
        }
    }
    return -1;
}
