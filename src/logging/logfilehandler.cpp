#include "logfilehandler.h"

#include "../core.h";

LogFileHandler::LogFileHandler()
{
    m_filename = Core::rootDir + "logfile.txt";

    file = new QFile(m_filename);
    stream = 0;

    if (file->open(QFile::WriteOnly | QFile::Truncate))
    {
        stream = new QTextStream(file);

        *stream << "LOGFILE \r\r\n\n";
        *stream << "Creation-Date: " << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz") << "\r\n";

        QString str;
        str = QString("LIISim-Version: %0 \r\n").arg(Core::LIISIM_VERSION);
        *stream << str;


        // WRITE BUILD INFORMAITONS

        str.sprintf("Qt-Version: %s \r\n", QT_VERSION_STR);
        *stream << str;

        #ifdef _MSC_VER
                *stream << "MSVC Version: "
                        << _MSC_VER
                        << " (" << _MSC_FULL_VER << ") "
                        << "\r\n";
        #endif

        #ifdef __GNUC__
                *stream << "GNU GCC/G++ Version: "
                        << __GNUC__
                        << "."
                        << __GNUC_MINOR__
                        << "\r\n";
        #endif

        #ifdef QT_DEBUG
            *stream << "Mode: Debug\r\n";
        #else
            *stream << "Mode: Release\r\n";
        #endif


        *stream << "\r\nMESSAGES:\r\r\n\n";

        stream->flush();
    }
}


LogFileHandler::~LogFileHandler()
{
    if(stream) *stream << "Goodbye.\r\n";

    file->close();
    delete stream;
    delete file;
}


void LogFileHandler::handleMessage(LIISimMessageType type, const QString &msg, const QMessageLogContext &context)
{
    QMutexLocker lock(&msgMutex);
    if(!stream) return;

    // no status messages
    if(type == STATUS_5000 || type == STATUS_CONST)
        return;

    *stream << msgTypeToString(type) ;
    if(type == DEBUG_QT || type == WARNING_QT || type == CRITICAL_QT || type == FATAL_QT)
    {
        *stream << "["
            << context.file << ": "
            << context.line << ", "
            << context.function << "] ";
    }

    *stream << ": " << msg << "\r\n";

    stream->flush();
}
