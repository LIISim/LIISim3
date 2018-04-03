#include "logmessagewidget.h"

LogMessageWidget::LogMessageWidget(QWidget *parent) : QTextBrowser(parent)
{
    setToolTip("Program Log");

    // show message history
    for(int i = 0;i < msgHistory.size(); i++)
    {
        MsgContainterType mc = msgHistory.at(i);
        QMessageLogContext context;
        handleMessage(mc.type,mc.text,context);
    }
}


LogMessageWidget::~LogMessageWidget()
{
}


void LogMessageWidget::handleMessage(LIISimMessageType type, const QString &msg, const QMessageLogContext &context)
{
    QMutexLocker lock(&msgMutex);
    // no status or detail messages
    if(type == DETAIL_1 ||
       type == STATUS_5000 ||
       type == STATUS_CONST
        #ifndef LIISIM_FULL
            || type == DEBUG
            || type == DEBUG_QT
        #endif
            )
         return;


    QString prefix = "";
    switch (type) {
    case NORMAL:
        setTextColor(Qt::black);
        break;
    case WARNING:
        prefix = "Warning: ";
        setTextColor(QColor("#ffb000"));
        break;
    case DEBUG:

        prefix = "Debug: ";
        setTextColor(Qt::gray);

        break;
    case ERR:
        prefix = "Error: ";
        setTextColor(Qt::red);

        break;
    case ERR_IO:
        prefix = "IO-Error: ";
        setTextColor(Qt::red);

        break;
    case ERR_CALC:
        prefix = "Calculation Error: ";
        setTextColor(Qt::red);

        break;
    case ERR_NULL:
        prefix = "NullPointer Exception: ";
        setTextColor(Qt::red);

        break;
    case INFO:
        prefix = "Info: ";
        setTextColor(Qt::blue);

        break;
    default:
        prefix = msgTypeToString(type)+": ";
        setTextColor(Qt::lightGray);
        break;
    }

    QString mmsg = prefix+msg;
    append(mmsg);
}

