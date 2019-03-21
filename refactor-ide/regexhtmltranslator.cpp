#include "regexhtmltranslator.h"

#include <QtDebug>

const RegexList_t RegexHTMLTranslator::DEFAULT_REGEX =
    {
#define _(r, h) RegexEntry{ \
        QRegularExpression{ r, QRegularExpression::MultilineOption }, \
        QLatin1String{ h } \
    }

    _(R"((\r?\n))", "\\1<br>"),
    _(R"( )", "&nbsp;"),
    _(R"(^(\<br\>)?(.*?):(\d+):(\d+)?(:?)(.*?)(\<br\>)?$)",
      R"(\1<font color="red">\2:\3:\4\5 <a href="file:\2#\3#\4">\6</a></font>\7)"),

#undef _
    };

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
