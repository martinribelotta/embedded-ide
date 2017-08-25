#include "codeeditor.h"

#include <cmath>
#include <strstream>

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

#include <astyle/astyle.h>
#include <astyle/astyle_main.h>

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

#include "clangcodecontext.h"
#include "documentarea.h"
#include "taglist.h"
#include "formfindreplace.h"
#include "appconfig.h"

#include <Qsci/qscilexeravs.h>
#include <Qsci/qscilexerbash.h>
#include <Qsci/qscilexerbatch.h>
#include <Qsci/qscilexercmake.h>
#include <Qsci/qscilexercoffeescript.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexercsharp.h>
#include <Qsci/qscilexercss.h>
#include <Qsci/qscilexercustom.h>
#include <Qsci/qscilexerd.h>
#include <Qsci/qscilexerdiff.h>
#include <Qsci/qscilexerfortran.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexeridl.h>
#include <Qsci/qscilexerjava.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerjson.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscilexermakefile.h>
#include <Qsci/qscilexermarkdown.h>
#include <Qsci/qscilexeroctave.h>
#include <Qsci/qscilexerpascal.h>
#include <Qsci/qscilexerperl.h>
#include <Qsci/qscilexerpo.h>
#include <Qsci/qscilexerpostscript.h>
#include <Qsci/qscilexerpov.h>
#include <Qsci/qscilexerproperties.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexerruby.h>
#include <Qsci/qscilexerspice.h>
#include <Qsci/qscilexersql.h>
#include <Qsci/qscilexertcl.h>
#include <Qsci/qscilexertex.h>
#include <Qsci/qscilexerverilog.h>
#include <Qsci/qscilexervhdl.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexeryaml.h>
#include <Qsci/qscistyle.h>

#include <QtXml/qdom.h>

#include <gdbdebugger/gdbdebugger.h>

#undef CLANG_DEBUG

CodeEditor::CodeEditor(QWidget *parent) :
    QsciScintilla(parent),
    mk(nullptr),
    ip(-1)
{
    m_completer = new QCompleter(this);
    m_completer->setObjectName("completer");
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setWidget(this);

    auto pModel = new QSortFilterProxyModel(m_completer);
    pModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    pModel->setSourceModel(new QStringListModel(m_completer));
    m_completer->setModel(pModel);
    connect(m_completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));

    replaceDialog = new FormFindReplace(this);
    replaceDialog->hide();

    auto findAction = new QAction(this);
    findAction->setShortcut(QKeySequence("ctrl+f"));
    connect(findAction, &QAction::triggered, replaceDialog, &FormFindReplace::show);
    addAction(findAction);

    auto formatAction = new QAction(this);
    formatAction->setShortcut(QKeySequence("ctrl+i"));
    connect(formatAction, &QAction::triggered, this, &CodeEditor::formatSelection);
    addAction(formatAction);

    auto saveAction = new QAction(this);
    saveAction->setShortcut(QKeySequence("ctrl+s"));
    connect(saveAction, &QAction::triggered, this, &CodeEditor::save);
    addAction(saveAction);
    connect(this, &QsciScintilla::marginClicked,
            [this](int margin, int line, Qt::KeyboardModifiers state){
        Q_UNUSED(margin);
        Q_UNUSED(state);
        if (markersAtLine(line) != 0) {
            if (GdbDebugger::instance()->isRunning()) {
                qDebug() << "del mark in" << margin << line;
                GdbDebugger::instance()->removeBreakPoint(m_documentFile, line);
                markerDelete(line);
            }
        } else {
            if (GdbDebugger::instance()->isRunning()) {
                qDebug() << "add mark in" << margin << line;
                GdbDebugger::instance()->insertBreakPoint(m_documentFile, line);
            }
        }
    });
    connect(GdbDebugger::instance(), &GdbDebugger::breakInserted, [this](int id, const QString& file, int line){
        const QFileInfo otherFile(QDir(mk->workingDir).absoluteFilePath(file));
        const QFileInfo thisFile(QDir(mk->workingDir).absoluteFilePath(m_documentFile));
        qDebug() << otherFile.absoluteFilePath() << thisFile.absoluteFilePath();
        if (thisFile == otherFile) {
            line--;
            breakPointMarkToLine.insert(id, line);
            markerAdd(line, SC_MARK_ARROW);
        }
    });
    connect(GdbDebugger::instance(), &GdbDebugger::breakDeleted, [this](int id){
        if (breakPointMarkToLine.contains(id)) {
            int line = breakPointMarkToLine.value(id);
            markerDelete(line);
        }
    });

    connect(this, &QsciScintilla::linesChanged, this, &CodeEditor::adjustLineNumberMargin);

    loadConfig();
    connect(&AppConfig::mutableInstance(), &AppConfig::configChanged, this, &CodeEditor::loadConfig);
}

void CodeEditor::insertCompletion(const QString &completion)
{
    QString s = completion;
    if (s.startsWith("Pattern : ")) {
        s = s.remove("Pattern : ").remove(QRegExp(R"([\\<|\[]\#[^\#]*\#[\>|\]])"));
    } else if (s.contains(':')) {
        s = s.split(':').at(0).trimmed();
    }

    int line, index;
    getCursorPosition(&line, &index);
    unsigned long position = static_cast<unsigned long>(positionFromLineIndex(line, index));
    long start_pos = SendScintilla(SCI_WORDSTARTPOSITION, position, true);
    long end_pos = SendScintilla(SCI_WORDENDPOSITION, position, true);
    // Delete word under cursor
    SendScintilla(SCI_DELETERANGE, static_cast<unsigned long>(start_pos), end_pos - start_pos);

    // Set start position to insert new completion
    SendScintilla(SCI_GOTOPOS, start_pos);
    insert(s);
    // Set position at end of inserted word
    SendScintilla(SCI_GOTOPOS, start_pos + s.length());
    m_completer->popup()->hide();
}

void CodeEditor::codeContextUpdate(const QStringList& list)
{
    // qDebug() << "completion:\n" << list;
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
    auto m = new QMenu(this);
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

class MyQsciLexerCPP: public QsciLexerCPP {
public:
    MyQsciLexerCPP(QObject *parent = 0, bool caseInsensitiveKeywords = false) :
        QsciLexerCPP(parent, caseInsensitiveKeywords)
    {
        setFoldCompact(false);
    }
};

template<typename T>
T *helperCreator() {
    return new T();
}

template<typename T>
struct QHashVal { QString k; T v; };

template<typename K, typename V>
static QHash<K, V> operator+(QHash<K, V> h, const QHashVal<V>& e) {
    h[e.k] = e.v;
    return h;
}

typedef QsciLexer* (*creator_t)();

#define _(mime, type) \
    (QHashVal<creator_t>{QString(mime), reinterpret_cast<creator_t>(&helperCreator<type>)})

static QHash<QString, creator_t> creatorMap = QHash<QString, creator_t>() +
_("application/json", QsciLexerJSON) +
_("text/x-octave", QsciLexerOctave) +
_("text/x-fortran", QsciLexerFortran) +
_("text/x-yaml", QsciLexerYAML) +
_("text/x-css", QsciLexerCSS) +
_("text/x-ps", QsciLexerPostScript) +
_("text/x-diff", QsciLexerDiff) +
_("text/x-avs", QsciLexerAVS) +
_("text/x-markdown", QsciLexerMarkdown) +
_("text/markdown", QsciLexerMarkdown) +
_("text/x-makefile", QsciLexerMakefile) +
_("text/x-pov", QsciLexerPOV) +
_("text/x-sql", QsciLexerSQL) +
_("text/x-html", QsciLexerHTML) +
_("text/x-po", QsciLexerPO) +
_("text/x-python", QsciLexerPython) +
_("text/x-lua", QsciLexerLua) +
_("text/x-xml", QsciLexerXML) +
_("text/x-idl", QsciLexerIDL) +
_("text/x-fortran", QsciLexerFortran) +
_("text/x-ruby", QsciLexerRuby) +
_("text/x-tex", QsciLexerTeX) +
_("text/x-bat", QsciLexerBatch) +
_("application/x-shellscript", QsciLexerBash) +
_("text/x-perl", QsciLexerPerl) +
_("text/x-cmake", QsciLexerCMake) +
_("text/x-java", QsciLexerJava) +
_("text/x-csharp", QsciLexerCSharp) +
_("text/x-properties", QsciLexerProperties) +
_("text/x-vhdl", QsciLexerVHDL) +
_("text/x-csrc", MyQsciLexerCPP) +
_("text/x-pascal", QsciLexerPascal) +
_("text/x-spice", QsciLexerSpice) +
_("text/x-matlab", QsciLexerMatlab) +
_("text/x-tcl", QsciLexerTCL) +
_("text/x-verilog", QsciLexerVerilog) +
_("text/x-javascript", QsciLexerJavaScript) +
_("text/x-dlang", QsciLexerD) +
_("text/x-coffe", QsciLexerCoffeeScript)
;
#undef _

static QsciLexer *lexerFromFile(const QString& name) {
    // qDebug() << "creatorMap" << creatorMap;
    QFile f(name);
    QMimeType type;
    if (f.open(QFile::ReadOnly))
        type = QMimeDatabase().mimeTypeForFileNameAndData(name, &f);
    else
        type = QMimeDatabase().mimeTypeForFile(name);
    QString mimename = type.name();
    // qDebug() << "mime" << mimename;
    if (creatorMap.contains(mimename)) {
        qDebug() << "for" << name << "mime found as" << mimename;
        return creatorMap.value(mimename)();
    }
    foreach(mimename, type.parentMimeTypes()) {
        if (creatorMap.contains(mimename)) {
            qDebug() << "for" << name << "parent mime found as" << mimename;
            return creatorMap.value(mimename)();
        }
    }
    return nullptr;
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
           QsciLexer *l=lexerFromFile(info.fileName());
           if (l) {
               setLexer(l);
               lexer()->setFont(font());
           }
           auto mime = QMimeDatabase().mimeTypeForFile(fileName);
           qDebug() << mime.name() << mime.allAncestors();
           if (mime.inherits("text/x-csrc")) {
               if (makefileInfo()) {
                   auto lang = new CLangCodeContext(this);
                   lang->setWorkingDir(makefileInfo()->workingDir);
                   lang->discoverFor(fileName);
               } else
                   qDebug() << "no makefile info";
           }
           loadConfig();
           if (mime.inherits("text/x-makefile"))
               setIndentationsUseTabs(true); // Force tabs if makefile
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
            loadConfig();
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
    auto layout = new QHBoxLayout(&dialog);
    auto view = new TagList(&dialog);
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

void CodeEditor::setDebugPointer(int line)
{
    qDebug() << "set debug pointer " << line;
    if (ip != -1)
       markerDelete(ip - 1, SC_MARK_BACKGROUND);
    ip = line;
    if (ip != -1) {
       markerAdd(ip - 1, SC_MARK_BACKGROUND);
    }
}

void CodeEditor::adjustLineNumberMargin()
{
    QFontMetrics m(font());
    setMarginWidth(0, m.width(QString().fill('0', 2 + static_cast<int>(std::log10(lines())))));
}

QString CodeEditor::wordUnderCursor() const {
    int line, col;
    getCursorPosition(&line, &col);
    return wordAtLineIndex(line, col);
#if 0
    QString str = text(line);
    int startPos = str.left(col).lastIndexOf(QRegExp("\\b"));
    int endPos = str.indexOf(QRegExp("\\b"), col);
    if ( startPos >= 0 && endPos >= 0 && endPos > startPos )
        return str.mid(startPos, endPos - startPos);
    else
        return "";
#endif
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

    QWidget *v = viewport();
    QRect r = v->rect();
    replaceDialog->adjustSize();
    r.setHeight(replaceDialog->height());
    r.moveBottom(v->rect().height());
    r.setLeft(v->pos().x());
    replaceDialog->setGeometry(r);
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    if (replaceDialog->isVisible()) {
        event->ignore();
        return;
    }
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
    findSimbol->setEnabled(isTextUnderCursor && makefileInfo() != nullptr);
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

static STDCALL char* tempMemoryAllocation(unsigned long memoryNeeded)
{
    char* buffer = new char[memoryNeeded];
    return buffer;
}

static STDCALL void tempError(int errorNumber, const char* errorMessage)
{
    qDebug() << errorNumber << errorMessage;
}

void CodeEditor::formatSelection()
{
    auto mime = QMimeDatabase().mimeTypeForFile(windowFilePath());
    if (mime.inherits("text/x-csrc")) {
        int l, i;
        getCursorPosition(&l, &i);
        QString inText = text();
        QByteArray rawText = inText.toUtf8();
        char* utf8In = rawText.data();
        const QString style = AppConfig::mutableInstance().editorFormatterStyle();
        char* utf8Out = AStyleMain(utf8In,
                                   QString("--style=%1").arg(style).toLatin1().data(),
                                   tempError,
                                   tempMemoryAllocation);
        setText(QString::fromUtf8(utf8Out));
        setCursorPosition(l, i);
        ensureLineVisible(l);
    }
}

void CodeEditor::loadConfig()
{
    AppConfig& config = AppConfig::mutableInstance();
    QFont fonts(config.editorFontStyle());
    fonts.setPointSize(config.editorFontSize());
    setFont(fonts);

    loadStyle(stylePath(config.editorStyle()));

    setIndentationsUseTabs(!config.editorTabsToSpaces());
    setTabWidth(config.editorTabWidth());
    setAutoIndent(true);
    setBraceMatching(StrictBraceMatch);
    resetMatchedBraceIndicator();
    setBackspaceUnindents(true);
    setFolding(CircledTreeFoldStyle);
    setIndentationGuides(true);

    setCaretLineVisible(true);
    setMarginsFont(fonts);
    setMarginLineNumbers(0, true);
    // setMarginsBackgroundColor(QColor("#cccccc"));

    SendScintilla(SCI_SETMULTIPLESELECTION, 1l, 0l);
    SendScintilla(SCI_SETADDITIONALSELECTIONTYPING, 1l, 0l);

    setMarginSensitivity(1, true);
    markerDefine(QsciScintilla::RightArrow, SC_MARK_ARROW);
    markerDefine(QsciScintilla::Background, SC_MARK_BACKGROUND);
    setMargins(3);
    // setMarkerBackgroundColor(QColor("#ee1111"), SC_MARK_ARROW);
    setAnnotationDisplay(AnnotationIndented);
    adjustLineNumberMargin();
}

bool CodeEditor::loadStyle(const QString &xmlStyleFile)
{
    QFile file(xmlStyleFile);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "cannot load" << xmlStyleFile << "style:" << file.errorString();
        return false;
    }
    QDomDocument doc;
    QString errorMsg;
    if (!doc.setContent(&file, &errorMsg)) {
        qDebug() << "cannot load" << xmlStyleFile << "style:" << errorMsg;
        return false;
    }
    QDomElement root = doc.firstChildElement("NotepadPlus");
    if (root.isNull()) {
        qDebug() << "cannot load" << xmlStyleFile << "not contain NotepadPlus tag";
        return false;
    }
    if (1) {
        QDomElement globalStyles = root.firstChildElement("GlobalStyles");
        if (globalStyles.isNull()) {
            qDebug() << "cannot load" << xmlStyleFile << "not GlobalStyles";
            return false;
        }
        QDomElement wStyle = globalStyles.firstChildElement("WidgetStyle");
        while (!wStyle.isNull()) {
            QString name = wStyle.attribute("name");
            int styleID = wStyle.attribute("styleID", "-1").toInt();
            QColor fgColor = QColor(QString("#%1").arg(wStyle.attribute("fgColor")));
            QColor bgColor = QColor(QString("#%1").arg(wStyle.attribute("bgColor")));
            //QString fontName = wStyle.attribute("fontName");
            int fontStyle = wStyle.attribute("fontStyle", "0").toInt();
            //int fontSize = wStyle.attribute("fontSize", QString("%1").arg(font().pixelSize())).toInt();
            if (styleID == 0) {
                if (name == "Global override") {
                    setColor(fgColor);
                    setPaper(bgColor);
                    goto set_global;
                } else if (name == "Current line background colour") {
                    setCaretLineBackgroundColor(bgColor);
                } else if (name == "Selected text colour") {
                    setSelectionBackgroundColor(bgColor);
                } else if (name == "Edge colour") {
                    setEdgeColor(fgColor);
                } else if (name == "Fold") {
                    // setFoldMarginColors(fgColor, bgColor);
                } else if (name == "Fold active") {
                } else if (name == "Fold margin") {
                    setFoldMarginColors(fgColor, bgColor);
                } else if (name == "White space symbol") {
                    setWhitespaceForegroundColor(fgColor);
                    setWhitespaceBackgroundColor(paper());
                } else if (name == "Active tab focused indicator") {
                } else if (name == "Active tab unfocused indicator") {
                } else if (name == "Active tab text") {
                } else if (name == "Inactive tabs") {
                }
            } else {
set_global:
                QsciStyle style(styleID);
                if (fgColor.isValid())
                    style.setColor(fgColor);
                if (bgColor.isValid())
                    style.setPaper(bgColor);
                QFont f(font());
                //if (!fontName.isEmpty() && QFont(fontName).family() == fontName)
                //    f.setFamily(fontName);
                //if (fontSize > 0)
                //    f.setPixelSize(fontSize);
                if (fontStyle&1)
                    f.setBold(true);
                if (fontStyle&2)
                    f.setItalic(true);
                if (fontStyle&4)
                    f.setUnderline(true);
                style.setFont(f);

                style.apply(this);
            }
            wStyle = wStyle.nextSiblingElement("WidgetStyle");
        }
    }
    if (lexer()) {
        QString currentLexerName = lexer()->lexer();
        QString currentLexerDesc = lexer()->language();
        QDomElement lexerStyles = root.firstChildElement("LexerStyles");
        if (!lexerStyles.isNull()) {
            QDomElement lType = lexerStyles.firstChildElement("LexerType");
            while (!lType.isNull()) {
                QString lexerName = lType.attribute("name");
                QString lexerDesc = lType.attribute("desc");
                if (lexerName == currentLexerName || lexerDesc == currentLexerDesc) {
                    QDomElement wStyle = lType.firstChildElement("WordsStyle");
                    while (!wStyle.isNull()) {
                        QString name = wStyle.attribute("name");
                        int styleID = wStyle.attribute("styleID", "-1").toInt();
                        if (!name.isEmpty() && styleID != -1) {
                            QColor fgColor = QColor(QString("#%1").arg(wStyle.attribute("fgColor")));
                            QColor bgColor = QColor(QString("#%1").arg(wStyle.attribute("bgColor")));
                            //QString fontName = wStyle.attribute("fontName");
                            int fontStyle = wStyle.attribute("fontStyle", "0").toInt();
                            //int fontSize = wStyle.attribute("fontSize", QString("%1").arg(font().pixelSize())).toInt();
                            QsciStyle style(styleID);
                            if (fgColor.isValid())
                                style.setColor(fgColor);
                            if (bgColor.isValid())
                                style.setPaper(bgColor);
                            QFont f(font());
                            //if (!fontName.isEmpty() && QFont(fontName).family() == fontName)
                            //    f.setFamily(fontName);
                            //if (fontSize > 0)
                            //    f.setPixelSize(fontSize);
                            if (fontStyle&1)
                                f.setBold(true);
                            if (fontStyle&2)
                                f.setItalic(true);
                            if (fontStyle&4)
                                f.setUnderline(true);
                            style.setFont(f);

                            style.apply(this);
                        }
                        wStyle = wStyle.nextSiblingElement("WordsStyle");
                    }
                    break;
                }
                lType = lType.nextSiblingElement("LexerType");
            }
        } else
            qDebug() << "No styles element";
    }
    return true;
}
