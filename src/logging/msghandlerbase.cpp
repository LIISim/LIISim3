#include "msghandlerbase.h"


#include <qapplication.h>
#include <iostream>

#include <QMutexLocker>
#include <QMessageBox>

#include "../core.h"

// initialize static members
QList<MsgHandlerBase*> MsgHandlerBase::registredHandlers = QList<MsgHandlerBase*>();
QQueue<MsgContainterType> MsgHandlerBase::msgHistory = QQueue<MsgContainterType>();
QMultiMap<QString,int> MsgHandlerBase::msgOnceIDs = QMultiMap<QString,int>();

/**
 * @brief MsgHandlerBase::MsgHandlerBase Constructor
 */
MsgHandlerBase::MsgHandlerBase()
{
    registredHandlers.append(this);
}

/**
 * @brief MsgHandlerBase::~MsgHandlerBase Destructor
 */
MsgHandlerBase::~MsgHandlerBase()
{
    registredHandlers.removeOne(this);
}

/**
 * @brief MsgHandlerBase::handleQtMessage static method for posting a message.
 * This method can be used for installing a global message handler (see qInstallMessageHandler)
 * @param type message type
 * @param context log context
 * @param msg message text
 */
void MsgHandlerBase::handleQtMessage(QtMsgType type,
                                     const QMessageLogContext &context,
                                     const QString &msg)
{

    // transform QtMsgType to LIISimMessageType
    LIISimMessageType mtype = LIISimMessageType::NORMAL;
    switch (type) {
        case QtDebugMsg:
            mtype = LIISimMessageType::DEBUG_QT;
            break;
        case QtWarningMsg:
            mtype = LIISimMessageType::WARNING_QT;
            break;
        case QtCriticalMsg:
            mtype = LIISimMessageType::CRITICAL_QT;
            break;
        case QtFatalMsg:
            mtype = LIISimMessageType::FATAL_QT;
            break;
    }

    if(mtype == WARNING_QT)
    {
        std::cout << msgTypeToString(mtype).toStdString()
                  << ": " <<msg.toStdString() << std::endl;
    }
    else
    {
        // send message to all registered handlers
        for(int i = 0; i < registredHandlers.size(); i++)
        {
            registredHandlers[i]->handleMessage(mtype,msg,context);
        }
    }

    // store message to history
    MsgContainterType mc;
    mc.type = mtype;
    mc.text = msg;
    msgHistory.enqueue(mc);
    if(msgHistory.size() > 50)
        msgHistory.dequeue();
}

/**
 * @brief MsgHandlerBase::msg static method for posting a message.
 * This method calls the handleMethod slot on all registered subclass instances
 * @param msg message text
 * @param type message type
 */
void MsgHandlerBase::msg(const QString &msg,LIISimMessageType type)
{
    // send message to all registered handlers
    for(int i = 0; i < registredHandlers.size(); i++)
    {
        registredHandlers[i]->handleMessage(type,msg,QMessageLogContext());
    }

    // add message to history
    MsgContainterType mc;
    mc.type = type;
    mc.text = msg;
    msgHistory.enqueue(mc);
    if(msgHistory.size() > 50)
        msgHistory.dequeue();
}


/**
 * @brief MsgHandlerBase::msg_once
 * is used in definition of shortcuts MSG_ONCE
 *  Usage:
 *      MSG_ONCE("ExampleGroup", 1, "An error occured", ERR);
 * @param classname
 * @param id
 * @param msg
 * @param type
 */
void MsgHandlerBase::msg_once(const QString &group, const int id, const QString &msg, LIISimMessageType type)
{
    if(!msgOnceIDs.contains(group, id))
    {
        msgOnceIDs.insert(group, id);
        MsgHandlerBase::msg(msg, type);
    }
}


void MsgHandlerBase::msg_once_window(const QString &group, const int id, const QString &title, const QString &msg, LIISimMessageType type)
{
    // show message in dialog window
    if(!msgOnceIDs.contains(group, id))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QString("LIISim: %0").arg(title));
        msgBox.setText(msg);

        switch(type)
        {
            case WARNING:
            case WARNING_QT:
                msgBox.setIconPixmap(QIcon(Core::rootDir + "resources/icons/status_warning.png").pixmap(32));
                break;

            case ERR:
            case ERR_IO:
            case ERR_CALC:
            case ERR_NULL:
            case CRITICAL_QT:
            case FATAL_QT:
                msgBox.setIconPixmap(QIcon(Core::rootDir + "resources/icons/status_cancel.png").pixmap(32));
                break;

            default:
                msgBox.setIconPixmap(QIcon(Core::rootDir + "resources/icons/status_info.png").pixmap(32));
                break;
        }

        msgBox.exec();
    }

    // normal msg_once: shows and inserts message in list
    msg_once(group, id, msg, type);
}


void MsgHandlerBase::msg_once_reset(const QString &group, const int id)
{
    if(msgOnceIDs.contains(group, id))
        msgOnceIDs.remove(group, id);
}


void MsgHandlerBase::msg_once_reset_group(const QString &group)
{
    if(msgOnceIDs.contains(group))
        msgOnceIDs.remove(group);
}


void MsgHandlerBase::msg_once_reset_all()
{
    msgOnceIDs.clear();
}


/**
 * @brief MsgHandlerBase::msgTypeToString get a QString representation of a message type
 * @param type message type
 * @return QString representation of a message type
 */
QString MsgHandlerBase::msgTypeToString(LIISimMessageType type)
{
    QString res = "Default";

    if(type == NORMAL)
        res = "Normal";
    else if(type == WARNING)
        res = "Warning";
    else if(type == WARNING_QT)
        res = "qWarn";
    else if(type == DEBUG)
        res = "Debug";
    else if(type == DEBUG_QT)
        res = "qDebug";
    else if(type == ERR)
        res = "Error";
    else if(type == ERR_IO)
        res = "IO-Error";
    else if(type == ERR_CALC)
        res = "Calc-Error";
    else if(type == ERR_NULL)
        res = "Null-Error";
    else if(type == INFO)
        res = "Info";
    else if(type == CRITICAL_QT)
        res = "qCritical";
    else if(type == FATAL_QT)
        res = "qFatal";
    else if(type == DETAIL_1)
        res = "Detail1";
    else if(type == STATUS_5000 || type == STATUS_CONST)
        res = "Status";

    return res;
}
