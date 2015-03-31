#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QFile>

#include "makefileinfo.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QCompleter;

class LineNumberArea;

class QsvColorDefFactory;
class QsvLangDef;
class QsvSyntaxHighlighter;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void lineNumberAreaDoubleClick(const QPoint &p);
    int lineNumberAreaWidth();
    const MakefileInfo *makefileInfo() const { return mk; }

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *e);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &, int);

public slots:
    void insertCompletion(const QString &completion);
    void codeContextUpdate(const QStringList& list);

    void setMakefileInfo(const MakefileInfo *mk) { this->mk = mk; }

    void moveTextCursor(int row, int col);

    void refreshHighlighterLines();

    bool load(const QString &fileName);
    bool save();

    void addHighlightLine(int line, const QColor& c) {
        highlightLines.insert(line, c);
        refreshHighlighterLines();
    }

    void delHighlightLine(int line) {
        highlightLines.remove(line);
        refreshHighlighterLines();
    }

    void clearHighlightLines() {
        highlightLines.clear();
        refreshHighlighterLines();
    }

    void toggleHighlightLine(int line, const QColor& c) {
        if (highlightLines.contains(line))
            delHighlightLine(line);
        else
            addHighlightLine(line, c);
    }

signals:
    void fileError(const QString& errorText);
    void updateCodeContext();

private:
    QTextCursor textUnderCursor() const;
    QString lineUnderCursor() const;
    void completionShow();

    QWidget *lineNumberArea;
    QCompleter *m_completer;
    QString m_documentFile;
    QHash<int, QColor> highlightLines;
    QsvColorDefFactory *defColors;
    QsvLangDef *langDef;
    QsvSyntaxHighlighter *syntax;
    const MakefileInfo *mk;
};

#endif // CODEEDITOR_H
