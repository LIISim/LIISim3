#ifndef MSGHANDLERBASE_H
#define MSGHANDLERBASE_H

#include <qapplication.h>
#include "../general/LIISimMessageType.h"

#include <QList>
#include <QQueue>
#include <QMutex>

/** @defgroup Logging
  * @brief Global message system and Classes/Widgets for Log-Visualization
  */

/**
 * @brief The MsgContainterType class stores text and type of a message.
 * This Class is only used to store the message history (last 50 messages)
 */
class MsgContainterType
{
public:
    MsgContainterType(){}
    LIISimMessageType type;
    QString text;
};

/**
 * @brief The MsgHandlerBase class serves as an abstract
 * base class for all types of message visualizations (conosle-out, logfile, etc.).
 * It also provides static methods for global message handling.
 *
 * It's also possible to take control over the qDebug/qWarn/qCritical/qFatal messages:
 * Therefore install the global message handler: qInstallMessageHandler(&MsgHandlerBase::handleQtMessage);
 *
 */

// Shortcuts for static method calls
#define MESSAGE(X,T) MsgHandlerBase::msg(X,T)
#define MSG_NORMAL(X) MsgHandlerBase::msg(X)
#define MSG_DEBUG(X) MsgHandlerBase::msg(X,LIISimMessageType::DEBUG)
#define MSG_WARN(X) MsgHandlerBase::msg(X,LIISimMessageType::WARNING)
#define MSG_ERR(X) MsgHandlerBase::msg(X,LIISimMessageType::ERR)
#define MSG_INFO(X) MsgHandlerBase::msg(X,LIISimMessageType::INFO)
#define MSG_DETAIL_1(X) MsgHandlerBase::msg(X,LIISimMessageType::DETAIL_1)
#define MSG_STATUS_CONST(X) MsgHandlerBase::msg(X,LIISimMessageType::STATUS_CONST)
#define MSG_STATUS(X) MsgHandlerBase::msg(X,LIISimMessageType::STATUS_5000)

#define MSG_ASYNC(X,T) emit Core::instance()->asyncMsg(X,T);

//MSG_ONCE: takes a id and a message and forwards the message only if it hasn't been before
//W: group, X: id, Y: msg string, Z: LIISimMessageType
// Usage: MSG_ONCE("SpectroscopicMaterial", 1, "An error occured", ERR);
// Please see: logging/msg_once_list.txt for all used groups
#define MSG_ONCE(W, X, Y, Z) MsgHandlerBase::msg_once(W, X, Y, Z)

// Same as MSG_ONCE but additionally opens dialog window
//W: group, X: id, Y1: window title, Y2, msg string, Z: LIISimMessageType
#define MSG_ONCE_WINDOW(W, X, Y1, Y2, Z) MsgHandlerBase::msg_once_window(W, X, Y1, Y2, Z)

//MSG_ONCE_RESET: resets a id so it can be shown again
//W: group, X: id
#define MSG_ONCE_RESET(W, X) MsgHandlerBase::msg_once_reset(W, X)

//MSG_ONCE_RESET_GROUP: resets all ids of the class
//W: group
// Usage: MSG_ONCE_GROUP("SpectroscopicMaterial");
#define MSG_ONCE_RESET_GROUP(W) MsgHandlerBase::msg_once_reset_group(W)

//MSG_ONCE_RESET_GROUP: resets all ids (called if NotificationWindow is opened)
#define MSG_ONCE_RESET_ALL() MsgHandlerBase::msg_once_reset_all()

class MsgHandlerBase
{
public:
    MsgHandlerBase();
    virtual ~MsgHandlerBase();

    static void handleQtMessage(QtMsgType type,
                                const QMessageLogContext &context,
                                const QString &msg);

    static void msg(const QString& msg, LIISimMessageType type = LIISimMessageType::NORMAL);

    static void msg_once(const QString &group, const int id, const QString &msg, LIISimMessageType type = LIISimMessageType::NORMAL);
    static void msg_once_window(const QString &group, const int id, const QString &title, const QString &msg, LIISimMessageType type = LIISimMessageType::NORMAL);

    static void msg_once_reset(const QString &group, const int id);
    static void msg_once_reset_group(const QString &group);
    static void msg_once_reset_all();

    static QString msgTypeToString(LIISimMessageType type);

protected:

    /**
     * @brief handleMessage abstract method for message handling.
     * Subclasses have to implement this method.
     * This method is called on all MsgHandlerBase instances when
     * a message has been posted
     * @param type message type
     * @param msg text
     * @param context log context (for qt-messages)
     */
    virtual void handleMessage(LIISimMessageType type,
                               const QString &msg,
                               const QMessageLogContext &context) = 0;

    /**
     * @brief msgHistory holds last X messages
     */
    static QQueue<MsgContainterType> msgHistory;

    static QMultiMap<QString,int> msgOnceIDs;

    QMutex msgMutex;


private:

    /**
     * @brief registredHandlers global list of registered subclass instatnces
     */
    static QList<MsgHandlerBase*> registredHandlers;

};

#endif // MSGHANDLERBASE_H
