#include "consoleouthandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

ConsoleOutHandler::ConsoleOutHandler()
{
}

ConsoleOutHandler::~ConsoleOutHandler()
{
}

void ConsoleOutHandler::handleMessage(LIISimMessageType type, const QString &msg, const QMessageLogContext &context)
{
    QMutexLocker lock(&msgMutex);

    // do not print messages with detail level 1
    if(type == DETAIL_1)
        return;

    // no status messages
    if(type == STATUS_5000 || type == STATUS_CONST)
        return;

    if(type == NORMAL)
    {
        std::cerr << msg.toStdString() << std::endl;
    }
    else
    {
        std::cerr << msgTypeToString(type).toStdString() << ": "
                  << msg.toStdString() << std::endl;
    }
}
