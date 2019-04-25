/*
 * This file is part of Embedded-IDE
 * 
 * Copyright 2019 Martin Ribelotta <martinribelotta@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "appconfig.h"
#include "formfindreplace.h"
#include "plaintexteditor.h"
#include "textmessagebrocker.h"

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

    TextMessageBrocker::instance().subscribe("", [this](const QString& msg) {
        QRegularExpression re(R"((.+?)\:(\d+))");
        auto m = re.match(msg);
        if (m.hasMatch()) {
            auto file = m.captured(1);
            auto line = m.captured(2).toInt();
            this->setCursor(QPoint{ line, 0 });
            if (file == this->path()) {
                markerAdd(line, SC_MARK_BACKGROUND);
            } else {
                markerDeleteAll(SC_MARK_BACKGROUND);
            }
        }
    });

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

PlainTextEditor::~PlainTextEditor() = default;

// NPP detect indent from nppIndenture writed by Evan King
// https://github.com/evan-king/nppIndenture
// Licenced by GPL-3.0
namespace npp_detectident {

constexpr auto MIN_INDENT = 2; // minimum width of a single indentation
constexpr auto MAX_INDENT = 8; // maximum width of a single indentation

constexpr auto MIN_DEPTH = MIN_INDENT; // ignore lines below this indentation level
constexpr auto MAX_DEPTH = 3*MAX_INDENT; // ignore lines beyond this indentation level

// % of lines allowed to contradict indentation option without penalty
constexpr auto GRACE_FREQUENCY = 1/50.0f;

struct ParseResult {
    int num_tab_lines = 0;
    int num_space_lines = 0;
    float grace = 0.0f;

    // indentation => count(lines of that exact indentation)
    int depth_counts[MAX_DEPTH+1] = {0};
};

ParseResult parseDocument(QsciScintilla *doc) {
    ParseResult result;

    const int num_lines = doc->lines();
    result.grace = float(num_lines) * GRACE_FREQUENCY;
    for(int i = 0; i < num_lines; ++i) {
        const int depth = doc->indentation(i);
        if(depth < MIN_DEPTH || depth > MAX_DEPTH) continue;

        auto pos = doc->SendScintilla(QsciScintilla::SCI_POSITIONFROMLINE, i);
        auto lineHeadChar = doc->SendScintilla(QsciScintilla::SCI_GETCHARAT, pos);

        if(lineHeadChar == '\t') result.num_tab_lines++;

        if(lineHeadChar == ' ') {
            result.num_space_lines++;
            result.depth_counts[depth]++;
        }
    }

    return result;
}

struct IndentInfo {
    enum class IndentType { Invalid, Space, Tab };

    IndentType type = IndentType::Invalid;
    int num = 0;
};

IndentInfo detectIndentInfo(QsciScintilla *doc) {

    const ParseResult result = parseDocument(doc);
    IndentInfo info;

    // decide `type`
    if(result.num_tab_lines + result.num_space_lines == 0)
        info.type = IndentInfo::IndentType::Invalid;
    else if(result.num_space_lines > (result.num_tab_lines * 4))
        info.type = IndentInfo::IndentType::Space;
    else if(result.num_tab_lines > (result.num_space_lines * 4))
        info.type = IndentInfo::IndentType::Tab;
    /*else
        {
            const auto sci = plugin.getSciCallFunctor();
            const bool useTab = sci.call<bool>(SCI_GETUSETABS);
            info.type = useTab ? IndentType::Tab : IndentType::Space;
        }*/

    // decide `num`
    if(info.type == IndentInfo::IndentType::Space) {

        // indent size => count(space-indented lines with incompatible indentation)
        int margins[MAX_INDENT+1] = {0};

        // for each depth option, count the incompatible lines
        for(int i = MIN_DEPTH; i <= MAX_DEPTH; i++) {
            for(int k = MIN_INDENT; k <= MAX_INDENT; k++) {
                if(i % k == 0) continue;
                margins[k] += result.depth_counts[i];
            }
        }

        // choose the last indent with the smallest margin (ties go to larger indent)
        // Considers margins within grace of zero as =zero,
        // so occasional typos don't force smaller indentation
        int which = MIN_INDENT;
        for(int i = MIN_INDENT; i <= MAX_INDENT; ++i) {
            if(result.depth_counts[i] == 0) continue;
            if(margins[i] <= margins[which] || margins[i] < result.grace) which = i;
        }

        info.num = which;
    }

    return info;
}

}

bool PlainTextEditor::load(const QString &path)
{
    QFile f(path);
    if (f.open(QFile::ReadOnly)) {
        if (read(&f)) {
            setPath(path);
            loadConfig();
            if (AppConfig::instance().editorDetectIdent()) {
                auto info = npp_detectident::detectIndentInfo(this);
                if (info.type == npp_detectident::IndentInfo::IndentInfo::IndentType::Tab) {
                    setIndentationsUseTabs(true);
                } else if (info.type == npp_detectident::IndentInfo::IndentInfo::IndentType::Space) {
                    setIndentationsUseTabs(false);
                    setIndentationWidth(info.num);
                } else {
                    // Nothing to do... fallback to config
                }
            }
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

void PlainTextEditor::loadConfigWithStyle(const QString& style, const QFont& editorFont, int tabs, bool tabsToSpace)
{
    setFont(editorFont);
    if (lexer()) {
        lexer()->setDefaultFont(editorFont);
        lexer()->setFont(editorFont);
    }
    loadStyle(QString(":/styles/%1.xml").arg(style));
    setIndentationsUseTabs(!tabsToSpace);
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
    markerDefine(QsciScintilla::Background, SC_MARK_BACKGROUND);
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
    int eLine, eCol;
    if (!doc.setContent(&file, &errorMsg, &eLine, &eCol)) {
        qDebug() << "cannot load" << xmlStyleFile
                 << "style:" << errorMsg
                 << "at" << eLine << ":" << eCol;
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
                } else if (name == "Default Style") {
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
