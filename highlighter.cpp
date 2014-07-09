#include "highlighter.h"

#include <QTextCharFormat>

struct HighlightingRule {
    QRegExp pattern;
    QTextCharFormat format;
};

struct Highlighter::Priv_t {
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat preprocessorFormat;
};

//! [0]
Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent), priv(new Priv_t)
{
    HighlightingRule rule;

    priv->keywordFormat.setForeground(Qt::darkBlue);
    priv->keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\balignas\\b"
                    << "\\balignof\\b"
                    << "\\basm\\b"
                    << "\\bauto\\b"
                    << "\\bbool\\b"
                    << "\\bbreak\\b"
                    << "\\bcase\\b"
                    << "\\bcatch\\b"
                    << "\\bchar\\b"
                    << "\\bchar16_t\\b"
                    << "\\bchar32_t\\b"
                    << "\\bclass\\b"
                    << "\\bconst\\b"
                    << "\\bconstexpr\\b"
                    << "\\bconst_cast\\b"
                    << "\\bcontinue\\b"
                    << "\\bdecltype\\b"
                    << "\\bdefault\\b"
                    << "\\bdelete\\b"
                    << "\\bdo\\b"
                    << "\\bdouble\\b"
                    << "\\bdynamic_cast\\b"
                    << "\\belse\\b"
                    << "\\benum\\b"
                    << "\\bexplicit\\b"
                    << "\\bexport\\b"
                    << "\\bextern\\b"
                    << "\\bfalse\\b"
                    << "\\bfloat\\b"
                    << "\\bfor\\b"
                    << "\\bfriend\\b"
                    << "\\bgoto\\b"
                    << "\\bif\\b"
                    << "\\binline\\b"
                    << "\\bint\\b"
                    << "\\blong\\b"
                    << "\\bmutable\\b"
                    << "\\bnamespace\\b"
                    << "\\bnew\\b"
                    << "\\bnoexcept\\b"
                    << "\\bnullptr\\b"
                    << "\\boperator\\b"
                    << "\\bprivate\\b"
                    << "\\bprotected\\b"
                    << "\\bpublic\\b"
                    << "\\bregister\\b"
                    << "\\breinterpret_cast\\b"
                    << "\\breturn\\b"
                    << "\\bshort\\b"
                    << "\\bsigned\\b"
                    << "\\bsizeof\\b"
                    << "\\bstatic\\b"
                    << "\\bstatic_assert\\b"
                    << "\\bstatic_cast\\b"
                    << "\\bstruct\\b"
                    << "\\bswitch\\b"
                    << "\\btemplate\\b"
                    << "\\bthis\\b"
                    << "\\bthread_local\\b"
                    << "\\bthrow\\b"
                    << "\\btrue\\b"
                    << "\\btry\\b"
                    << "\\btypedef\\b"
                    << "\\btypeid\\b"
                    << "\\btypename\\b"
                    << "\\bunion\\b"
                    << "\\bunsigned\\b"
                    << "\\busing\\b"
                    << "\\bvirtual\\b"
                    << "\\bvoid\\b"
                    << "\\bvolatile\\b"
                    << "\\bwchar_t\\b"
                    << "\\bwhile\\b";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = priv->keywordFormat;
        priv->highlightingRules.append(rule);
        //! [0] //! [1]
    }
    //! [1]

    //! [2]
    priv->classFormat.setFontWeight(QFont::Bold);
    priv->classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = priv->classFormat;
    priv->highlightingRules.append(rule);
    //! [2]

    //! [3]
    priv->singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = priv->singleLineCommentFormat;
    priv->highlightingRules.append(rule);

    priv->multiLineCommentFormat.setForeground(Qt::darkGreen);
    //! [3]

    //! [4]
    priv->quotationFormat.setForeground(Qt::magenta);
    rule.pattern = QRegExp("\".*\"");
    rule.format = priv->quotationFormat;
    priv->highlightingRules.append(rule);
    //! [4]

    //! [5]
    priv->functionFormat.setFontItalic(true);
    priv->functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = priv->functionFormat;
    priv->highlightingRules.append(rule);
    //! [5]

    //! [6]
    priv->commentStartExpression = QRegExp("/\\*");
    priv->commentEndExpression = QRegExp("\\*/");

    priv->preprocessorFormat.setFontItalic(true);
    priv->preprocessorFormat.setForeground(Qt::gray);
    rule.pattern = QRegExp("^\\s*\\#[a-zA-Z]+\\s*.*$");
    rule.format = priv->preprocessorFormat;
    priv->highlightingRules.append(rule);
}

Highlighter::~Highlighter()
{
    delete priv;
}
//! [6]

//! [7]
void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, priv->highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    //! [7] //! [8]
    setCurrentBlockState(0);
    //! [8]

    QString trimmedText = text.trimmed();
    if (trimmedText.startsWith('#') && trimmedText.endsWith('\\')) {
        setFormat(0, text.length(), priv->preprocessorFormat);
        setCurrentBlockState(2);
    }

    if (previousBlockState() == 2) {
        setFormat(0, text.length(), priv->preprocessorFormat);
        setCurrentBlockState(trimmedText.endsWith('\\')? 2 : 0);
    }

    //! [9]
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = priv->commentStartExpression.indexIn(text);

    //! [9] //! [10]
    while (startIndex >= 0) {
        //! [10] //! [11]
        int endIndex = priv->commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                    + priv->commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, priv->multiLineCommentFormat);
        startIndex = priv->commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}
//! [11]
