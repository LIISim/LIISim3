#include "notificationmanager.h"

#include "core.h"

NotificationManager::NotificationManager() : MsgHandlerBase()
{
    notificationWindow = nullptr;

    warning = 0;
    error = 0;
}


NotificationManager::~NotificationManager()
{
    if(notificationWindow)
    {
        notificationWindow->close();
        delete notificationWindow;
    }
}


void NotificationManager::handleMessage(LIISimMessageType type, const QString &msg, const QMessageLogContext &context)
{
    if(type == WARNING)
    {
        warning++;
        messageHistory.push_back(Message(type, msg));
    }
    else if(type == ERR || type == ERR_IO || type == ERR_CALC || type == ERR_NULL)
    {
        error++;
        messageHistory.push_back(Message(type, msg));
    }
    // verbose mode
    //else
    //    messageHistory.push_back(Message(type, msg));

    emit countChanged(warning, error);

    emit update();
}


QWidget* NotificationManager::getNotificationCenter(QWidget *parent)
{
    NotificationCenter *nc = new NotificationCenter(parent);
    connect(this, SIGNAL(countChanged(int,int)), nc, SLOT(setCount(int,int)));
    connect(this, SIGNAL(reset()), nc, SLOT(resetCount()));
    return nc;
}


void NotificationManager::registerListener(NotificationCenter *listener)
{
    if(!listeners.contains(listener))
        listeners.push_back(listener);
}


void NotificationManager::unregisterListener(NotificationCenter *listener)
{
    listeners.removeAll(listener);
}


void NotificationManager::showNotifications()
{
    warning = 0;
    error = 0;

    emit reset();

    if(notificationWindow == nullptr)
    {
        notificationWindow = new NotificationWindow(&messageHistory);
        connect(this, SIGNAL(update()), notificationWindow, SLOT(update()), Qt::QueuedConnection);
    }

    if(!notificationWindow->isVisible())
        notificationWindow->show();

    if(!notificationWindow->isActiveWindow())
        notificationWindow->activateWindow();

    if(notificationWindow->isMinimized())
        notificationWindow->showNormal();

    notificationWindow->update();
}
