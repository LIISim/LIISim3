#ifndef NOTIFICATIONWINDOW_H
#define NOTIFICATIONWINDOW_H

#include <QWidget>
#include <QTextBrowser>

class Message;

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
    NotificationWindow(QList<Message> *messageHistory, QWidget *parent = 0);

    QTextBrowser *textBrowser;

    QList<Message> *messageHistory;

    virtual QSize sizeHint() const;

public slots:
    /**
     * @brief update Refills the history window
     */
    void update();

};

#endif // NOTIFICATIONWINDOW_H
