#include "codeeditor.h"

#include <Qsci/qsciapis.h>
#include <Qsci/qscilexer.h>
#include <Qsci/qscilexercpp.h>

#include <QVariant>
#include <QFileInfo>

static bool isFixedPitch(const QFont & font) {
    const QFontInfo fi(font);
    return fi.fixedPitch();
}

static QFont getMonospaceFont(int z = 10){
    QFont font("Monospace");
    font.setPointSize(z);
    if (isFixedPitch(font)) return font;
    font.setStyleHint(QFont::Monospace);
    if (isFixedPitch(font)) return font;
    font.setStyleHint(QFont::TypeWriter);
    if (isFixedPitch(font)) return font;
    font.setFamily("courier");
    if (isFixedPitch(font)) return font;
    return font;
}

static void loadApiFrom(QsciAPIs *api, const QString& name) {
    QFile apiFile(name);
    if (apiFile.open(QFile::ReadOnly)) {
        QString line;
        while(!(line = apiFile.readLine()).isEmpty())
            api->add(line);
    }
}

static void loadApis(QsciAPIs *api) {
    loadApiFrom(api, ":/api/keywords");
    loadApiFrom(api, ":/api/prepro");
}

CodeEditor::CodeEditor(QWidget *parent) :
    QsciScintilla(parent)
{
    initializeEditor();
    setFileName(QString());
    connect(this, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
}

CodeEditor::~CodeEditor()
{
}

QFont CodeEditor::defaultFont()
{
    return getMonospaceFont();
}

CodeEditor::Context_t CodeEditor::context() const
{
    Context_t ctx;
    QFileInfo info(fileName());
    info.makeAbsolute();
    if (info.exists()) {
        ctx["filename"] = info.fileName();
        ctx["path"] = info.path();
        ctx["filepath"] = info.filePath();
        ctx["basename"] = info.baseName();
        ctx["suffix"] = info.suffix();
    }
    return ctx;
}

void CodeEditor::initializeEditor()
{
    setAutoIndent(true);
    setBraceMatching(StrictBraceMatch);
    setIndentationGuides(true);
    setIndentationsUseTabs(false);
    setIndentationWidth(4);
    setEdgeMode(EdgeLine);
    setEdgeColumn(80);
    setAnnotationDisplay(AnnotationBoxed);

    setFont(defaultFont());

    setLexer(new QsciLexerCPP());
    lexer()->setDefaultFont(defaultFont());
    lexer()->setFont(defaultFont());
    lexer()->setAutoIndentStyle(AiClosing|AiOpening);

    setAutoCompletionSource(AcsAll);
    setAutoCompletionThreshold(1);
    QsciAPIs *cApi = new QsciAPIs(lexer());
    loadApis(cApi);
    cApi->prepare();

    setMarginsFont(font());
    setMarginWidth(0, QFontMetrics(font()).width(QString::number(lines())) + 6);
    setMarginLineNumbers(0, true);
    setMarginsBackgroundColor(QColor("#cccccc"));

    setCaretLineVisible(true);
    setCaretLineBackgroundColor(lexer()->defaultPaper().lighter(110));

    FoldStyle state = static_cast<FoldStyle>((!folding()) * 5);
    if (!state)
        foldAll(false);
    setFolding(state);
}

void CodeEditor::onTextChanged()
{
    setMarginWidth(0, fontMetrics().width(QString::number(lines())) + 6);
}
