#ifndef SIGNALPAIR_H
#define SIGNALPAIR_H

#include "signal.h"

/**
 * @brief stores a pair of signals (raw signal, absolute signal)
 * @ingroup Hierachical-Data-Model
 * @details this class is used by MPoint for internal data storage
 */
class SignalPair
{

public:
    SignalPair();
    ~SignalPair();

    /** @brief Signal of Signal::SType RAW */
    Signal raw;

    /** @brief Signal of Signal::SType ABS */
    Signal absolute;

};

#endif // SIGNALPAIR_H
