#include "codeeditor.h"

#include <QAction>
#include <QDir>
#include <QPainter>
#include <QTextBlock>
#include <QCompleter>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QShortcut>
#include <QKeyEvent>
#include <QFileInfo>
#include <QSettings>
#include <QTemporaryFile>
#include <QApplication>

#include <QHBoxLayout>
#include <QListView>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QMimeDatabase>
#include <QMenu>
#include <QListWidget>
#include <QDialog>
#include <QMessageBox>

#include <QtDebug>

#include <qsvsh/qsvsyntaxhighlighter.h>
#include <qsvsh/qsvcolordeffactory.h>
#include <qsvsh/qsvcolordef.h>
#include <qsvsh/qsvlangdef.h>
#include <qsvsh/qsvlangdeffactory.h>

#include "qsvtextoperationswidget.h"
#include "clangcodecontext.h"
#include "documentarea.h"
#include "taglist.h"

#undef CLANG_DEBUG

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor) {
        codeEditor = editor;
    }
    virtual ~LineNumberArea();

    QSize sizeHint() const {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

    void mouseDoubleClickEvent(QMouseEvent *e) {
        codeEditor->lineNumberAreaDoubleClick(e->pos());
    }

private:
    CodeEditor *codeEditor;
};

LineNumberArea::~LineNumberArea()
{
}

CodeEditor::CodeEditor(QWidget *parent) :
    QPlainTextEdit(parent),
    defColors(0l),
    langDef(0l),
    syntax(0l),
    mk(0l),
    ip(-1)
{
    m_completer = new QCompleter(this);
    m_completer->setObjectName("completer");
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setWidget(this);

    QSortFilterProxyModel *pModel = new QSortFilterProxyModel(m_completer);
    pModel->setSourceModel(new QStringListModel(m_completer));
    m_completer->setModel(pModel);
    connect(m_completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));

    QFont f(QSettings().value("editor/font/style", "DejaVu Sans Mono").toString());
    f.setPointSize(QSettings().value("editor/font/size", 10).toInt());
    setFont(f);
    setWordWrapMode(QTextOption::NoWrap);

    lineNumberArea = new LineNumberArea(this);
    textOpWidget = new QsvTextOperationsWidget(this);

    QAction *findAction = new QAction(this);
    findAction->setShortcut(QKeySequence("ctrl+f"));
    connect(findAction, SIGNAL(triggered()), textOpWidget, SLOT(showSearch()));
    addAction(findAction);

    QAction *replaceAction = new QAction(this);
    replaceAction->setShortcut(QKeySequence("ctrl+h"));
    connect(replaceAction, SIGNAL(triggered()), textOpWidget, SLOT(showReplace()));
    addAction(replaceAction);
#if 0
    // TODO
    QAction *gotoAction = new QAction(this);
    gotoAction->setShortcut(QKeySequence("ctrl+g"));
    connect(gotoAction, SIGNAL(triggered()), textOpWidget, SLOT(showGotoLine()));
    addAction(gotoAction);
#endif
    QAction *saveAction = new QAction(this);
    saveAction->setShortcut(QKeySequence("ctrl+s"));
    connect(saveAction, &QAction::triggered, this, &CodeEditor::save);
    addAction(saveAction);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(refreshHighlighterLines()));

    updateLineNumberAreaWidth(0);
    refreshHighlighterLines();

}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 16 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::insertCompletion(const QString &completion)
{
    QString s = completion;
    QTextCursor tc = textUnderCursor();
    if (s.startsWith("Pattern : ")) {
        s = s.remove("Pattern : ").remove(QRegExp(R"([\\<|\[]\#[^\#]*\#[\>|\]])"));
    } else if (s.contains(':')) {
        s = s.split(':').at(0).trimmed();
    }
    tc.insertText(s);
    setTextCursor(tc);
    m_completer->popup()->hide();
}

void CodeEditor::codeContextUpdate(const QStringList& list)
{
    QSortFilterProxyModel *pModel = qobject_cast<QSortFilterProxyModel*>(m_completer->model());
    QStringListModel *m = qobject_cast<QStringListModel*>(pModel->sourceModel());
    m->setStringList(list);
    completionShow();
}

void CodeEditor::moveTextCursor(int row, int col)
{
    QTextDocument *doc = document();
    QTextBlock block = doc->begin();
    int off = 0;
    for(int y=1 ; y<row; y++ ) {
        off += block.length();
        if (block != doc->end())
            block = block.next();
        else
            return;
    }
    off+= col;

    QTextCursor c = textCursor();
    c.setPosition( off );
    setTextCursor( c );
}

void CodeEditor::completionShow()
{
    QString underCursor = textUnderCursor().selectedText();
    QSortFilterProxyModel *pModel = qobject_cast<QSortFilterProxyModel*>(m_completer->model());
    pModel->setFilterFixedString(underCursor);
    int w = m_completer->popup()->sizeHintForColumn(0) +
            m_completer->popup()->verticalScrollBar()->sizeHint().width();
    QRect cr = cursorRect();
    QRect rr = QRect(viewport()->mapTo(this, cr.topLeft()), cr.size());
    rr.setWidth(std::min(w, size().width() - rr.left()));
    m_completer->complete(rr);
}

static QString findStyleByName(const QString& defaultName) {
    QDir d(":/qsvsh/qtsourceview/data/colors/");
    foreach(QString name, d.entryList(QStringList("*.xml"))) {
        QDomDocument doc("mydocument");
        QFile file(d.filePath(name));
        if (file.open(QIODevice::ReadOnly) && doc.setContent(&file)) {
            QDomNodeList itemDatas = doc.elementsByTagName("itemDatas");
            if (!itemDatas.isEmpty()) {
                QDomNamedNodeMap attr = itemDatas.at(0).attributes();
                QString name = attr.namedItem("name").toAttr().value();
                if (defaultName == name)
                    return file.fileName();
            }
        }
    }
    return QString();
}

bool CodeEditor::load(const QString &fileName)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        setPlainText(f.readAll());
        if (f.error() == QFile::NoError) {
           m_documentFile = f.fileName();
           QFileInfo info(f);
           setWindowFilePath(info.absoluteFilePath());
           setWindowTitle(info.fileName());

           if (defColors)
               delete defColors;
           if (langDef)
               delete langDef;
           if (syntax)
               syntax->deleteLater();
           langDef = 0l;
           syntax = 0l;

           defColors = new QsvColorDefFactory( findStyleByName(QSettings().value("editor/colorstyle", "Kate").toString()) );
           langDef = QsvLangDefFactory::getInstanse()->getHighlight(info.fileName());
           if (defColors && langDef) {
               setProperty("makefileMode", langDef->getMimeTypes().contains("text/x-makefile"));
               syntax = new QsvSyntaxHighlighter( document() , defColors, langDef );
               syntax->setObjectName("syntaxer");
               QPalette p = palette();
               p.setColor(QPalette::Base, defColors->getColorDef("dsWidgetBackground").getBackground());
               setPalette(p);
               qDebug() << m_documentFile << "as" << langDef->getName() << langDef->getMimeTypes();
               if (CLangCodeContext::HANDLE_TYPE.contains(langDef->getName())) {
                   if (makefileInfo())
                       (new CLangCodeContext(this))->setWorkingDir(makefileInfo()->workingDir);
                   else
                       qDebug() << "no makefile info";
               }

               refreshHighlighterLines();
           }
           return true;
        }
    }
    emit fileError(f.errorString());
    return false;
}

bool CodeEditor::save()
{
    if (m_documentFile.isEmpty())
        return true;
    QFile f(m_documentFile);
    if (f.open(QFile::WriteOnly)) {
        f.write(toPlainText().toLocal8Bit());
        if (f.error() == QFile::NoError) {
            document()->setModified(false);
            return true;
        }
    } else
        emit fileError(f.errorString());
    return false;
}

void CodeEditor::reload()
{
    if (!m_documentFile.isEmpty()) {
        int pos = textCursor().position();
        QFile f(m_documentFile);
        if (f.open(QFile::ReadOnly)) {
            setPlainText(f.readAll());
            QTextCursor cursor = this->textCursor();
            cursor.setPosition(pos, QTextCursor::MoveAnchor);
            this->setTextCursor(cursor);
        }
    }
}

void CodeEditor::smartHome()
{
    QTextCursor c = textCursor();
    QTextCursor::MoveMode moveAnchor = QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)?
         QTextCursor::KeepAnchor:QTextCursor::MoveAnchor;
    if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
        c.movePosition(QTextCursor::Start, moveAnchor);
        setTextCursor(c);
        return;
    }

    int blockLen = c.block().text().length();
    if (blockLen == 0 )
        return;

    int originalPosition = c.position();
    c.movePosition(QTextCursor::StartOfLine, moveAnchor);
    int startOfLine = c.position();
    int i = 0;
    while ( c.block().text()[i].isSpace()) {
        i ++;
        if (i==blockLen) {
            i = 0;
            break;
        }
    }
    if ((originalPosition == startOfLine) || (startOfLine + i != originalPosition ))
        c.setPosition( startOfLine + i, moveAnchor );
    setTextCursor( c );
}

void CodeEditor::smartEnd()
{
    QTextCursor c = textCursor();
    QTextCursor::MoveMode moveAnchor = QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)?
         QTextCursor::KeepAnchor:QTextCursor::MoveAnchor;
    if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
        c.movePosition(QTextCursor::End, moveAnchor);
        setTextCursor(c);
        return;
    }

    int blockLen = c.block().text().length();
    if (blockLen == 0)
        return;

    int originalPosition = c.position();
    c.movePosition(QTextCursor::StartOfLine,moveAnchor);
    int startOfLine = c.position();
    c.movePosition(QTextCursor::EndOfLine,moveAnchor);
    int i = blockLen;
    while (c.block().text()[i-1].isSpace())	{
        i --;
        if (i==1)		{
            i = blockLen;
            break;
        }
    }
    if ((originalPosition == startOfLine) || (startOfLine + i != originalPosition ))
        c.setPosition( startOfLine + i, moveAnchor );

    setTextCursor( c );
}

extern QString mkUrl(const QString& p, const QString& x, const QString& y);

void CodeEditor::findTagUnderCursor()
{
    QString tag = wordUnderCursor().selectedText();
    ETags tagList = makefileInfo()->tags;
    QList<ETags::Tag> tagFor = tagList.find(tag);

    QDialog dialog(this);
    QHBoxLayout *layout = new QHBoxLayout(&dialog);
    //QListWidget *view = new QListWidget(&dialog);
    TagList *view = new TagList(&dialog);
    view->setTagList(tagFor);
    layout->addWidget(view);
    layout->setMargin(0);
    connect(view, SIGNAL(itemActivated(QListWidgetItem *)), &dialog, SLOT(accept()));
    dialog.resize(dialog.sizeHint());
    if (dialog.exec() == QDialog::Accepted) {
        ETags::Tag sel = view->selectedTag();
        if (sel.isEmpty()) {
            qDebug() << "No selection found";
        } else {
            QString url = makefileInfo()->workingDir + QDir::separator() + sel.file;
            qDebug() << "opening " << url;
            emit requireOpen(url, sel.line, 0, makefileInfo());
        }
    }
}

static bool isWord(QChar c)
{
    return (c.isLetterOrNumber() || c == '_');
}

QTextCursor CodeEditor::wordUnderCursor() const
{
    QTextCursor tc = textCursor();
    QTextDocument *doc = document();
    do {
        if (!tc.movePosition(QTextCursor::Left))
            break;
    } while(isWord(doc->characterAt(tc.position())));
    if (!isWord(doc->characterAt(tc.position())))
        tc.movePosition(QTextCursor::Right);
    while (isWord(doc->characterAt(tc.position())))
        tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    return tc;
}

QTextCursor CodeEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    QTextDocument *doc = document();
    do {
        if (!tc.movePosition(QTextCursor::Left))
            break;
    } while(!doc->characterAt(tc.position()).isSpace());
    if (doc->characterAt(tc.position()).isSpace())
        tc.movePosition(QTextCursor::Right);
    while (!doc->characterAt(tc.position()).isSpace())
        tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    return tc;
}

QString CodeEditor::lineUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::LineUnderCursor);
    return tc.selectedText();
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    emit widgetResized();
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    if (m_completer->popup()->isVisible()) {
        switch(event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            event->ignore();
            return;
        }
        QPlainTextEdit::keyPressEvent(event);
        completionShow();
    } else {
        QString line;
        switch(event->key()) {
        case Qt::Key_Space:
            if (event->modifiers() == Qt::CTRL) {
                emit updateCodeContext();
                event->accept();
                return;
            }
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            line = lineUnderCursor();
            break;
        case Qt::Key_Tab:
            do {
                int insertTab = QSettings().value("editor/replaceTabs", 0).toInt();
                if (insertTab && !property("makefileMode").toBool()) {
                    this->insertPlainText(QString(" ").repeated(insertTab));
                    return;
                }
            } while(0);
            break;
        case Qt::Key_Home:
            smartHome();
            event->accept();
            return;
        case Qt::Key_End:
            smartEnd();
            event->accept();
            return;
        }
        QPlainTextEdit::keyPressEvent(event);
        if (!line.isEmpty()) {
            int i=0;
            QString nextIndend;
            while(i<line.length() && line.at(i).isSpace()) {
                nextIndend += line.at(i);
                i++;
            }
            if (i<line.length() && !line.at(i).isSpace())
                insertPlainText(nextIndend);
        }
    }
}

void CodeEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QString word = wordUnderCursor().selectedText();
    qDebug() << word;
    bool isTextUnderCursor = !word.isEmpty();
    QMenu *menu = createStandardContextMenu();
    menu->addSeparator();
    QAction *findSimbol = menu->addAction(tr("Find symbol under cursor"), this, SLOT(findTagUnderCursor()));
    findSimbol->setEnabled(isTextUnderCursor);
    menu->exec(event->globalPos());
    event->accept();
}

void CodeEditor::closeEvent(QCloseEvent *event)
{
    if (document()->isModified()) {
        QMessageBox messageBox(QMessageBox::Question,
                               tr("Document Modified"),
                               tr("The document is not save. Save it?"),
                               QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort,
                               this);
        messageBox.setButtonText(QMessageBox::Yes, tr("Yes"));
        messageBox.setButtonText(QMessageBox::No, tr("No"));
        messageBox.setButtonText(QMessageBox::Abort, tr("Abort"));
        int r = messageBox.exec();
        switch (r) {
        case QMessageBox::Yes:
            save();
            break;
        case QMessageBox::Abort:
            event->ignore();
            break;
        case QMessageBox::No:
            break;
        }
    }
}

void CodeEditor::refreshHighlighterLines()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor;
        QsvSyntaxHighlighter *syntax = findChild<QsvSyntaxHighlighter*>("syntaxer");
        if (syntax)
            lineColor = syntax->colorDefFactory()->getColorDef("dsWidgetCurLine").getBackground();

        if (!lineColor.isValid())
            lineColor = QColor(0xe0, 0xee, 0xf6);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    QList<int> invalids;
    QHashIterator<int, QColor> i(highlightLines);
    while (i.hasNext()) {
        i.next();
        int line = i.key();
        QColor c = i.value();
        QTextEdit::ExtraSelection selection;

        QColor lineColor = c;

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.setPosition(0);
        if (selection.cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line - 1)) {
            selection.cursor.clearSelection();
            extraSelections.append(selection);
        } else {
            qDebug() << "invalid line" << line;
            invalids.append(line); // Don't modify hash table over loop
        }
    }
    foreach(int invalidLine, invalids)
        highlightLines.remove(invalidLine);

    setExtraSelections(extraSelections);
}

static inline const QColor transparent(const QColor& c, float a) {
    return QColor(c.red(), c.green(), c.blue(), (int) (255*a));
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.setBrush(QBrush(QColor(0, 0, 0, 0)));
            painter.drawText(0, top, lineNumberArea->width() - 8, fontMetrics().height(),
                             Qt::AlignRight, number);
            if (highlightLines.contains(blockNumber + 1)) {
                painter.setPen(QPen(Qt::NoPen));
                painter.setBrush(QBrush(transparent(QColor(Qt::red).lighter(160), 0.5)));
                QRect r(QPoint(0, top), QSize(lineNumberArea->width(), fontMetrics().height()));
                painter.drawRect(r);
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void CodeEditor::lineNumberAreaDoubleClick(const QPoint &p)
{
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid()) {
        if (block.isVisible() && p.y() >= top && p.y() <= bottom) {
            toggleHighlightLine(blockNumber + 1, QColor(Qt::green).lighter(160));
            break;
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}
