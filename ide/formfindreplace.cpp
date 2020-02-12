/*
 * This file is part of Embedded-IDE
 * 
 * Copyright 2019 Martin Ribelotta <martinribelotta@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "formfindreplace.h"
#include "Qsci/qsciscintilla.h"
#include "appconfig.h"

#include "ui_formfindreplace.h"

#include <QKeyEvent>
#include <QMessageBox>
#include <QtDebug>

FormFindReplace::FormFindReplace(QsciScintilla *ed) :
    QWidget(ed->viewport()),
    ui(std::make_unique<Ui::FormFindReplace>()),
    editor(ed)
{
    ui->setupUi(this);
    const struct { QAbstractButton *b; const char *name; } buttonmap[]={
        { ui->buttonFind, "edit-find" },
        { ui->buttonFindPrev_2, "dialog-close" },
        { ui->buttonReplace, "edit-find-replace" },
    };
    for (const auto& e: buttonmap)
        e.b->setIcon(QIcon{AppConfig::resourceImage({ "actions", e.name })});

    ui->textToFind->addMenuActions({
        { tr("Regular Expression"), "regex" },
        { tr("Case Sensitive"), "case" },
        { tr("Wole Words"), "wword" },
        { tr("Selection Only"), "selonly" },
        { tr("Wrap search"), "wrap" },
        { tr("Backward search"), "backward" },
    });
    ui->textToFind->setPropertyChecked("wrap", true);
    ui->textToReplace->addMenuActions({ { tr("Replace All"), "replaceAll" } });
    connect(ui->textToFind, &FindLineEdit::menuActionClicked, [this]() { setProperty("isFirst", true); });
    auto layout = new QGridLayout(ed->viewport());
    layout->setRowStretch(0, 1);
    layout->addWidget(this, 1, 0);
    layout->setMargin(0);
    layout->setContentsMargins(0, 0, 0, 0);
    editor->installEventFilter(this);
}

FormFindReplace::~FormFindReplace()
{
}

void FormFindReplace::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    ui->textToFind->setFocus();
    ui->textToFind->setText(editor->selectedText());
    setProperty("isFirst", true);
}

void FormFindReplace::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    editor->setFocus();
}

void FormFindReplace::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
        event->accept();
    } else
        QWidget::keyPressEvent(event);
}

bool FormFindReplace::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)
    switch (event->type()) {
    case QEvent::KeyPress:
        if (isVisible()) {
            if (!ui->textToFind->hasFocus())
                ui->textToFind->setFocus();
            this->event(event);
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

bool FormFindReplace::on_buttonFind_clicked()
{
    auto found = false;
    auto expr = ui->textToFind->text();
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
    auto pal = ui->textToFind->palette();
    pal.setBrush(QPalette::Base, found? palette().base() : QBrush(QColor(Qt::red).lighter()));
    ui->textToFind->setPalette(pal);
    return found;
}

void FormFindReplace::on_buttonReplace_clicked()
{
    while (on_buttonFind_clicked()) {
        auto replaceText = ui->textToReplace->text();
        editor->replace(replaceText);
        if (!ui->textToReplace->isPropertyChecked("replaceAll"))
            break;
    }
}

void FormFindReplace::on_textToFind_textChanged(const QString &text)
{
    Q_UNUSED(text)
    int line;
    int pos;
    auto p = property("currentPos").toPoint();
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
