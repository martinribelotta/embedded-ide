#include "documentarea.h"
#include "codeeditor.h"
#include "qhexedit.h"
#include "mapviewer.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QTabWidget>
#include <QTabBar>
#include <QtDebug>

void TabWidget::tabInserted(int) { emit refresh(); }
void TabWidget::tabRemoved(int) { emit refresh(); }

DocumentArea::DocumentArea(QWidget *parent) :
    QWidget(parent),
    tab(nullptr),
    lastIpEditor(nullptr)
{
    auto l1 = new QVBoxLayout(this);
    auto buttonBox = new QWidget(this);
    auto buttonBoxLayout = new QHBoxLayout(buttonBox);
    buttonBoxLayout->setMargin(0);
    buttonBoxLayout->setSpacing(0);
    buttonBoxLayout->setContentsMargins(0, 1, 0, 0);

    auto windowListCombo = new QComboBox(buttonBox);
    auto closeAll = new QToolButton(buttonBox);
    auto saveCurrent = new QToolButton(buttonBox);
    auto saveAll = new QToolButton(buttonBox);
    auto closeCurrent = new QToolButton(buttonBox);
    auto reloadCurrent = new QToolButton(buttonBox);

    windowListModel = new QStandardItemModel(this);
    windowListCombo->setModel(windowListModel);
    windowListCombo->setObjectName("windowListCombo");
    tab = new TabWidget(this);
    connect(tab, &TabWidget::refresh, [this, buttonBox]() {
        windowListModel->clear();
        buttonBox->setEnabled(tab->count() > 0);
        for(int i=0; i<tab->count(); i++) {
            QStandardItem *item = new QStandardItem(tab->tabText(i));
            item->setIcon(QIcon(":/images/document-new.svg"));
            item->setData(i);
            windowListModel->appendRow(item);
        }
        windowListModel->sort(0);
    });
    tab->setObjectName("documentTabArea");

    buttonBoxLayout->addWidget(windowListCombo);
    buttonBoxLayout->addWidget(reloadCurrent);
    buttonBoxLayout->addWidget(saveCurrent);
    buttonBoxLayout->addWidget(saveAll);
    buttonBoxLayout->addWidget(closeAll);
    buttonBoxLayout->addWidget(closeCurrent);

    reloadCurrent->setIcon(QIcon(":/images/actions/view-refresh.svg"));
    reloadCurrent->setToolTip(tr("Reload File"));

    closeAll->setIcon(QIcon(":/images/actions/window-close.svg"));
    closeAll->setToolTip(tr("Close All"));

    closeCurrent->setIcon(QIcon(":/images/document-close.svg"));
    closeCurrent->setToolTip(tr("Close Document"));

    saveAll->setIcon(QIcon(":/images/document-save-all.svg"));
    saveAll->setToolTip(tr("Save All"));


    saveCurrent->setIcon(QIcon(":/images/document-save.svg"));
    saveCurrent->setToolTip(tr("Save File"));

    connect(closeAll, SIGNAL(clicked()), this, SLOT(closeAll()));
    connect(saveAll, SIGNAL(clicked()), this, SLOT(saveAll()));
    connect(saveCurrent, SIGNAL(clicked()), this, SLOT(saveCurrent()));
    connect(reloadCurrent, SIGNAL(clicked()), this, SLOT(reloadCurrent()));
    connect(closeCurrent, SIGNAL(clicked()), this, SLOT(closeCurrent()));

    tab->setDocumentMode(true);
    connect(tab, &TabWidget::currentChanged, [this, windowListCombo](int i) {
        windowListCombo->setCurrentText(tab->tabText(i));
    });

    connect(windowListCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), [this](int row) {
        int n = windowListModel->item(row, 0)->data().toInt();
        tab->widget(0);
        tab->setCurrentIndex(n);
        tab->widget(n)->setFocus();
    });

    l1->addWidget(buttonBox);
    l1->addWidget(tab);
    l1->setMargin(0);
    l1->setContentsMargins(0, 0, 0, 0);
    l1->setSpacing(0);
    buttonBox->setEnabled(tab->count() > 0);
}

QList<CodeEditor *> DocumentArea::documentsDirty() const
{
    QList<CodeEditor*> list;

    for(int i=0; i<tab->count(); i++) {
        CodeEditor *e = qobject_cast<CodeEditor*>(tab->widget(i));
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
        auto editor = new CodeEditor(this);
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
        idx = tab->addTab(editor, editor->windowTitle());
        tab->setTabToolTip(idx, filePath);
        connect(editor, &CodeEditor::destroyed, this, &DocumentArea::tabDestroy);
    }
    tab->setCurrentIndex(idx);
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

bool DocumentArea::documentToClose(int idx)
{
    QWidget *w = tab->widget(idx);
    if (w) {
        if (w->close()) {
            w->deleteLater();
            tab->removeTab(idx);
            return true;
        }
    }
    return false;
}

void DocumentArea::closeAll()
{
    while(tab->count() > 0)
        if (!documentToClose(0))
            break;
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

void DocumentArea::closeCurrent()
{
    documentToClose(tab->currentIndex());
}

void DocumentArea::windowListUpdate()
{
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
            tab->setTabTitle(idx, title);
            for(int row=0; row<windowListModel->rowCount(); row++) {
                auto item = windowListModel->item(row, 0);
                if (item->data().toInt() == idx) {
                    item->setText(title);
                }
            }
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
