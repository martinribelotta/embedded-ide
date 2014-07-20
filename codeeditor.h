#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QFile>

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QProcess;
class QCompleter;

class LineNumberArea;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void lineNumberAreaDoubleClick(const QPoint &p);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *e);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void refreshHighlighterLines();
    void updateLineNumberArea(const QRect &, int);
    void sendCurrentCode();
    void insertCompletion(const QString &completion);
    void completionDone();

public slots:
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

private:
    QTextCursor textUnderCursor() const;
    QString lineUnderCursor() const;
    void completionActivate();
    void completionShow();

    QWidget *lineNumberArea;
    QCompleter *m_completer;
    QString m_documentFile;
    QProcess *completionProc;
    QHash<int, QColor> highlightLines;
};

#endif // CODEEDITOR_H
