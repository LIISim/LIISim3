#include "statusmessagewidget.h"


StatusMessageWidget::StatusMessageWidget(QWidget *parent) : QStatusBar(parent)
{
}

StatusMessageWidget::~StatusMessageWidget()
{
}


void StatusMessageWidget::handleMessage(LIISimMessageType type, const QString &msg, const QMessageLogContext &context)
{
    QMutexLocker lock(&msgMutex);

    // only status messages
    if( type == STATUS_5000 )
        showMessage(msg,5000);
    else if(type == STATUS_CONST)
        showMessage(msg);
}
