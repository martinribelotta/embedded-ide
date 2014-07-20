#include "codeeditor.h"

#include <QPainter>
#include <QTextBlock>
#include <QCompleter>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QShortcut>
#include <QKeyEvent>
#include <QProcess>
#include <QFileInfo>

#include <QHBoxLayout>
#include <QListView>
#include <QStringListModel>

#include <QtDebug>

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor) {
        codeEditor = editor;
    }

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

CodeEditor::CodeEditor(QWidget *parent) :
    QPlainTextEdit(parent)
{
    m_completer = new QCompleter(this);
    m_completer->setObjectName("completer");
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setWidget(this);
    m_completer->setModel(new QStringListModel(m_completer));
    connect(m_completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));

    completionProc = new QProcess(this);
    connect(completionProc, SIGNAL(readyRead()), this, SLOT(completionDone()));
    connect(completionProc, SIGNAL(started()), this, SLOT(sendCurrentCode()));

    QFont f("monospace");
    f.setStyleHint(QFont::Monospace);
    setFont(f);
    setWordWrapMode(QTextOption::NoWrap);

    lineNumberArea = new LineNumberArea(this);

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

void CodeEditor::sendCurrentCode()
{
    completionProc->write(toPlainText().toLocal8Bit());
    completionProc->write("\r\n"); // XXX REquired by completion on EOF. Whi?
    completionProc->closeWriteChannel();
}

void CodeEditor::insertCompletion(const QString &completion)
{
    qDebug() << "Complete with" << completion;
    QTextCursor tc = textUnderCursor();
    tc.insertText(completion);
    setTextCursor(tc);
    m_completer->popup()->hide();
}

static QStringList parseClangOut(const QString& out) {
    qDebug() << "clang out:" << out;
    QStringList list;
    QRegExp re("COMPLETION\\: ([^\\n]*)");
    int pos = 0;
    while(re.indexIn(out, pos) != -1) {
        list.append(re.cap(1));
        pos += re.matchedLength();
    }
    return list;
}

void CodeEditor::completionDone()
{
    QStringListModel *m = qobject_cast<QStringListModel*>(m_completer->model());
    m->setStringList(parseClangOut(completionProc->readAll()));
    completionShow();
}

void CodeEditor::completionActivate()
{
    QString llvmPath = "/opt/llvm/bin/"; // FIXME Configure it
    completionProc->start(QString("%3clang -cc1 -code-completion-at -:%1:%2 -")
                          .arg(textCursor().blockNumber() + 1)
                          .arg(textCursor().columnNumber() + 1)
                          .arg(llvmPath));
}

void CodeEditor::completionShow()
{
    QString underCursor = textUnderCursor().selectedText();
    m_completer->setCompletionPrefix(underCursor);
    qDebug() << "under cursor" << underCursor;
    int w = m_completer->popup()->sizeHintForColumn(0) +
            m_completer->popup()->verticalScrollBar()->sizeHint().width();
    QRect r = cursorRect();
    r.setWidth(w);
    m_completer->complete(r);
}

bool CodeEditor::load(const QString &fileName)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        setPlainText(f.readAll());
        if (f.error() == QFile::NoError) {
           m_documentFile = f.fileName();
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
    }
    emit fileError(f.errorString());
    return false;
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
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    if (m_completer->popup()->isVisible()) {
        switch(e->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return;
        }
        QPlainTextEdit::keyPressEvent(e);
        completionShow();
    } else if (e->modifiers() == Qt::CTRL && e->key() == Qt::Key_Space) {
        completionActivate();
    } else {
        QString line;
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
            line = lineUnderCursor();
        QPlainTextEdit::keyPressEvent(e);
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

void CodeEditor::refreshHighlighterLines()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

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
