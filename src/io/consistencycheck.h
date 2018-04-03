#ifndef CONSISTENCYCHECK_H
#define CONSISTENCYCHECK_H

#include "core.h"

class ConsistencyCheck
{
public:
    bool check(const SignalIORequest &rq);
    void prepareRun(MRun *run);

private:
    bool irregularityDetected;

    //specific error
    bool signalCountError;
    bool channelCountError;

    //final stats
    int noChannels;
    int noSignals;
};

#endif // CONSISTENCYCHECK_H
