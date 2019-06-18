#include "markdownview.h"

#include <3rdpart/hoedown/src/document.h>
#include <3rdpart/hoedown/src/html.h>

static constexpr auto DEFAULT_STYLESHEET = R"(
pre {
    background: #ffe4a4;
    padding: 10px;
}
code {
    background: #ffe4a4;
}
)";

MarkdownView::MarkdownView(QWidget *parent) : QTextBrowser(parent)
{
}

QString MarkdownView::renderHtml(const QString &markdownText)
{
    auto utf8 = markdownText.toUtf8();
    auto ptr = reinterpret_cast<const uint8_t*>(utf8.data());
    hoedown_html_flags flags = HOEDOWN_HTML_USE_XHTML;
    hoedown_extensions exts = static_cast<hoedown_extensions>(
            HOEDOWN_EXT_TABLES |
            HOEDOWN_EXT_FENCED_CODE |
            HOEDOWN_EXT_FOOTNOTES |
            HOEDOWN_EXT_AUTOLINK |
            HOEDOWN_EXT_STRIKETHROUGH |
            HOEDOWN_EXT_UNDERLINE |
            HOEDOWN_EXT_HIGHLIGHT |
            HOEDOWN_EXT_QUOTE |
            HOEDOWN_EXT_SUPERSCRIPT |
            HOEDOWN_EXT_MATH |
            HOEDOWN_EXT_NO_INTRA_EMPHASIS |
            HOEDOWN_EXT_SPACE_HEADERS |
            HOEDOWN_EXT_MATH_EXPLICIT
        );
    auto renderer = hoedown_html_renderer_new(flags, 0);
    auto document = hoedown_document_new(renderer, exts, 16);
    auto html = hoedown_buffer_new(size_t(utf8.size()));
    hoedown_document_render(document, html, ptr, size_t(utf8.size()));
    auto htmlText = QString::fromUtf8(reinterpret_cast<const char*>(html->data),
                                      int(html->size));
    hoedown_buffer_free(html);
    hoedown_document_free(document);
    hoedown_html_renderer_free(renderer);
    return htmlText;
}

void MarkdownView::setMarkdown(const QString &markdown)
{
    setHtml(renderHtml(markdown));
    document()->setDefaultStyleSheet(DEFAULT_STYLESHEET);
}
