#include "appconfig.h"
#include "consoleinterceptor.h"
#include "processmanager.h"

#include <QTextCursor>
#include <QTextBrowser>
#include <QGridLayout>
#include <QScrollBar>
#include <QToolButton>

ConsoleInterceptor::ConsoleInterceptor(QTextBrowser *textBrowser, ProcessManager *pman, const QString& pname, QObject *parent) :
    QObject(parent), browser(textBrowser)
{
    const auto size = QSize(16, 16);
    auto gl = new QGridLayout(textBrowser);
    auto bclr = new QToolButton(textBrowser);
    bclr->setIcon(QIcon(":/images/actions/edit-clear.svg"));
    bclr->setAutoRaise(true);
    bclr->setIconSize(size);
    bclr->setToolTip(tr("Clear Console"));
    connect(bclr, &QToolButton::clicked, textBrowser, &QTextBrowser::clear);

    auto bstop = new QToolButton(textBrowser);
    bstop->setEnabled(false);
    bstop->setIcon(QIcon(":/images/actions/window-close.svg"));
    bstop->setAutoRaise(true);
    bstop->setIconSize(size);
    bstop->setToolTip(tr("Stop Current Process"));
    connect(bstop, &QToolButton::clicked, [pman, pname]() { pman->terminate(pname, true, 300); });
    connect(pman->processFor(pname), &QProcess::stateChanged,
            [bstop](QProcess::ProcessState state) { bstop->setEnabled(state == QProcess::Running); });

    gl->addWidget(bclr,  0, 1);
    gl->addWidget(bstop, 0, 2);
    gl->setColumnStretch(0, 1);
    gl->setRowStretch(1, 1);
    gl->setContentsMargins(0, 0, textBrowser->verticalScrollBar()->sizeHint().width(), 0);
    gl->setSpacing(0);

    pman->setStderrInterceptor(pname, [this](QProcess *p, const QString& text) {
        appendToConsole(QProcess::StandardError, p, text);
    });
    pman->setStdoutInterceptor(pname, [this](QProcess *p, const QString& text) {
        appendToConsole(QProcess::StandardError, p, text);
    });
}

ConsoleInterceptor::~ConsoleInterceptor() = default;

void ConsoleInterceptor::writeMessageTo(QTextBrowser *browser, const QString &message, const QColor &color)
{
    auto cursor = browser->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::End);
    QTextCharFormat fmt;
    auto font = AppConfig::instance().loggerFont();
    fmt.setFontFamily(font.family());
    fmt.setFontPointSize(font.pointSize());
    fmt.setForeground(color);
    cursor.setCharFormat(fmt);
    cursor.insertText(message);
    cursor.endEditBlock();
    browser->verticalScrollBar()->setValue(browser->verticalScrollBar()->maximum());
}

void ConsoleInterceptor::writeHtmlTo(QTextBrowser *browser, const QString &html)
{
    auto cursor = browser->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::End);
    cursor.insertHtml(html);
    cursor.endEditBlock();
    browser->verticalScrollBar()->setValue(browser->verticalScrollBar()->maximum());
}

void ConsoleInterceptor::appendToConsole(QProcess::ProcessChannel s, QProcess *p, const QString &text)
{
    const auto& filters = s == QProcess::StandardError? stderrFilters : stdoutFilters;
    QString processedText{ text };
    for(const auto& c: filters)
        processedText = c(p, processedText);
    writeHtml(processedText);
}

void ConsoleInterceptor::writeMessage(const QString &message, const QColor &color)
{
    writeMessageTo(browser, message, color);
}

void ConsoleInterceptor::writeHtml(const QString &html)
{
    writeHtmlTo(browser, html);
}
