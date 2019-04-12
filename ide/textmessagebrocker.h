#ifndef TEXTMESSAGEBROCKER_H
#define TEXTMESSAGEBROCKER_H

#include <QObject>

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
