#ifndef LOGMESSAGEWIDGET_H
#define LOGMESSAGEWIDGET_H

#include <QTextBrowser>

#include "msghandlerbase.h"


/**
 * @brief The LogMessageWidget class is QTextBrowser-Widget and
 * visualizes Log-Messages ... (used in "Home" screen)
 * @ingroup Logging
 */
class LogMessageWidget : public QTextBrowser, public MsgHandlerBase
{
    Q_OBJECT
public:
    explicit LogMessageWidget(QWidget *parent = 0);
    ~LogMessageWidget();

protected:

    virtual void handleMessage(LIISimMessageType type,
                               const QString &msg,
                               const QMessageLogContext &context);
};

#endif // LOGMESSAGEWIDGET_H
