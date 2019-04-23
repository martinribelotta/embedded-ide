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
