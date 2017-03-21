#ifndef CODEEDITOR_H
#define CODEEDITOR_H

// #include <QPlainTextEdit>
#include <qscintilla/Qt4Qt5/Qsci/qsciscintilla.h>
#include <QFile>

#include "makefileinfo.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QCompleter;

class FormFindReplace;

class QsvColorDefFactory;
class QsvLangDef;
class QsvSyntaxHighlighter;
class QsvTextOperationsWidget;

class CodeEditor : public QsciScintilla //QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = 0);

    const MakefileInfo *makefileInfo() const { return mk; }

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void closeEvent(QCloseEvent *event);

public slots:
    void insertCompletion(const QString &completion);
    void codeContextUpdate(const QStringList& list);

    void setMakefileInfo(const MakefileInfo *mk) { this->mk = mk; }

    void moveTextCursor(int row, int col);

    void refreshHighlighterLines();

    void loadConfig();
    bool load(const QString &fileName);
    bool save();
    void reload();

    void clearSelection();
    void findTagUnderCursor();

    int getIp() const { return ip; }
    void clearDebugPointer() { setDebugPointer(-1, Qt::black); }
    void setDebugPointer(int line, const QColor& c) {
        if (highlightLines.contains(ip)) {
            highlightLines.remove(ip);
        }
        ip = line;
        if (ip != -1) {
            highlightLines.insert(ip, c);
        }
        refreshHighlighterLines();
    }

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
    void widgetResized();
    void requireOpen(const QString& file, int row, int col, const MakefileInfo *mk);
    void requestForSave(CodeEditor *sender);

private:
    QString wordUnderCursor() const;
    QString lineUnderCursor() const;
    void completionShow();
    QMenu *createContextMenu();
    QRect cursorRect() const;

    FormFindReplace *replaceDialog;
    QCompleter *m_completer;
    QString m_documentFile;
    QHash<int, QColor> highlightLines;
    const MakefileInfo *mk;
    int ip;
};

#endif // CODEEDITOR_H
