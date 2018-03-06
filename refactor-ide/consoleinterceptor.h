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

signals:

public slots:
};

#endif // CONSOLEINTERCEPTOR_H
