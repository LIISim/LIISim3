#ifndef LIISIMMESSAGETYPE_H
#define LIISIMMESSAGETYPE_H


/**
 * @brief defines the type of a LIISim text message
 */
enum LIISimMessageType{

    NORMAL,         // normal message
    WARNING,        // warning message
    WARNING_QT,     // qt warn message (qWarning())
    DEBUG,          // debug message
    DEBUG_QT,       // qt debug message
    ERR,            // general error message
    ERR_IO,         // io error message
    ERR_CALC,       // calculation error message
    ERR_NULL,       // null pointer exception
    INFO,           // info message
    CRITICAL_QT,    // qt critical message
    FATAL_QT,       // qt fatal message
    DETAIL_1,       // detailed message (level 1)
    STATUS_CONST,   // constant status message (for statusbar etc)
    STATUS_5000     // statusmessage, dissapeares after 5000 ms
};

#endif // METATYPES_H
