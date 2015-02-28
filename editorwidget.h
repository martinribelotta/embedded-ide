#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QWidget>

namespace Ui {
class EditorWidget;
}

class QsvColorDefFactory;
class QsvLangDef;
class QsvSyntaxHighlighter;

class EditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EditorWidget(QWidget *parent = 0);
    ~EditorWidget();

public slots:
    void moveCursor(int row, int col);
    bool load(const QString& fileName);
    bool save();

signals:
    void editorError(const QString& error);
    void modified(bool isModify);

private:
    Ui::EditorWidget *ui;
    QsvColorDefFactory *defColors;
    QsvLangDef *langDef;
    QsvSyntaxHighlighter *syntax;
};

#endif // EDITORWIDGET_H
