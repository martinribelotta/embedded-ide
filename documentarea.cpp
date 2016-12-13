#include "documentarea.h"
#include "codeeditor.h"
#include "qhexedit.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QTabBar>
#include <QtDebug>

DocumentArea::DocumentArea(QWidget *parent) :
    QTabWidget(parent)
{
#if 0
    setStyleSheet(
                "QTabWidget::pane { /* The tab widget frame */"
                "    border-top: 2px solid #C2C7CB;"
                "}"
                "QTabWidget::tab-bar {"
                "    left: 5px; /* move to the right by 5px */"
                "}"
                "QTabBar::tab {"
                "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                "                                stop: 0 #E1E1E1, stop: 0.4 #DDDDDD,"
                "                                stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3);"
                "    border: 2px solid #C4C4C3;"
                "    border-bottom-color: #C2C7CB; /* same as the pane color */"
                "    border-top-left-radius: 4px;"
                "    border-top-right-radius: 4px;"
                "    min-width: 8ex;"
                "    padding: 2px;"
                "}"
                "QTabBar::tab:selected, QTabBar::tab:hover {"
                "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                "                                stop: 0 #fafafa, stop: 0.4 #f4f4f4,"
                "                                stop: 0.5 #e7e7e7, stop: 1.0 #fafafa);"
                "}"
                "QTabBar::tab:selected {"
                "    border-color: #9B9B9B;"
                "    border-bottom-color: #C2C7CB; /* same as pane color */"
                "}"
                "QTabBar::tab:!selected {"
                "    margin-top: 2px; /* make non-selected tabs look smaller */"
                "}"
                );
#endif
    QWidget *buttonBox = new QWidget(this);
    QHBoxLayout *buttonBoxLayout = new QHBoxLayout(buttonBox);
    buttonBoxLayout->setMargin(2);
    //buttonBoxLayout->setContentsMargins(0, 0, 0, 0);
    QToolButton *closeAll = new QToolButton(buttonBox);
    QToolButton *saveCurrent = new QToolButton(buttonBox);
    QToolButton *reloadCurrent = new QToolButton(buttonBox);
    buttonBoxLayout->addWidget(reloadCurrent);
    buttonBoxLayout->addWidget(saveCurrent);
    buttonBoxLayout->addWidget(closeAll);

    reloadCurrent->setIcon(QIcon::fromTheme("view-refresh", QIcon(":/icon-theme/icon-theme/view-refresh.png")));
    closeAll->setIcon(QIcon::fromTheme("tab-close-other", QIcon(":/icon-theme/icon-theme/tab-close-other.png")));
    saveCurrent->setIcon(QIcon::fromTheme("document-save", QIcon(":/icon-theme/icon-theme/document-save.png")));
    connect(closeAll, SIGNAL(clicked()), this, SLOT(closeAll()));
    connect(saveCurrent, SIGNAL(clicked()), this, SLOT(saveCurrent()));
    connect(reloadCurrent, SIGNAL(clicked()), this, SLOT(reloadCurrent()));
    setCornerWidget(buttonBox, Qt::TopRightCorner);
    setDocumentMode(false);
    setTabsClosable(true);
    setMovable(true);
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(documentToClose(int)));
}

bool DocumentArea::fileOpen(const QString &file, int row, int col, const MakefileInfo *mk)
{
    int idx = documentFind(file);
    if (idx == -1) {
        CodeEditor *editor = new CodeEditor(this);
        connect(editor, &CodeEditor::requireOpen, this, &DocumentArea::fileOpen);
        editor->setMakefileInfo(mk);
        if (!editor->load(file))
            return false;
        idx = addTab(editor, editor->windowTitle());
        connect(editor, SIGNAL(modificationChanged(bool)), this, SLOT(modifyTab(bool)));
        connect(editor, &CodeEditor::destroyed, this, &DocumentArea::tabDestroy);
    }
    setCurrentIndex(idx);
    CodeEditor *w = qobject_cast<CodeEditor*>(widget(idx));
    if (w) {
        w->moveTextCursor(row, col);
        w->setFocus();
    }
    return true;
}

bool DocumentArea::binOpen(const QString &file)
{
    int idx = documentFind(file);
    if (idx == -1) {
        QHexEditData *data = QHexEditData::fromFile(file);
        if (!data)
            return false;
        QHexEdit *editor = new QHexEdit(this);
        editor->setData(data);
        editor->setReadOnly(true);
        editor->setWindowTitle(QFileInfo(file).fileName());
        editor->setWindowFilePath(QFileInfo(file).absoluteFilePath());
        idx = addTab(editor, editor->windowTitle());
        connect(editor, &CodeEditor::destroyed, this, &DocumentArea::tabDestroy);
    }
    setCurrentIndex(idx);
    return true;
}

void DocumentArea::saveAll()
{
    for(int i=0; i<count(); i++) {
        CodeEditor *e = qobject_cast<CodeEditor*>(widget(i));
        if (e)
            e->save();
    }
}

void DocumentArea::documentToClose(int idx)
{
    CodeEditor *w = qobject_cast<CodeEditor*>(widget(idx));
    if (w) {
        w->deleteLater();
    }
    removeTab(idx);
}

void DocumentArea::closeAll()
{
    clear();
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

void DocumentArea::modifyTab(bool isModify)
{
    CodeEditor *w = qobject_cast<CodeEditor*>(sender());
    if (w) {
        int idx = indexOf(w);
        if (idx != -1) {
            QString title = w->windowTitle();
            if (isModify)
                title += tr(" [*]");
            setTabText(idx, title);
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
    for(int i=0; i<count(); i++) {
        CodeEditor *w = qobject_cast<CodeEditor*>(widget(i));
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
