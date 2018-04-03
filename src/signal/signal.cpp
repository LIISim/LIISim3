#include "signal.h"

#include <algorithm>
#include <QDebug>
#include <qmath.h>

#include "../calculations/standarddeviation.h"
#include "../general/LIISimException.h"

Signal::Signal()
{
    this->start_time = 0.0;
    this->dt = 0.0;
    this->channelID = 0;
}


Signal::Signal(int size)
{
    this->dt = 0.0;
    this->start_time = 0.0;
    this->data.resize(size);
    this->dataDiameter.resize(size);
}


Signal::Signal(int size, double dt) : dt(dt)
{
    this->start_time = 0.0;
    this->data.resize(size);
    this->dataDiameter.resize(size);
}


Signal::Signal(int size, double dt, double start_time)
    : dt(dt), start_time(start_time)
{
    this->data.resize(size);
    this->dataDiameter.resize(size);
}


/**
 * @brief Signal::stypeToStrig returns a QString for given Signal::SType
 * @param stype Signal::SType
 * @return QString
 */
QString Signal::stypeToString(Signal::SType stype)
{
    QString res = "none";

    switch(stype)
    {
        case Signal::RAW:
            res = "Raw";
            break;
        case Signal::ABS:
            res = "Absolute";
            break;
        case Signal::TEMPERATURE:
            res = "Temperature";
            break;
    }
    return res;
}


Signal::SType Signal::stypeFromString(QString str)
{
    if(str == "Raw"||str =="raw")
        return Signal::RAW;
    else if(str == "Absolute"|| str == "absolute"|| str=="abs")
        return Signal::ABS;
    else if(str == "Temperature"|| str == "temperature")
        return Signal::TEMPERATURE;

    return Signal::RAW;
}



double Signal::time(int index)
{
    return start_time + dt * double(index);
}


/**
 * @brief Signal::maxTime get maximum time value (time at last signal data point)
 * @return time in seconds
 */
double Signal::maxTime() const
{
    return start_time + ( data.size() - 1 ) * dt;
}


/**
 * @brief Signal::at get signal value by time
 * @param time time in seconds
 * @return signal value at given time
 * @details This function returns the signal at given time or 0.0
 * if the requested time is out of the signal's data range.
 * If the requested time is inbetween two samples the signal
 * value is interpolated linearly.
 */
double Signal::at(double time)
{
    // check if out of range
    if(time < start_time || time > maxTime() )
    {        
        return 0.0;
    }

    // calculate double index and floor index
    double didx = ( time - start_time ) / dt;

    int i0 = qFloor( didx );

    // do linear interpolation if needed
    if(didx - i0 == 0.0)
    {
        return data.at( i0 );
    }
    else
    {
        int i1 = qCeil( didx );
        if(i1 >= data.size())
            i1 = data.size() - 1;

        double v0 = data.at( i0 );
        double v1 = data.at( i1 );
        return v0 + (v1-v0) / dt * ( time - start_time - i0 * dt );
    }
}



/**
 * @brief Signal::indexAt return index for data/fitData
 * @param time
 * @return
 */
int Signal::indexAt(double time)
{
    // check if out of range
    if(time < start_time || time > maxTime() )
    {
        return -1;
    }

    // calculate double index and floor index
    double didx = ( time - start_time ) / dt;

    int i0 = std::rint(didx);

    if(i0 > (data.size() - 1))
        i0 = data.size() - 1;

    return i0;
}


/**
 * @brief Signal::hasDataAt check if signal contains data for given time
 * @param time time in seconds
 * @return
 */
bool Signal::hasDataAt(double time) const
{
    if(time < start_time || time > maxTime() )
        return false;
    return true;
}


/**
 * @brief Resample signal data.
 * @param newStartTime new start time
 * @param newDt new sample size (time step)
 * @param noSamples new number of samples / data points
 * @throw
 */
void Signal::resample(double newStartTime, double newDt, int noSamples)
{
    if(this->data.isEmpty())
        throw LIISimException( "Signal::reSample: empty signal !" );

    // validate inputs
    if(noSamples <= 0)
        throw LIISimException( "Signal::reSample: number of samples must be > 0 !" );

    if(newDt <= 0 )
        throw LIISimException( "Signal::reSample: new delta time must be > 0 !" );


    QVector<double> newData;
    newData.fill(0.0,  noSamples);

    // sample data at new time step
    for(int i = 0; i < noSamples; i++)
    {
        double t = newStartTime + i * newDt;
        newData[i] = at(t);
    }

    // rewrite data and time information
    data = newData;
    start_time = newStartTime;
    dt = newDt;
}


double Signal::getMaxValue()
{
    return *(std::max_element(data.begin(), data.end()));
}


int Signal::getMaxIndex() const
{
    return std::distance(data.begin(), std::max_element(data.begin(), data.end()));
}


/**
 * @brief Signal::getSection
 * returns data[start] to data[end] of signal
 * @param start index of signal start
 * @param end index of signal end
 * @return Signal new Signal object with same properties as parent object
 */
Signal Signal::getSection(int start, int end)
{
    /**
     * NOT USED: should be reimplemented and then used in Plugin:GetSection
     */

    int size;
    double start_time;

    // get end of the signal
    if(end == 0) end = this->data.size();

    // calculate new properties
    size = end - start;
    start_time = this->start_time + double(start) * this->dt;

    // take properties from parent signal (type, channel, dt)
    Signal signal(*this);
    signal.start_time = start_time;     // set new starttime [s]
    signal.data.clear();                // clear data
    signal.data.resize(size);           // resize data vector

    // write data to new signal
    for(int i = start, j=0; i < end; i++, j++)
    {
        signal.data[j] = this->data[i];

        if(this->fitData.size() > 0)
            signal.fitData[j] = this->fitData[i];
    }

    return signal;
}


/**
 * @brief Signal::getSection
 * returns data between startTime and endTime
 * @param startTime of signal [s]
 * @param endTime of signal [s]
 * @return Signal new Signal object with same properties as parent object
 */
Signal Signal::getSection(double startTime, double endTime)
{
     // take properties from parent signal (type, channel, dt)
    Signal signal(*this);

    //qDebug() << startTime << " " << endTime;
    //qDebug() << hasDataAt(startTime) << " " << hasDataAt(endTime);

    // check if start and end time are within signal range
    if(!hasDataAt(startTime) && hasDataAt(endTime))
    {
        startTime = start_time;
    }
    else if(hasDataAt(startTime) && !hasDataAt(endTime))
    {
        endTime = maxTime();
    }
    else if(!hasDataAt(startTime) && !hasDataAt(endTime))
    {
        return signal;
    }

    signal.data.clear();

    double didx;

    //get index for startTime
    didx = (startTime - start_time ) / dt;
    int idx_start = qFloor( didx );

    //get index for endTime
    didx = (endTime - start_time ) / dt;
    int idx_end = qFloor( didx );

    //qDebug() << "start: " << idx_start << " end" << idx_end;


    signal.data.resize(idx_end - idx_start); // resize data vector

    // write data to new signal
    for(int i = idx_start, j=0; i < idx_end; i++, j++)
    {
        signal.data[j] = this->data[i];
    }

    return signal;
}


double Signal::calcRangeAverage(double start, double end)
{
    double avg = 0.0;

    Signal dataInterval = getSection(start, end);

    for(int k = 0; k < dataInterval.data.size();k++)
    {
      avg += dataInterval.data.at(k); // sum of data
    }

    avg /= double(dataInterval.data.size());

    return avg;
}


QPair<double, double> Signal::calcRangeAverageStdev(double start, double end)
{
    StandardDeviation stdv;
    Signal dataInterval = getSection(start, end);

    stdv.addVector(dataInterval.data);

    QPair<double, double> result;

    result.first  = stdv.getMean();
    result.second = stdv.getStandardDeviation();

    return result;
}
