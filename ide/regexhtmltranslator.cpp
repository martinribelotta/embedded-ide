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
#include "regexhtmltranslator.h"

#include <QtDebug>

const RegexList_t RegexHTMLTranslator::DEFAULT_REGEX =
    {
#define _(r, h) RegexEntry{ \
        QRegularExpression{ r, QRegularExpression::MultilineOption }, \
        QLatin1String{ h } \
    }

    _(R"((?
))", "\1<br>"),
    _(R"( )", "&nbsp;"),
    _(R"(^(\<br\>)?(.*?):(\d+):(\d+)?(:?)(.*?)(\<br\>)?$)",
      R"(\1<font color="red">\2:\3:\4\5 <a href="file:\2#\3#\4">\6</a></font>\7)"),

#undef _
    };

RegexHTMLTranslator RegexHTMLTranslator::CONSOLE_TRANSLATOR{};

QString &RegexHTMLTranslator::operator()(QProcess *p, QString &s)
{
    Q_UNUSED(p);
    s = s.toHtmlEscaped();
    for(const auto& e: regexs) {
        // qDebug() << "AFTER" << s;
        s.replace(e.re, e.html);
        // qDebug() << "BEFORE" << s;
    }
    return s;
}
