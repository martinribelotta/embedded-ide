#include "documentarea.h"
#include "editorwidget.h"

#include <QToolButton>
#include <QTabBar>
#include <QtDebug>

DocumentArea::DocumentArea(QWidget *parent) :
    QTabWidget(parent)
{
    QToolButton *closeAll = new QToolButton(this);
    closeAll->setIcon(QIcon::fromTheme("tab-close-other", QIcon(":/icon-theme/icon-theme/tab-close-other.png")));
    connect(closeAll, SIGNAL(clicked()), this, SLOT(closeAll()));
    setCornerWidget(closeAll, Qt::TopRightCorner);
    setDocumentMode(true);
    setTabsClosable(true);
    setMovable(true);
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(documentToClose(int)));
}

bool DocumentArea::fileOpen(const QString &file, int row, int col)
{
    int idx = documentFind(file);
    if (idx == -1) {
        EditorWidget *editor = new EditorWidget(this);
        if (!editor->load(file))
            return false;
        idx = addTab(editor, editor->windowTitle());        
        connect(editor, SIGNAL(modified(bool)), this, SLOT(modifyTab(bool)));
    }
    setCurrentIndex(idx);
    EditorWidget *w = qobject_cast<EditorWidget*>(widget(idx));
    if (w) {
        w->moveCursor(row, col);
        w->setFocus();
    }
    return true;
}

void DocumentArea::saveAll()
{
    for(int i=0; i<count(); i++) {
        EditorWidget *e = qobject_cast<EditorWidget*>(widget(i));
        if (e)
            e->save();
    }
}

void DocumentArea::documentToClose(int idx)
{
    EditorWidget *w = qobject_cast<EditorWidget*>(widget(idx));
    if (w) {
        w->deleteLater();
    }
    removeTab(idx);
}

void DocumentArea::closeAll()
{
    clear();
}

void DocumentArea::modifyTab(bool isModify)
{
    EditorWidget *w = qobject_cast<EditorWidget*>(sender());
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
        qWarning("sender of modifyTab is not a EditorWidget");
}

int DocumentArea::documentFind(const QString &file)
{
    for(int i=0; i<count(); i++) {
        EditorWidget *w = qobject_cast<EditorWidget*>(widget(i));
        if (w) {
            if (w->windowFilePath() == file)
                return i;
        }
    }
    return -1;
}
