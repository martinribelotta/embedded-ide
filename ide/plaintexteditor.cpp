#include "appconfig.h"
#include "formfindreplace.h"
#include "plaintexteditor.h"

#include <QDomDocument>
#include <Qsci/qscistyle.h>
#include <Qsci/qscilexer.h>

#include <QFile>
#include <QMenu>
#include <QMessageBox>
#include <QRegularExpression>
#include <QtDebug>

#include <cmath>

PlainTextEditor::PlainTextEditor(QWidget *parent) : QsciScintilla(parent)
{
    loadConfig();
    connect(&AppConfig::instance(), &AppConfig::configChanged, this, &PlainTextEditor::loadConfig);
    connect(this, &QsciScintilla::linesChanged, this, &PlainTextEditor::adjustLineNumberMargin);
    connect(this, &QsciScintilla::selectionChanged, [this](){
        auto editorLength = SendScintilla(SCI_GETLENGTH);
        SendScintilla(SCI_SETINDICATORCURRENT, 0);
        SendScintilla(SCI_INDICATORCLEARRANGE, 0, editorLength);
        auto word = selectedText();
        if (!word.isEmpty()) {
            int endpos = 0;
            auto startpos = 0;
            startpos = findText(word, SCFIND_WHOLEWORD, startpos, &endpos);
            while(startpos != -1) {
                SendScintilla(SCI_INDICATORFILLRANGE,
                              static_cast<unsigned long>(startpos),
                              endpos - startpos);
                startpos = findText(word, SCFIND_WHOLEWORD, endpos + 1, &endpos);
            }
        }
    });
    connect(this, &PlainTextEditor::modificationChanged, [this]() {
        notifyModifyObservers();
    });
    connect(this, &PlainTextEditor::userListActivated,
            [this](int id, const QString& text) {
        Q_UNUSED(id);
        auto position = static_cast<unsigned long>(SendScintilla(SCI_GETCURRENTPOS));
        auto start = SendScintilla(SCI_WORDSTARTPOSITION, position, true);
        auto end = SendScintilla(SCI_WORDENDPOSITION, position, true);
        SendScintilla(SCI_SETSELECTIONSTART, start);
        SendScintilla(SCI_SETSELECTIONEND, end);
        SendScintilla(SCI_REPLACESEL, textAsBytes(text).constData());
        SendScintilla(SCI_GOTOPOS, start + text.length());
    });

    auto findDialog = new FormFindReplace(this);
    findDialog->hide();

#define _(keycode, functor) do { \
        auto acc = new QAction(this); \
        acc->setShortcut(QKeySequence(keycode)); \
        connect(acc, &QAction::triggered, functor); \
        addAction(acc); \
    } while(0)
    _("ctrl+s", [this]() { save(path()); });
    _("ctrl+r", [this]() { load(path()); });
    _("ctrl+space", [this]() { triggerAutocompletion(); });
    _("ctrl+f", [findDialog]() { findDialog->show(); });
#undef _
}

PlainTextEditor::~PlainTextEditor()
= default;

bool PlainTextEditor::load(const QString &path)
{
    QFile f(path);
    if (f.open(QFile::ReadOnly)) {
        if (read(&f)) {
            setPath(path);
            loadConfig();
            return true;
        }
    }
    return false;
}

bool PlainTextEditor::save(const QString &path)
{
    QFile f(path);
    if (f.open(QFile::WriteOnly)) {
        if (write(&f)) {
            setPath(path);
            setModified(false);
            return true;
        }
    }
    return false;
}

void PlainTextEditor::reload()
{
    QFile f(path());
    if (f.open(QFile::ReadOnly)) {
        auto c = cursor();
        read(&f);
        setCursor(c);
        setModified(false);
    }
}

bool PlainTextEditor::isReadonly() const
{
    return QsciScintilla::isReadOnly();
}

void PlainTextEditor::setReadonly(bool rdOnly)
{
    QsciScintilla::setReadOnly(rdOnly);
}

bool PlainTextEditor::isModified() const
{
    return QsciScintilla::isModified();
}

void PlainTextEditor::setModified(bool m)
{
    QsciScintilla::setModified(m);
}

QPoint PlainTextEditor::cursor() const
{
    int line, col;
    getCursorPosition(&line, &col);
    return {col, line};
}

void PlainTextEditor::setCursor(const QPoint &pos)
{
    setCursorPosition(pos.y() - 1, pos.x());
}

class PlainTextEditorCreator: public IDocumentEditorCreator
{
public:
    ~PlainTextEditorCreator() override;
    bool canHandleMime(const QMimeType &mime) const override {
        return mime.inherits("text/plain");
    }

    IDocumentEditor *create(QWidget *parent = nullptr) const override {
        return new PlainTextEditor(parent);
    }
};
PlainTextEditorCreator::~PlainTextEditorCreator() = default;

IDocumentEditorCreator *PlainTextEditor::creator()
{
    return IDocumentEditorCreator::staticCreator<PlainTextEditorCreator>();
}

QString PlainTextEditor::wordUnderCursor() const
{
    int line, col;
    getCursorPosition(&line, &col);
    return wordAtLineIndex(line, col);
}

void PlainTextEditor::adjustLineNumberMargin()
{
    QFontMetrics m(font());
    setMarginWidth(0, m.width(QString().fill('0', 2 + static_cast<int>(std::log10(lines())))));
}

int PlainTextEditor::findText(const QString &text, int flags, int start, int *targend)
{
    ScintillaBytes s = textAsBytes(text);
    auto endpos = SendScintilla(SCI_GETLENGTH);
    SendScintilla(SCI_SETSEARCHFLAGS, flags);
    SendScintilla(SCI_SETTARGETSTART, start);
    SendScintilla(SCI_SETTARGETEND, endpos);

    auto pos = SendScintilla(SCI_SEARCHINTARGET,
                             static_cast<unsigned long>(s.length()),
                             ScintillaBytesConstData(s));
    if (pos == -1)
        return -1;
    auto targstart = static_cast<int>(SendScintilla(SCI_GETTARGETSTART));
    *targend = static_cast<int>(SendScintilla(SCI_GETTARGETEND));
    return targstart;
}

void PlainTextEditor::closeEvent(QCloseEvent *event)
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
            save(windowFilePath());
            break;
        case QMessageBox::Abort:
            event->ignore();
            break;
        case QMessageBox::No:
            break;
        }
    }
}

void PlainTextEditor::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event);
    createContextualMenu()->exec(event->globalPos());
}

void PlainTextEditor::loadConfigWithStyle(const QString& style, const QFont& editorFont, int tabs, bool tabSpace)
{
    setFont(editorFont);
    if (lexer())
        lexer()->setDefaultFont(editorFont);
    loadStyle(QString(":/styles/%1.xml").arg(style));
    setIndentationsUseTabs(tabSpace);
    setTabWidth(tabs);

    setAutoIndent(true);
    setBraceMatching(StrictBraceMatch);
    resetMatchedBraceIndicator();
    setBackspaceUnindents(true);
    setFolding(CircledTreeFoldStyle);
    setIndentationGuides(true);

    setCaretLineVisible(true);
    setMarginsFont(font());
    setMarginLineNumbers(0, true);
    // setMarginsBackgroundColor(QColor("#cccccc"));

    SendScintilla(SCI_SETMULTIPLESELECTION, 1l, 0l);
    SendScintilla(SCI_SETADDITIONALSELECTIONTYPING, 1l, 0l);

    setMarginSensitivity(1, true);
    markerDefine(QsciScintilla::Circle, SC_MARK_CIRCLE);
    markerDefine(QsciScintilla::RightArrow, SC_MARK_ARROW);
    // setMargins(3);
    setMarkerBackgroundColor(QColor("#1111ee"), SC_MARK_CIRCLE);
    setMarkerBackgroundColor(QColor("#ee1111"), SC_MARK_ARROW);
    setAnnotationDisplay(AnnotationIndented);
    adjustLineNumberMargin();

    SendScintilla(SCI_INDICSETSTYLE, 0, INDIC_ROUNDBOX);
    SendScintilla(SCI_INDICSETFORE, 0, 255);
}

void PlainTextEditor::loadConfig()
{
    auto &conf = AppConfig::instance();
    loadConfigWithStyle(conf.editorStyle(), conf.editorFont(), conf.editorTabWidth(), conf.editorTabsToSpaces());
}

bool PlainTextEditor::loadStyle(const QString &xmlStyleFile)
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
    if (true) {
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

QStringList PlainTextEditor::allWords()
{
    QStringList words;
    QString s = text();
    QRegularExpression re(R"(\b\w+\b)");
    auto mi = re.globalMatch(s);
    while (mi.hasNext())
        words.append(mi.next().captured());
    return words;
}

void PlainTextEditor::triggerAutocompletion()
{
    int _;
    if (!apiContext(int(SendScintilla(SCI_GETCURRENTPOS)), _, _).isEmpty())
        autoCompleteFromDocument();
    else {
        showUserList(1, allWords());
    }
}

QMenu *PlainTextEditor::createContextualMenu()
{
#define _(en, icon, text, keys, functor) do { \
    auto a = m->addAction(QIcon(":/images/actions/" icon ".svg"), text, functor); \
    a->setShortcut(QKeySequence(keys)); \
    a->setEnabled(en); \
} while(0)

    auto m = new QMenu(this);
    bool isSelected = !selectedText().isEmpty();
    bool canPaste = static_cast<bool>(SendScintilla(SCI_CANPASTE));
    _(isUndoAvailable(), "edit-undo", tr("Undo"), "Ctrl+Z", [this]() { undo(); });
    _(isRedoAvailable(), "edit-redo", tr("Redo"), "Ctrl+Shift+Z", [this]() { redo(); });
    m->addSeparator();
    _(isSelected, "edit-cut", tr("Cut"), "Ctrl+X", [this]() { cut(); });
    _(isSelected, "edit-copy", tr("Copy"), "Ctrl+C", [this]() { copy(); });
    _(canPaste, "edit-paste", tr("Paste"), "Ctrl+V", [this]() { paste(); });
    _(isSelected, "edit-delete", tr("Delete"), "DEL", [this]() { removeSelectedText(); });
    m->addSeparator();
    _(true, "edit-select-all", tr("Select All"), "CTRL+A", [this]() { selectAll(); });
#undef _
    return m;
}
