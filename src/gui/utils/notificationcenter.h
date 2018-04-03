#ifndef NOTIFICATIONCENTER_H
#define NOTIFICATIONCENTER_H

#include <QToolButton>

#include "ribbontoolbox.h"

class NotificationCenter : public RibbonToolBox
{
    friend class NotificationManager;

    Q_OBJECT
public:
    NotificationCenter(QWidget *parent = 0);
    ~NotificationCenter();

private:
    QToolButton *info;

private slots:
    void onInfoClicked();

public slots:
    void setCount(int warnings, int errors);
    void resetCount();

};

#endif // NOTIFICATIONCENTER_H
