#include "plaintexteditor.h"

#include <QDomDocument>
#include <Qsci/qscistyle.h>
#include <Qsci/qscilexer.h>

#include <QFile>
#include <QtDebug>

PlainTextEditor::PlainTextEditor(QWidget *parent) : QsciScintilla(parent)
{
    loadConfig();
    connect(this, &QsciScintilla::linesChanged, this, &PlainTextEditor::adjustLineNumberMargin);
    connect(this, &QsciScintilla::selectionChanged, [this](){
        int editorLength = SendScintilla(SCI_GETLENGTH);
        SendScintilla(SCI_SETINDICATORCURRENT, 0);
        SendScintilla(SCI_INDICATORCLEARRANGE, 0, editorLength);
        auto word = selectedText();
        if (!word.isEmpty()) {
            int endpos = 0;
            int startpos = 0;
            startpos = findText(word, SCFIND_WHOLEWORD, startpos, &endpos);
            while(startpos != -1) {
                SendScintilla(SCI_INDICATORFILLRANGE, startpos, endpos - startpos);
                startpos = findText(word, SCFIND_WHOLEWORD, endpos + 1, &endpos);
            }
        }
    });
}

PlainTextEditor::~PlainTextEditor()
{

}

bool PlainTextEditor::load(const QString &path)
{
    QFile f(path);
    if (f.open(QFile::ReadOnly)) {
        if (read(&f)) {
            setWindowFilePath(path);
            return true;
        }
    }
    return false;
}

bool PlainTextEditor::save(const QString &path)
{
    QFile f(path);
    if (f.open(QFile::WriteOnly))
        return write(&f);
    else
        return false;
}

QString PlainTextEditor::path() const
{
    return windowFilePath();
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
    return QPoint(col, line);
}

void PlainTextEditor::setCursor(const QPoint &pos)
{
    setCursorPosition(pos.y(), pos.x());
}

class PlainTextEditorCreator: public IDocumentEditorCreator
{
public:
    bool canHandleMime(const QMimeType &mime) const override {
        return mime.inherits("text/plain");
    }

    IDocumentEditor *create(QWidget *parent = nullptr) const override {
        return new PlainTextEditor(parent);
    }
};

IDocumentEditorCreator *PlainTextEditor::creator()
{
    IDocumentEditorCreator *staticCreator = nullptr;
    if (!staticCreator)
        staticCreator = new PlainTextEditorCreator();
    return staticCreator;
}

void PlainTextEditor::adjustLineNumberMargin()
{
    QFontMetrics m(font());
    setMarginWidth(0, m.width(QString().fill('0', 2 + static_cast<int>(std::log10(lines())))));
}

int PlainTextEditor::findText(const QString &text, int flags, int start, int *targend)
{
    ScintillaBytes s = textAsBytes(text);
    int endpos = SendScintilla(SCI_GETLENGTH);
    SendScintilla(SCI_SETSEARCHFLAGS, flags);
    SendScintilla(SCI_SETTARGETSTART, start);
    SendScintilla(SCI_SETTARGETEND, endpos);

    int pos = SendScintilla(SCI_SEARCHINTARGET, s.length(), ScintillaBytesConstData(s));
    if (pos == -1)
        return -1;
    long targstart = SendScintilla(SCI_GETTARGETSTART);
    *targend = SendScintilla(SCI_GETTARGETEND);
    return targstart;
}

void PlainTextEditor::loadConfig()
{
    QFont fonts("Monospace");
    fonts.setPointSize(10);
    setFont(fonts);

    loadStyle(":/styles/Default.xml");

    setIndentationsUseTabs(false);
    setTabWidth(4);
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
