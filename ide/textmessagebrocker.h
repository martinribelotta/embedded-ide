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
#ifndef TEXTMESSAGEBROCKER_H
#define TEXTMESSAGEBROCKER_H

#include <QObject>

namespace TextMessages {
constexpr auto STDERR_LOG = "stderrLog";
constexpr auto STDOUT_LOG = "stdoutLog";
constexpr auto ACTION_LABEL = "actionLabel";
constexpr auto DEBUG_IP_CHANGE = "debug_ip_change";
};

class TextMessageBrocker : public QObject
{
    Q_OBJECT

private:
    explicit TextMessageBrocker(QObject *parent = nullptr);

public:
    static TextMessageBrocker &instance();

    template<typename Function>
    TextMessageBrocker& subscribe(const QString& topic, Function func) {
        connect(this, &TextMessageBrocker::published,
                [this, topic, func](const QString& t, const QString& msg)
        {
            if (topic == t)
                func(msg);
        });
        return *this;
    }

    template<typename Class, typename Function>
    TextMessageBrocker& subscribe(const QString& topic, Class obj, Function func) {
        connect(this, &TextMessageBrocker::published,
                [this, topic, obj, func](const QString& t, const QString& msg)
        {
            if (topic == t)
                (obj->*func)(msg);
        });
        return *this;
    }

signals:
    void published(const QString& topic, const QString& message);

public slots:
    void publish(const QString& topic, const QString& message);
};

#endif // TEXTMESSAGEBROCKER_H
