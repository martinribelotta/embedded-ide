#include "documentarea.h"
#include "editorwidget.h"

#include <QToolButton>
#include <QTabBar>
#include <QtDebug>

DocumentArea::DocumentArea(QWidget *parent) :
    QTabWidget(parent)
{
    //setStyleSheet(QString("background-color: %1;").arg(QColor(Qt::darkGray).name(QColor::HexArgb)));
//    setStyleSheet(
//                "QTabWidget::pane { /* The tab widget frame */"
//                "    border-top: 2px solid #C2C7CB;"
//                "}"
//                "QTabWidget::tab-bar {"
//                "    left: 5px; /* move to the right by 5px */"
//                "}"
//                "QTabBar::tab {"
//                "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
//                "                                stop: 0 #E1E1E1, stop: 0.4 #DDDDDD,"
//                "                                stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3);"
//                "    border: 2px solid #C4C4C3;"
//                "    border-bottom-color: #C2C7CB; /* same as the pane color */"
//                "    border-top-left-radius: 4px;"
//                "    border-top-right-radius: 4px;"
//                "    min-width: 8ex;"
//                "    padding: 2px;"
//                "}"
//                "QTabBar::tab:selected, QTabBar::tab:hover {"
//                "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
//                "                                stop: 0 #fafafa, stop: 0.4 #f4f4f4,"
//                "                                stop: 0.5 #e7e7e7, stop: 1.0 #fafafa);"
//                "}"
//                "QTabBar::tab:selected {"
//                "    border-color: #9B9B9B;"
//                "    border-bottom-color: #C2C7CB; /* same as pane color */"
//                "}"
//                "QTabBar::tab:!selected {"
//                "    margin-top: 2px; /* make non-selected tabs look smaller */"
//                "}"
//                );
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
