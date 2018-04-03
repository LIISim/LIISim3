#ifndef MPOINT_H
#define MPOINT_H

#include "signalpair.h"
#include <QList>
#include <QMap>
#include <QMutex>


#include "covmatrix.h"

/**
 * @brief MPoint stores for each Channel a SignalPair
 * @ingroup Hierachical-Data-Model
 * @details A Measurement Point stores for each Channel a pair of signals
 * which consists of an absolute and a raw Signal.
 * A measurement point also hols a temperature signal which could be calculated
 * from the MPoint's signal data for each channel.
 */
class MPoint
{
    /** @brief used to lock the access to the MPoint's channel list */
    QMutex mutexChList;

    /** @brief chList contains for each channel a pair aof signals (raw and absolute) */
    QList<SignalPair*> chList;

    int channelCount_raw_abs;

    /** @brief tempSignals Collection of Temperature Signals, maps Channel-ID to Signal.*/
    QMap<int,Signal> tempSignals;

    double trigger_time;

public:

    MPoint(int channelCount_raw_abs);
    ~MPoint();

    /** @brief covariance matrix for every data point (filled by Plugin:MultiSignalAverage */
    QList<CovMatrix> covar_list_raw;
    QList<CovMatrix> covar_list_abs;

    // get signal by channel id and signal type
    Signal getSignal(int chID, Signal::SType stype);
    void setSignal(const Signal & s,int chID, Signal::SType stype);

    int addTemperatureChannel(int tchid = -1);
    void removeTemperatureChannel(int ch_id);

    double getMinSignalTime( int chID = -1 );
    double getMinSignalTime( Signal::SType stype );
    double getMaxSignalTime( int chID = -1 );
    double getMaxSignalTime( Signal::SType stype );

    int channelCount(Signal::SType stype);
    bool isValidChannelID(int id, Signal::SType stype, bool throwExcep = true);
    bool isValidFitTChannelID(int chID);

    QList<int> channelIDs(Signal::SType stype);

    void setTriggerTime(double triggerTime);
    double getTriggerTime();

    bool rawValid;
    bool absValid;
    bool tempValid;
};

#endif // MPOINT_H
