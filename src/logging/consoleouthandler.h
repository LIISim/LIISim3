#ifndef CONSOLEOUTHANDLER_H
#define CONSOLEOUTHANDLER_H

#include "msghandlerbase.h"

/**
 * @brief The ConsoleOutHandler class prints received messages
 * to the standard output.
 * @ingroup Logging
 */
class ConsoleOutHandler : public MsgHandlerBase
{
public:
    ConsoleOutHandler();
    ~ConsoleOutHandler();

protected:

    virtual void handleMessage(LIISimMessageType type,
                               const QString &msg,
                               const QMessageLogContext &context);

};

#endif // CONSOLEOUTHANDLER_H
