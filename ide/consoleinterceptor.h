#ifndef CONSOLEINTERCEPTOR_H
#define CONSOLEINTERCEPTOR_H

#include <QObject>
#include <QProcess>

#include <functional>

class QTextBrowser;
class QProcess;

class ProcessManager;

using ConsoleInterceptorFilter = std::function<QString& (QProcess *p, QString& s)>;

class ConsoleInterceptor : public QObject
{
    Q_OBJECT
public:

    explicit ConsoleInterceptor(QTextBrowser *textBrowser, ProcessManager *pman, const QString &pname, QObject *parent = nullptr);
    virtual ~ConsoleInterceptor();

    static void writeMessageTo(QTextBrowser *browser, const QString& message, const QColor& color);
    static void writeHtmlTo(QTextBrowser *browser, const QString& html);

    void addStdOutFilter(const ConsoleInterceptorFilter& f) { stdoutFilters.append(f); }
    void addStdErrFilter(const ConsoleInterceptorFilter& f) { stderrFilters.append(f); }

    void appendToConsole(QProcess::ProcessChannel s, QProcess *p, const QString& text);
signals:

public slots:
    void writeMessage(const QString& message, const QColor& color);
    void writeHtml(const QString& html);

private:
    QTextBrowser *browser;
    QList<ConsoleInterceptorFilter> stdoutFilters;
    QList<ConsoleInterceptorFilter> stderrFilters;
};

#endif // CONSOLEINTERCEPTOR_H
