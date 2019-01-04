#include "notificationwindow.h"

#include <QVBoxLayout>

NotificationWindow::NotificationWindow(QList<LogMessage> messageHistory, QWidget *parent) : QWidget(parent), messageHistory(messageHistory)
{
    setWindowTitle("Notifications");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    textBrowser = new QTextBrowser(this);
    mainLayout->addWidget(textBrowser);

    if(messageHistory.isEmpty())
        textBrowser->append("<i>No notifications</i>");
    else
    {
        for(LogMessage msg : messageHistory)
        {
            QString finalMsg = msg.timestamp.toString("hh:mm:ss");
            finalMsg.append(" - ");

            switch(msg.type)
            {
            case WARNING:
                finalMsg.append("Warning: ");
                textBrowser->setTextColor(QColor("#ffb000"));
                break;
            case ERR:
                finalMsg.append("Error: ");
                textBrowser->setTextColor(Qt::red);
                break;
            case ERR_CALC:
                finalMsg.append("Calculation Error: ");
                textBrowser->setTextColor(Qt::red);
                break;
            case ERR_IO:
                finalMsg.append("IO Error: ");
                textBrowser->setTextColor(Qt::red);
                break;
            default:
                textBrowser->setTextColor(Qt::gray);
                break;
            }

            finalMsg.append(msg.msg);
            textBrowser->append(finalMsg);
        }
    }
}


QSize NotificationWindow::sizeHint() const
{
    return QSize(800, 500);
}


void NotificationWindow::update(LogMessage msg)
{
    if(messageHistory.isEmpty())
        textBrowser->clear();

    messageHistory.append(msg);

    QString finalMsg = msg.timestamp.toString("hh:mm:ss");
    finalMsg.append(" - ");

    switch(msg.type)
    {
    case WARNING:
        finalMsg.append("Warning: ");
        textBrowser->setTextColor(QColor("#ffb000"));
        break;
    case ERR:
        finalMsg.append("Error: ");
        textBrowser->setTextColor(Qt::red);
        break;
    case ERR_CALC:
        finalMsg.append("Calculation Error: ");
        textBrowser->setTextColor(Qt::red);
        break;
    case ERR_IO:
        finalMsg.append("IO Error: ");
        textBrowser->setTextColor(Qt::red);
        break;
    default:
        textBrowser->setTextColor(Qt::gray);
        break;
    }

    finalMsg.append(msg.msg);
    textBrowser->append(finalMsg);


    /*if(messageHistory)
    {
        textBrowser->clear();

        if(messageHistory->size() == 0)
        {
            textBrowser->append("<i>No notifications</i>");
            return;
        }

        for(Message msg : messageHistory)
        {
            QString finalMsg = msg.timestamp.toString("hh:mm:ss");
            finalMsg.append(" - ");

            switch(msg.type)
            {
            case WARNING:
                finalMsg.append("Warning: ");
                textBrowser->setTextColor(QColor("#ffb000"));
                break;
            case ERR:
                finalMsg.append("Error: ");
                textBrowser->setTextColor(Qt::red);
                break;
            case ERR_CALC:
                finalMsg.append("Calculation Error: ");
                textBrowser->setTextColor(Qt::red);
                break;
            case ERR_IO:
                finalMsg.append("IO Error: ");
                textBrowser->setTextColor(Qt::red);
                break;
            default:
                textBrowser->setTextColor(Qt::gray);
                break;
            }

            finalMsg.append(msg.msg);
            textBrowser->append(finalMsg);
        }
    }*/
}
