#ifndef LIISIMEXCEPTION_H
#define LIISIMEXCEPTION_H

#include <QString>
#include "LIISimMessageType.h"


/**
 * @brief custom exception (should be thrown and catched for each possible error)
 */
class LIISimException : public std::exception
{
    QString msg;
    LIISimMessageType msg_type;
public:

    LIISimException ( const QString &err) : msg(err) {
        msg_type = ERR;
    }

    LIISimException ( const QString &err, LIISimMessageType mtype) : msg(err) {
        msg_type = mtype;
    }

    ~LIISimException() throw() {}
    const QString& what() { return msg; }
    const LIISimMessageType & type(){return msg_type;}
};



#endif // LIISIMEXCEPTION_H
