#include "markdownview.h"

#include <maddy/parser.h>

MarkdownView::MarkdownView(QWidget *parent) : QTextBrowser(parent)
{
}

void MarkdownView::setMarkdown(const QString &markdown)
{
    maddy::Parser parser;
    std::stringstream ss(markdown.toStdString());
    auto htmlText = QString::fromStdString(parser.Parse(ss));
    setHtml(htmlText);
}
