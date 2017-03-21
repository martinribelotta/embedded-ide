#include "codeeditor.h"
#include "formfindreplace.h"
#include "Qsci/qsciscintilla.h"
#include "ui_formfindreplace.h"

#include <QKeyEvent>
#include <QMessageBox>

FormFindReplace::FormFindReplace(QsciScintilla *editor) :
    QWidget(editor),
    ui(new Ui::FormFindReplace)
{
    this->editor = editor;
    ui->setupUi(this);
    setAutoFillBackground(true);
    ui->textToFind->addMenuAction(QHash<QString, QString>{
        { tr("Regular Expression"), "regex" },
        { tr("Case Sensitive"), "case" },
        { tr("Wole Words"), "wword" },
        { tr("Selection Only"), "selonly" },
        { tr("Wrap search"), "wrap" },
        { tr("Backward search"), "backward" },
    });
    ui->textToReplace->addMenuAction(QHash<QString, QString>{
        { tr("Replace All"), "replaceAll" }
    });
}

FormFindReplace::~FormFindReplace()
{
    delete ui;
}

void FormFindReplace::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    ui->textToFind->setFocus();
    setProperty("isFirst", true);
}

void FormFindReplace::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
        event->accept();
    } else
        QWidget::keyPressEvent(event);
}

bool FormFindReplace::on_buttonFind_clicked()
{
    bool found = false;
    QString expr = ui->textToFind->text();
    if (property("isFirst").toBool()) {
        bool re = ui->textToFind->isPropertyChecked("regex");
        bool cs = ui->textToFind->isPropertyChecked("case");
        bool wo = ui->textToFind->isPropertyChecked("wword");
        bool wrap = ui->textToFind->isPropertyChecked("wrap");
        bool forward = !ui->textToFind->isPropertyChecked("backward");
        int line = -1;
        int index = -1;
        bool show = true;
        bool posix = false;
        bool selonly = ui->textToFind->isPropertyChecked("selonly");
        if (selonly)
            found = editor->findFirstInSelection(expr, re, cs, wo, forward, show, posix);
        else
            found = editor->findFirst(expr, re, cs, wo, wrap, forward, line, index, show, posix);
        setProperty("isFirst", false);
    } else
        found = editor->findNext();
    QPalette pal = ui->textToFind->palette();
    pal.setBrush(QPalette::Base, found? palette().base() : QBrush(QColor(Qt::red).lighter()));
    ui->textToFind->setPalette(pal);
    return found;
}

void FormFindReplace::on_buttonReplace_clicked()
{
    while (on_buttonFind_clicked()) {
        QString replaceText = ui->textToReplace->text();
        editor->replace(replaceText);
        if (!ui->textToReplace->isPropertyChecked("replaceAll"))
            break;
    }
}

void FormFindReplace::on_textToFind_textChanged(const QString &text)
{
    Q_UNUSED(text);
    int line, pos;
    QPoint p = property("currentPos").toPoint();
    if (!p.isNull()) {
        line = p.x();
        pos = p.y();
        editor->setCursorPosition(line, pos);
    }
    editor->getCursorPosition(&line, &pos);
    setProperty("currentPos", QPoint(line, pos));
    setProperty("isFirst", true);
    on_buttonFind_clicked();
}

void FormFindReplace::on_textToFind_returnPressed()
{
    setProperty("currentPos", QPoint());
    setProperty("isFirst", false);
    on_buttonFind_clicked();
}
