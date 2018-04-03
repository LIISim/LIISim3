#ifndef LOGFILEHANDLER_H
#define LOGFILEHANDLER_H

#include <QTextStream>
#include <QFile>
#include "msghandlerbase.h"

/**
 * @brief The LogFileHandler class writes received messages to a logfile...
 * @ingroup Logging
 */
class LogFileHandler : public MsgHandlerBase
{
public:
    LogFileHandler();
    ~LogFileHandler();

protected:

    virtual void handleMessage(LIISimMessageType type,
                               const QString &msg,
                               const QMessageLogContext &context);

private:

    QString m_filename;
    QFile* file;
    QTextStream* stream;
};

#endif // LOGFILEHANDLER_H
