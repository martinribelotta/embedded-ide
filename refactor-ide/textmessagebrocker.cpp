#include "textmessagebrocker.h"

TextMessageBrocker::TextMessageBrocker(QObject *parent) : QObject(parent)
{

}

TextMessageBrocker &TextMessageBrocker::instance()
{
    static TextMessageBrocker *ptr = nullptr;
    if (!ptr)
        ptr = new TextMessageBrocker();
    return *ptr;
}

void TextMessageBrocker::publish(const QString &topic, const QString &message)
{
    emit published(topic, message);
}
