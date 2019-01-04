#include "notificationmessage.h"

LogMessage::LogMessage() : type(LIISimMessageType::NORMAL), msg(""), timestamp(QDateTime::currentDateTime())
{

}

LogMessage::LogMessage(const LIISimMessageType type, const QString &msg)
    : type(type), msg(msg), timestamp(QDateTime::currentDateTime())
{

}
