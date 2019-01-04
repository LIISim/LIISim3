#ifndef NOTIFICATIONMESSAGE_H
#define NOTIFICATIONMESSAGE_H

#include <QDateTime>

#include "../../general/LIISimMessageType.h"

/**
 * @brief The Message class is used for the history in the NotificationWindow.
 */
class LogMessage
{
public:
    LogMessage();
    LogMessage(const LIISimMessageType type, const QString &msg);

    const LIISimMessageType type;
    const QString msg;
    const QDateTime timestamp;
};

#endif // NOTIFICATIONMESSAGE_H
