#include "consoleinterceptor.h"
#include "processmanager.h"

#include <QTextCursor>
#include <QTextBrowser>
#include <QGridLayout>
#include <QScrollBar>
#include <QToolButton>

ConsoleInterceptor::ConsoleInterceptor(QTextBrowser *textBrowser, ProcessManager *pman, const QString& pname, QObject *parent) :
    QObject(parent)
{
    connect(textBrowser, &QTextBrowser::textChanged,
            [textBrowser]() { textBrowser->verticalScrollBar()->setValue(textBrowser->verticalScrollBar()->maximum()); });

    const auto size = QSize(16, 16);
    auto gl = new QGridLayout(textBrowser);
    auto bclr = new QToolButton(textBrowser);
    bclr->setIcon(QIcon(":/images/actions/edit-clear.svg"));
    bclr->setAutoRaise(true);
    bclr->setIconSize(size);
    connect(bclr, &QToolButton::clicked, textBrowser, &QTextBrowser::clear);

    auto bstop = new QToolButton(textBrowser);
    bstop->setEnabled(false);
    bstop->setIcon(QIcon(":/images/actions/window-close.svg"));
    bstop->setAutoRaise(true);
    bstop->setIconSize(size);
    connect(bstop, &QToolButton::clicked, [pman, pname]() { pman->terminate(pname, true, 300); });
    connect(pman->processFor(pname), &QProcess::stateChanged,
            [bstop](QProcess::ProcessState state) { bstop->setEnabled(state == QProcess::Running); });

    gl->addWidget(bclr,  0, 1);
    gl->addWidget(bstop, 0, 2);
    gl->setColumnStretch(0, 1);
    gl->setRowStretch(1, 1);
    auto rMargin = textBrowser->verticalScrollBar()->sizeHint().width();
    gl->setContentsMargins(0, 0, rMargin, 0);
    gl->setSpacing(0);

    auto consoleText = [textBrowser](const QString &text, const QColor& color) {
        auto cursor = textBrowser->textCursor();
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::End);
        QTextCharFormat fmt;
        fmt.setFontFamily("monospace");
        fmt.setFontPointSize(10);
        fmt.setForeground(color);
        cursor.setCharFormat(fmt);
        cursor.insertText(text);
        cursor.endEditBlock();
    };
    pman->setStderrInterceptor(pname, [consoleText](QProcess *, const QString& text) { consoleText(text, Qt::red); });
    pman->setStdoutInterceptor(pname, [consoleText](QProcess *, const QString& text) { consoleText(text, Qt::blue); });
}

ConsoleInterceptor::~ConsoleInterceptor()
{
}
