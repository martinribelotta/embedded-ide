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
#ifndef MARKDOWNVIEW_H
#define MARKDOWNVIEW_H

#include <QTextBrowser>

class MarkdownView : public QTextBrowser
{
    Q_OBJECT
public:
    explicit MarkdownView(QWidget *parent = nullptr);

    static QString renderHtml(const QString& markdownText);

signals:

public slots:
    void setMarkdown(const QString& markdown);
};

#endif // MARKDOWNVIEW_H
