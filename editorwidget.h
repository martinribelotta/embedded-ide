#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QWidget>
#include <QFile>

namespace Ui {
class EditorWidget;
}

class EditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EditorWidget(QWidget *parent = 0);
    ~EditorWidget();

public slots:
    bool load(const QString& fileName);
    bool save();

signals:
    void editorError(const QString& error);

private:
    Ui::EditorWidget *ui;
};

#endif // EDITORWIDGET_H
