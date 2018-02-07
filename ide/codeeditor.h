#ifndef CODEEDITOR_H
#define CODEEDITOR_H

// #include <QPlainTextEdit>
#include <Qsci/qsciscintilla.h>
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
    void formatSelection();

    void insertCompletion(const QString &completion);

    void codeContextUpdate(const QStringList& list);

    void setMakefileInfo(const MakefileInfo *mk) { this->mk = mk; }

    void moveTextCursor(int row, int col);

    void loadConfig();
    bool loadStyle(const QString& xmlStyleFile);
    bool load(const QString &fileName);
    bool save();
    void reload();

    void clearSelection();
    void findTagUnderCursor();

    int getDebugPointer() const { return ip; }
    void clearDebugPointer() { setDebugPointer(-1); }
    void setDebugPointer(int line);

private slots:
    void adjustLineNumberMargin();
    void textSelected();

signals:
    void fileError(const QString& errorText);
    void updateCodeContext();
    void requireOpen(const QString& file, int row, int col, const MakefileInfo *mk);
    void requestForSave(CodeEditor *sender);

private:
    QString wordUnderCursor() const;
    QString lineUnderCursor() const;
    void completionShow();
    QMenu *createContextMenu();
    QRect cursorRect() const;
    int findText(const QString& text, int flags, int start, int *targend);

    FormFindReplace *replaceDialog;
    QCompleter *m_completer;
    QString m_documentFile;
    QHash<int, int> breakPointMarkToLine;
    const MakefileInfo *mk;
    int ip;
};

QString stylePath(const QString& styleName);

#endif // CODEEDITOR_H
