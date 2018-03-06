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
    explicit FormFindReplace(QsciScintilla *ed);
    virtual ~FormFindReplace();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

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
