#include "notificationmessage.h"

Message::Message(const LIISimMessageType type, const QString &msg)
    : type(type), msg(msg), timestamp(QDateTime::currentDateTime())
{

}
