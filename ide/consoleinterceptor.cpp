/*
 * This file is part of Embedded-IDE
 * 
 * Copyright 2019 Martin Ribelotta <martinribelotta@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "appconfig.h"
#include "consoleinterceptor.h"
#include "processmanager.h"

#include <QTextCursor>
#include <QTextBrowser>
#include <QGridLayout>
#include <QScrollBar>
#include <QToolButton>

ConsoleInterceptor::ConsoleInterceptor(QTextBrowser *textBrowser, QObject *parent) :
    QObject(parent), browser(textBrowser)
{
    const auto size = QSize(16, 16);
    auto gl = new QGridLayout(textBrowser);
    m_clearButton = new QToolButton(textBrowser);
    m_clearButton->setIcon(QIcon(AppConfig::resourceImage({ "actions", "edit-clear" })));
    m_clearButton->setAutoRaise(true);
    m_clearButton->setIconSize(size);
    m_clearButton->setToolTip(tr("Clear Console"));

    m_killButton = new QToolButton(textBrowser);
    m_killButton->setEnabled(false);
    m_killButton->setIcon(QIcon(AppConfig::resourceImage({ "actions", "window-close" })));
    m_killButton->setAutoRaise(true);
    m_killButton->setIconSize(size);
    m_killButton->setToolTip(tr("Stop Current Process"));

    gl->addWidget(m_clearButton,  0, 1);
    gl->addWidget(m_killButton, 0, 2);
    gl->setColumnStretch(0, 1);
    gl->setRowStretch(1, 1);
    gl->setContentsMargins(0, 0, textBrowser->verticalScrollBar()->sizeHint().width(), 0);
    gl->setSpacing(0);
    textBrowser->setFont(QFont("Courier"));
    connect(&AppConfig::instance(), &AppConfig::configChanged, [textBrowser](AppConfig *conf) {
        textBrowser->setFont(conf->loggerFont());
    });
}

ConsoleInterceptor::~ConsoleInterceptor() {}

void ConsoleInterceptor::writeMessageTo(QTextBrowser *browser, const QString &message, const QColor &color)
{
    QTextCharFormat fmt;
    fmt.setForeground(color.isValid()? color : browser->palette().text().color());
    writeMessageTo(browser, message, fmt);
}

void ConsoleInterceptor::writeMessageTo(QTextBrowser *browser, const QString &message, const QTextCharFormat &fmt)
{
    auto cursor = browser->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::End);
    auto font = AppConfig::instance().loggerFont();
    QTextCharFormat fmt2(fmt);
    fmt2.setFontFamily(font.family());
    fmt2.setFontPointSize(font.pointSize());
    cursor.setCharFormat(fmt2);
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
        if (c(browser, processedText))
            return;
    writeMessage(processedText, browser->palette().text().color());
}

void ConsoleInterceptor::writeMessage(const QString &message, const QColor &color)
{
    writeMessageTo(browser, message, color);
}

void ConsoleInterceptor::writeFmtMessage(const QString &message, const QTextCharFormat &fmt)
{
    writeMessageTo(browser, message, fmt);
}

void ConsoleInterceptor::writeHtml(const QString &html)
{
    writeHtmlTo(browser, html);
}
