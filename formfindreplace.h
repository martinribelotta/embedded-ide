#ifndef FORMFINDREPLACE_H
#define FORMFINDREPLACE_H

#include <QWidget>

namespace Ui {
class FormFindReplace;
}

class QsciScintilla;

class FormFindReplace : public QWidget
{
    Q_OBJECT

public:
    explicit FormFindReplace(QsciScintilla *editor);
    ~FormFindReplace();

protected:
    void showEvent(QShowEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    bool on_buttonFind_clicked();

    void on_buttonReplace_clicked();

    void on_textToFind_textChanged(const QString &text);

    void on_textToFind_returnPressed();

private:
    Ui::FormFindReplace *ui;
    QsciScintilla *editor;
};

#endif // FORMFINDREPLACE_H
