#ifndef CONSOLEINTERCEPTOR_H
#define CONSOLEINTERCEPTOR_H

#include <QObject>

class QTextBrowser;

class ProcessManager;

class ConsoleInterceptor : public QObject
{
    Q_OBJECT
public:
    explicit ConsoleInterceptor(QTextBrowser *textBrowser, ProcessManager *pman, const QString &pname, QObject *parent = nullptr);
    virtual ~ConsoleInterceptor();

    static void writeMessageTo(QTextBrowser *browser, const QString& message, const QColor& color);
signals:

public slots:
    void writeMessage(const QString& message, const QColor& color);

private:
    QTextBrowser *browser;
};

#endif // CONSOLEINTERCEPTOR_H
