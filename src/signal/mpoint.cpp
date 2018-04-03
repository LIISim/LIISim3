#include "mpoint.h"
#include <QDebug>
#include <QMutexLocker>
#include "../general/LIISimException.h"
#include <limits>


/**
 * @brief MPoint::MPoint
 * @param channelCount_raw_abs number of raw/absolute signal channels.
 * The number of temperature signals is initialized with 0
 * @details creates a Measurement point object with given number of channels
 */
MPoint::MPoint(int channelCount_raw_abs)
{
    QMutexLocker lock(&mutexChList);

    if(channelCount_raw_abs < 0)
        throw LIISimException("MPoint: negative number of channels requested!");


    // allocate memory for signals pairs for each channel
    for(int i = 0; i< channelCount_raw_abs; i++)
    {
        SignalPair* sp = new SignalPair;
        sp->absolute.channelID = i+1;
        sp->raw.channelID = i+1;
        chList.push_back(sp);
    }

    this->channelCount_raw_abs = channelCount_raw_abs;

    trigger_time = 0;

    rawValid = false;
    absValid = false;
    tempValid = false;

    // set type member of temperature signal
 /*   Signal ts;
    ts.type = Signal::TEMPERATURE;
    ts.channelID = 1;
    tempSignals.insert(1,ts);*/
}


/**
 * @brief MPoint::~MPoint Destructor
 */
MPoint::~MPoint()
{

    QMutexLocker lock(&mutexChList);


    // for each channel delete the signal pair
    while(!chList.isEmpty())
    {
        SignalPair* sp = chList.first();
        delete sp;
        chList.pop_front();
    }
}


/**
 * @brief MPoint::getSignal get signal from Measurement point by channel ID and signal type
 * @param chID ID of channel
 * @param stype type of signal (raw, absolute)
 * @return Signal with given type and channel ID
 * @details if the channel ID is invalid or the requested signal is
 * not absolute or raw, an exception is thrown!
 */
Signal MPoint::getSignal(int chID, Signal::SType stype)
{

    QMutexLocker lock(&mutexChList);
    if(!isValidChannelID(chID,stype))
    {
        QString msg;
        msg.sprintf("MPoint: getSignal invalid channel ID: %d",chID);
        msg.append(" for signal type "+Signal::stypeToString(stype));
        throw LIISimException(msg);
    }

    if(stype == Signal::RAW)
    {
        return chList.at(chID-1)->raw;
    }
    else if(stype == Signal::ABS)
    {
       return chList.at(chID-1)->absolute;
    }
    else if(stype == Signal::TEMPERATURE)
    {
       return tempSignals.value(chID);
    }

    throw LIISimException("MPoint: getSignal requested invalid signal type!");
}


/**
 * @brief MPoint::getMinSignalTime Helper method wich allows to
 * determine the minimum start time of all signals
 * of this MPoint or the minimum start time for a certain channel
 * @param chID channel id, default value -1: search for all channels
 * @return minimum start time in seconds
 */
double MPoint::getMinSignalTime(int chID)
{
    QMutexLocker lock(&mutexChList);
    double mint = std::numeric_limits<double>::max();

    // default argument -1: search for all channels
    double t = 0.0;
    if(chID == -1)
    {
        for(int i = 0; i < chList.size(); i++)
        {
            t = chList.at(i)->raw.start_time;
            if( t < mint)
                mint = t;
            t = chList.at(i)->absolute.start_time;
            if( t < mint)
                mint = t;
        }
    }
    // check for valid channel id
    else if(isValidChannelID(chID, Signal::RAW))
    {
        t = chList.at(chID)->raw.start_time;
        if( t < mint)
            mint = t;
        t = chList.at(chID)->absolute.start_time;
        if( t < mint)
            mint = t;
    }

    return mint;
}


/**
 * @brief MPoint::getMinSignalTime Helper method wich allows to
 * determine the minimum start time of all signals
 * of this MPoint with given signal type
 * @param stype signal type
 * @return minimum start time of signals of given type
 */
double MPoint::getMinSignalTime(Signal::SType stype)
{
    QMutexLocker lock(&mutexChList);

    double mint = std::numeric_limits<double>::max();
    double t = 0.0;


    if(stype == Signal::TEMPERATURE)
    {
        QList<Signal> values = tempSignals.values();
        for(int i = 0; i < values.size(); i++)
        {
            t = values[i].maxTime();
            if( t < mint)
                mint = t;
        }
    }

    for(int i = 0; i < chList.size(); i++)
    {
        if(stype == Signal::RAW)
        {
            t = chList.at(i)->raw.start_time;
        }
        else if(stype == Signal::ABS)
        {
           t = chList.at(i)->absolute.start_time;
        }
        if( t < mint)
            mint = t;
    }
    return mint;
}



/**
 * @brief MPoint::getMinSignalTime Helper method wich allows to
 * determine the minimum start time of all signals
 * of this MPoint or the minimum start time for a certain channel
 * @param chID channel id, default value -1: search for all channels
 * @return minimum start time in seconds
 */
double MPoint::getMaxSignalTime(int chID)
{
    QMutexLocker lock(&mutexChList);
    double maxt = std::numeric_limits<double>::min();

    // default argument -1: search for all channels
    double t = 0.0;
    if(chID == -1)
    {
        for(int i = 0; i < chList.size(); i++)
        {
            t = chList.at(i)->raw.maxTime();
            if( t > maxt)
                maxt = t;
            t = chList.at(i)->absolute.maxTime();
            if( t > maxt)
                maxt = t;
        }
    }

    // check for valid channel id
    else if(isValidChannelID(chID,Signal::RAW))
    {
        t = chList.at(chID)->raw.maxTime();
        if( t > maxt)
            maxt = t;
        t = chList.at(chID)->absolute.maxTime();
        if( t > maxt)
            maxt = t;
    }

    return maxt;
}


/**
 * @brief MPoint::getMinSignalTime Helper method wich allows to
 * determine the minimum start time of all signals
 * of this MPoint with given signal type
 * @param stype signal type
 * @return minimum start time of signals of given type
 */
double MPoint::getMaxSignalTime(Signal::SType stype)
{
    QMutexLocker lock(&mutexChList);

    double maxt = std::numeric_limits<double>::min();
    double t = 0.0;

    if(stype == Signal::TEMPERATURE)
    {
        QList<Signal> values = tempSignals.values();
        for(int i = 0; i < values.size(); i++)
        {
            t = values[i].maxTime();
            if( t > maxt)
                maxt = t;
        }
    }

    for(int i = 0; i < chList.size(); i++)
    {
        if(stype == Signal::RAW)
        {
            t = chList.at(i)->raw.maxTime();
        }
        else if(stype == Signal::ABS)
        {
           t = chList.at(i)->absolute.maxTime();
        }
        if( t > maxt)
            maxt = t;
    }
    return maxt;
}


/**
 * @brief MPoint::setSignal set signal of Measurement Point by channel ID and signal type
 * @param s signal data
 * @param chID channel ID (Real ID: 1,2,3,..)
 * @param stype Signal type
 * @details if the channel ID is invalid or the requested signal is
 * not absolute or raw, an exception is thrown!
 */
void MPoint::setSignal(const Signal & s, int chID, Signal::SType stype)
{
    QMutexLocker lock(&mutexChList);
    if(!isValidChannelID(chID, stype))
    {
        QString msg;
        msg.sprintf("MPoint: setSignal invalid channel ID: %d",chID);
        throw LIISimException(msg);
    }

    if(stype == Signal::RAW)
    {
        chList.at(chID-1)->raw = s;
        return;
    }
    else if(stype == Signal::ABS)
    {
        chList.at(chID-1)->absolute = s;
        return;
    }
    else if(stype == Signal::TEMPERATURE)
    {
        tempSignals.insert(chID,s);
        return;
    }
    throw LIISimException("MPoint: setSignal requested invalid signal type!");
}


/**
 * @brief MPoint::channelCount returns number of channels for given Signal::SType
 * @param stype Signal type
 * @return number of channels
 */
int MPoint::channelCount(Signal::SType stype)
{
    if(stype == Signal::TEMPERATURE)
    {
        return tempSignals.size();
    }
    else
        return this->channelCount_raw_abs;
}


/**
 * @brief MPoint::addTemperatureChannel adds a new temperature channel to this MPoint
 * @return channel-ID of new channel
 */
int MPoint::addTemperatureChannel(int tchid)
{
    if(tchid < 0)
        return -1;

    Signal ts;
    ts.type = Signal::TEMPERATURE;
    ts.channelID = tchid;
    tempSignals.insert(tchid,ts);

    return tchid;
}


/**
 * @brief MPoint::removeTemperatureChannel removes channel id from this MPoint
 * @param ch_id temperature channel ID
 */
void MPoint::removeTemperatureChannel(int ch_id)
{
    tempSignals.remove(ch_id);
}


/**
 * @brief MPoint::isValidChannelID returns true if a signal of Signal::SType
 * stype exists for given id within MPoint
 * @param id channel id of signal
 * @param stype Signal::SType
 * @param throwExcep if this function evaluates to false, a LIISimException is thrown (default true)
 * @return
 */
bool MPoint::isValidChannelID(int id, Signal::SType stype, bool throwExcep)
{
    bool res = true;
    if(stype == Signal::TEMPERATURE)
    {
        if(!tempSignals.keys().contains( id ))
        {
            res = false;
        }
    }
    else
    {
        if(id<1 || id > channelCount(stype))
        {
            res = false;
        }
    }

    if(!res && throwExcep)
    {
        QString msg;
        msg.sprintf("MPoint::isValidChannelID invalid channel ID: %d ",id);
        msg.append(" for signal type "+Signal::stypeToString(stype));
        throw LIISimException(msg);
    }

    return res;
}


/**
 * @brief MPoint::isValidFitTChannelID returns true if temperature signal has fitdata
 * @param chID temperature channel id
 * @return
 */
bool MPoint::isValidFitTChannelID(int chID)
{
    if(isValidChannelID(chID, Signal::TEMPERATURE, false))
    {
        return (this->getSignal(chID, Signal::TEMPERATURE).fitData.size() > 0);
    }
    else
        return false;
}


/**
 * @brief MPoint::channelIDs returns a list of all valid channel-IDs by Signal type
 * @param stype Signal::SType
 * @return list of channel-IDs
 */
QList<int> MPoint::channelIDs(Signal::SType stype)
{

    if(stype == Signal::TEMPERATURE)
    {
        return tempSignals.keys();
    }
    QList<int> res;
    for(int i = 0; i < channelCount_raw_abs; i++)
    {
        res << i+1;
    }
    return res;
}


/**
 * @brief MPoint::setTriggerTime set trigger time from DataAcquisition
 * @param triggerTime
 */
void MPoint::setTriggerTime(double triggerTime)
{
    trigger_time = triggerTime;
}


/**
 * @brief MPoint::getTriggerTime get trigger time from DataAcquisition
 * @return
 */
double MPoint::getTriggerTime()
{
    return trigger_time;
}

