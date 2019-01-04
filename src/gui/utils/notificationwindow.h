#ifndef NOTIFICATIONWINDOW_H
#define NOTIFICATIONWINDOW_H

#include <QWidget>
#include <QTextBrowser>

#include "notificationmessage.h"

/**
 * @brief The NotificationWindow class displays the current error/warning
 * message history.
 */
class NotificationWindow : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief NotificationWindow
     * @param messageHistory A pointer to the list containing the message history
     * @param parent Parent QWidget
     */
    NotificationWindow(QList<LogMessage> messageHistory, QWidget *parent = 0);

    QTextBrowser *textBrowser;

    QList<LogMessage> messageHistory;

    virtual QSize sizeHint() const;

public slots:
    /**
     * @brief update Refills the history window
     */
    void update(LogMessage msg);

};

#endif // NOTIFICATIONWINDOW_H
