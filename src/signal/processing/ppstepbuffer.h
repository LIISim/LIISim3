#ifndef PPSTEPBUFFER_H
#define PPSTEPBUFFER_H

#include "boost/multi_array.hpp"
#include "../signal.h"


/**
 * @brief The PPStepBuffer class saves intermediate steps into boost::multi_array
 */
class PPStepBuffer
{
public:
    PPStepBuffer();

    // Two dimensions: signals, channels
    boost::multi_array<Signal,2> data;

    ~PPStepBuffer();

    unsigned long numberOfSignals();
    unsigned long numberOfDataPoints();
};

#endif // PPSTEPBUFFER_H
