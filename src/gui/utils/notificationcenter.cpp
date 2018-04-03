#include "notificationcenter.h"

#include "core.h"

NotificationCenter::NotificationCenter(QWidget *parent) : RibbonToolBox(" NOTIFICATIONS ", parent)
{
    info = new QToolButton(this);
    info->setText("0 Warning(s) \n0 Error(s)");
    //info->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    info->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    info->setStyleSheet("QToolButton { color : gray }");
    info->setToolTip("Show Notifications");
    setMinimumWidth(200);

    addWidget(info, 0, 0);

    Core::instance()->getNotificationManager()->registerListener(this);

    connect(info, SIGNAL(clicked(bool)), SLOT(onInfoClicked()));
}


NotificationCenter::~NotificationCenter()
{
    Core::instance()->getNotificationManager()->unregisterListener(this);
}


void NotificationCenter::setCount(int warnings, int errors)
{
    info->setText(QString("%0 Warning(s)\n%1 Error(s)").arg(warnings).arg(errors));
    if(warnings > 0 && errors == 0)
    {
        info->setIcon(QIcon(Core::rootDir + "resources/icons/status_warning.png"));
        info->setStyleSheet("QToolButton { color : chocolate }");
    }
    else if(errors > 0)
    {
        info->setIcon(QIcon(Core::rootDir + "resources/icons/status_cancel.png"));
        info->setStyleSheet("QToolButton { color : red }");
    }
}


void NotificationCenter::resetCount()
{
    info->setText("0 Warning(s) \n0 Error(s)");
    info->setIcon(QIcon());
    info->setStyleSheet("QToolButton { color : gray }");

    // reset all messages if window is opened
    MSG_ONCE_RESET_ALL();
}


void NotificationCenter::onInfoClicked()
{
    Core::instance()->getNotificationManager()->showNotifications();
}
