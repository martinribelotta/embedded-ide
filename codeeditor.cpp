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

#include <Qsci/qscilexercpp.h>

#undef CLANG_DEBUG

CodeEditor::CodeEditor(QWidget *parent) :
    QsciScintilla(parent),
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

    QFont font(QSettings().value("editor/font/style", "DejaVu Sans Mono").toString());
    font.setPointSize(QSettings().value("editor/font/size", 10).toInt());
    setFont(font);

    setAutoIndent(true);
    setBraceMatching(StrictBraceMatch);
    setBackspaceUnindents(true);
    setFolding(BoxedTreeFoldStyle);
    setIndentationGuides(true);

    setCaretLineVisible(true);
    setCaretLineBackgroundColor(QColor("#ffe4e4"));
    auto fontmetrics = QFontMetrics(font);
    setMarginsFont(font);
    setMarginWidth(0, fontmetrics.width("00000") + 6);
    setMarginLineNumbers(0, true);
    setMarginsBackgroundColor(QColor("#cccccc"));

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

    refreshHighlighterLines();

}

void CodeEditor::insertCompletion(const QString &completion)
{
    QString s = completion;
    if (s.startsWith("Pattern : ")) {
        s = s.remove("Pattern : ").remove(QRegExp(R"([\\<|\[]\#[^\#]*\#[\>|\]])"));
    } else if (s.contains(':')) {
        s = s.split(':').at(0).trimmed();
    }
    insert(s);
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
    setCursorPosition(row, col);
}

QRect CodeEditor::cursorRect() const
{
    int pos = static_cast<int>(SendScintilla(SCI_GETCURRENTPOS));
    int pos_line = static_cast<int>(SendScintilla(SCI_LINEFROMPOSITION, static_cast<unsigned long>(pos), 0l));
    int x = static_cast<int>(SendScintilla(SCI_POINTXFROMPOSITION, 0, pos));
    int y = static_cast<int>(SendScintilla(SCI_POINTYFROMPOSITION, 0, pos));
    int xp1 = static_cast<int>(SendScintilla(SCI_POINTXFROMPOSITION, 0, pos+1));
    int sizex = qMax(1, xp1-x);
    int sizey = static_cast<int>(SendScintilla(SCI_TEXTHEIGHT, static_cast<unsigned long>(pos_line), 0l));
    QRect r(x, y, sizex, sizey);
    return r;
}

void CodeEditor::completionShow()
{
    QString underCursor = wordUnderCursor(); // textUnderCursor().selectedText();
    QSortFilterProxyModel *pModel = qobject_cast<QSortFilterProxyModel*>(m_completer->model());
    pModel->setFilterFixedString(underCursor);
    int w = m_completer->popup()->sizeHintForColumn(0) +
            m_completer->popup()->verticalScrollBar()->sizeHint().width();
    QRect cr = cursorRect();
    QRect rr = QRect(viewport()->mapTo(this, cr.topLeft()), cr.size());
    rr.setWidth(std::min(w, size().width() - rr.left()));
    m_completer->complete(rr);
}

static QAction *setActionEnable(bool en, QAction *a) {
    a->setEnabled(en);
    return a;
}

QMenu *CodeEditor::createContextMenu()
{
    QMenu *m = new QMenu(this);
    bool isSelected = !selectedText().isEmpty(); // textCursor().hasSelection();
    bool canPaste = static_cast<bool>(SendScintilla(SCI_CANPASTE));
    setActionEnable(isUndoAvailable(), m->addAction(QIcon(":/images/edit-undo.svg"), tr("&Undo"), this, SLOT(undo())))->setShortcut(QKeySequence("Ctrl+Z"));
    setActionEnable(isRedoAvailable(), m->addAction(QIcon(":/images/edit-redo.svg"), tr("&Redo"), this, SLOT(redo())))->setShortcut(QKeySequence("Ctrl+Shift+Z"));
    m->addSeparator();
    setActionEnable(isSelected, m->addAction(QIcon(":/images/edit-cut.svg"), tr("Cu&t"), this, SLOT(cut())))->setShortcut(QKeySequence("Ctrl+X"));
    setActionEnable(isSelected, m->addAction(QIcon(":/images/edit-copy.svg"), tr("&Copy"), this, SLOT(copy())))->setShortcut(QKeySequence("Ctrl+C"));
    setActionEnable(canPaste, m->addAction(QIcon(":/images/edit-paste.svg"), tr("&Paste"), this, SLOT(paste())))->setShortcut(QKeySequence("Ctrl+V"));
    setActionEnable(isSelected, m->addAction(QIcon(":/images/edit-delete.svg"), tr("Delete"), this, SLOT(clearSelection())));
    m->addSeparator();
    m->addAction(tr("&Select All"), this, SLOT(selectAll()))->setShortcut(QKeySequence("CTRL+A"));
    return m;
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

static QsciLexer *lexerFromFile(const QString& filename) {
    QsciLexer *l = new QsciLexerCPP();
    return l;
}

bool CodeEditor::load(const QString &fileName)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        read(&f);
        setModified(false);
        if (f.error() == QFile::NoError) {
           m_documentFile = f.fileName();
           QFileInfo info(f);
           setWindowFilePath(info.absoluteFilePath());
           setWindowTitle(info.fileName());
           setLexer(lexerFromFile(info.fileName()));
           lexer()->setFont(font());
           auto mime = QMimeDatabase().mimeTypeForFile(fileName);
           qDebug() << mime.name() << mime.allAncestors();
           if (mime.inherits("text/x-csrc")) {
               if (makefileInfo())
                   (new CLangCodeContext(this))->setWorkingDir(makefileInfo()->workingDir);
               else
                   qDebug() << "no makefile info";
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
        bool r = write(&f);
        if (r)
            setModified(false);
        return r;
    } else
        emit fileError(f.errorString());
    return false;
}

void CodeEditor::reload()
{
    if (!m_documentFile.isEmpty()) {
        int l, i;
        getCursorPosition(&l, &i);
        QFile f(m_documentFile);
        bool mod = isModified();
        if (f.open(QFile::ReadOnly)) {
            read(&f);
            setCursorPosition(l, i);
            setModified(mod);
        }
    }
}

void CodeEditor::clearSelection()
{
    removeSelectedText();
}

void CodeEditor::findTagUnderCursor()
{
    QString tag = wordUnderCursor();
    ETags tagList = makefileInfo()->tags;
    QList<ETags::Tag> tagFor = tagList.find(tag);

    QDialog dialog(this);
    QHBoxLayout *layout = new QHBoxLayout(&dialog);
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

QString CodeEditor::wordUnderCursor() const {
    int line, col;
    getCursorPosition(&line, &col);
    QString str = text(line);
    int startPos = str.left(col).lastIndexOf(QRegExp("\\b"));
    int endPos = str.indexOf(QRegExp("\\b"), col);
    if ( startPos >= 0 && endPos >= 0 && endPos > startPos )
        return str.mid(startPos, endPos - startPos);
    else
        return "";
}

QString CodeEditor::lineUnderCursor() const
{
    int line, col;
    getCursorPosition(&line, &col);
    QString str = text(line);
    return str;
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QsciScintilla::resizeEvent(e);
    //QPlainTextEdit::resizeEvent(e);
    // QRect cr = contentsRect();
    // lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
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
        QsciScintilla::keyPressEvent(event);
        completionShow();
    } else {
        switch(event->key()) {
        case Qt::Key_Space:
            if (event->modifiers() == Qt::CTRL) {
                emit updateCodeContext();
                event->accept();
                return;
            }
            break;
        }
        QsciScintilla::keyPressEvent(event);
    }
}

void CodeEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QString word = wordUnderCursor();
    bool isTextUnderCursor = !word.isEmpty();
    QMenu *menu = createContextMenu();
    menu->addSeparator();
    QAction *findSimbol = menu->addAction(QIcon(":/images/edit-find.svg"),
                                          tr("Find symbol under cursor"),
                                          this, SLOT(findTagUnderCursor()));
    findSimbol->setEnabled(isTextUnderCursor);
    menu->exec(event->globalPos());
    event->accept();
}

void CodeEditor::closeEvent(QCloseEvent *event)
{
    if (isModified()) {
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
#if 0
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
#endif
}
