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
#ifndef REGEXHTMLTRANSLATOR_H
#define REGEXHTMLTRANSLATOR_H

#include <QColor>
#include <QList>
#include <QRegularExpression>

class QProcess;

struct RegexEntry {
    QRegularExpression re;
    QLatin1String html;
};

using RegexList_t = QList<RegexEntry>;

class RegexHTMLTranslator
{
public:

    RegexHTMLTranslator(const RegexList_t& reList) : regexs(reList) { }
    RegexHTMLTranslator() : regexs(DEFAULT_REGEX) { }

    QString& operator()(QProcess *p, QString& s);

    static const RegexList_t DEFAULT_REGEX;
    static RegexHTMLTranslator CONSOLE_TRANSLATOR;
private:
    RegexList_t regexs;
};

#endif // REGEXHTMLTRANSLATOR_H
