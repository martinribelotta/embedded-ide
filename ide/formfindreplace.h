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
    virtual ~FormFindReplace() override;

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
