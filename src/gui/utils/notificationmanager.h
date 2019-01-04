#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QList>

#include "../../logging/msghandlerbase.h"

#include "notificationmessage.h"
#include "notificationcenter.h"
#include "notificationwindow.h"


/**
 * @brief The NotificationManager class
 * Catches global messages and makes them available through the @class NotificationCenter
 * and @class NotificationWindow. For further information see @class MsgHandlerBase.
 */
class NotificationManager : public QObject, public MsgHandlerBase
{
    Q_OBJECT
public:
    NotificationManager();
    ~NotificationManager();

    /**
     * @brief handleMessage Handles all incoming messages. See @class MsgHandlerBase.
     */
    void handleMessage(LIISimMessageType type, const QString &msg, const QMessageLogContext &context);

    /**
     * @brief getNotificationCenter Builds a Notification Center and returns its pointer
     * @param parent The parent QWidget, can be left empty
     * @return
     */
    QWidget* getNotificationCenter(QWidget *parent = 0);

    void registerListener(NotificationCenter *listener);
    void unregisterListener(NotificationCenter *listener);

    /**
     * @brief showNotifications Called by the NotificationCenter if the button is pressed.
     * Resets the warning/error counter and opens the NotificationWindow
     */
    void showNotifications();

private:
    int warning;
    int error;

    QList<NotificationCenter*> listeners;

    QList<LogMessage> messageHistory;

    NotificationWindow *notificationWindow;

signals:
    /**
     * @brief update Received by the NotificationCenter to update the text.
     */
    void update(LogMessage msg);
    /**
     * @brief countChanged Emitted if the error/warning count changes.
     * @param warning Warning message count since last reset.
     * @param error Error message count since last reset.
     */
    void countChanged(int warning, int error);
    /**
     * @brief reset Emited if the GUI error/warning counter should be reseted.
     */
    void reset();

};

#endif // NOTIFICATIONMANAGER_H
