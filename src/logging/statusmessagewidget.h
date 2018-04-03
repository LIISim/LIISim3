#ifndef STATUSMESSAGEWIDGET_H
#define STATUSMESSAGEWIDGET_H

#include <QStatusBar>
#include "msghandlerbase.h"

/**
 * @brief The StatusMessageWidget class is a QStatusBar-Widget
 * and visualizes status messages ...
 * @ingroup Logging
 */
class StatusMessageWidget : public QStatusBar, public MsgHandlerBase
{
    Q_OBJECT
public:
    explicit StatusMessageWidget(QWidget *parent = 0);
    ~StatusMessageWidget();

protected:

    virtual void handleMessage(LIISimMessageType type,
                               const QString &msg,
                               const QMessageLogContext &context);

};

#endif // STATUSMESSAGEWIDGET_H
