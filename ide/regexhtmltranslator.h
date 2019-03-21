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
private:
    RegexList_t regexs;
};

#endif // REGEXHTMLTRANSLATOR_H
