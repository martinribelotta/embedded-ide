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
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *e);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void insertCompletion(const QString &completion);
    void completionDone();

public slots:
    bool load(const QString &fileName);
    bool save();

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
};

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

private:
    CodeEditor *codeEditor;
};

#endif // CODEEDITOR_H
