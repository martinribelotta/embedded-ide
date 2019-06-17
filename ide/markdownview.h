#ifndef MARKDOWNVIEW_H
#define MARKDOWNVIEW_H

#include <QTextBrowser>

class MarkdownView : public QTextBrowser
{
    Q_OBJECT
public:
    explicit MarkdownView(QWidget *parent = nullptr);

signals:

public slots:
    void setMarkdown(const QString& markdown);
};

#endif // MARKDOWNVIEW_H
