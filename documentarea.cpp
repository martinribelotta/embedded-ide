#include "documentarea.h"
#include "editorwidget.h"

#include <QToolButton>
#include <QTabBar>
#include <QtDebug>

DocumentArea::DocumentArea(QWidget *parent) :
    QTabWidget(parent)
{
    setDocumentMode(true);
    setTabsClosable(true);
    setMovable(true);
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(documentToClose(int)));
}

bool DocumentArea::fileOpen(const QString &file)
{
    int idx = documentFind(file);
    if (idx == -1) {
        EditorWidget *editor = new EditorWidget(this);
        if (!editor->load(file))
            return false;
        idx = addTab(editor, editor->windowTitle());
    }
    setCurrentIndex(idx);
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
