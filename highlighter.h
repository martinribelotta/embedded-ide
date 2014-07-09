#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit Highlighter(QTextDocument *parent = 0);
    virtual ~Highlighter();

protected:
    void highlightBlock(const QString &text);

private:
    struct Priv_t;
    Priv_t *priv;
};

#endif // HIGHLIGHTER_H
