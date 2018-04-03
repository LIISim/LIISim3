#ifndef DAQIO_H
#define DAQIO_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QPair>
#include <QMutex>
#include <QMap>

#include <externalLibraries/boost/boost/circular_buffer.hpp>

extern "C"
{
    #include "../externalLibraries/nidaqmx/include/NIDAQmx.h"
}

class DAQAnalogOut
{
public:
    DAQAnalogOut();
    DAQAnalogOut(unsigned int internalHandle, QString channel, float64 out);

    unsigned int internalHandle;
    TaskHandle taskHandle;
    QString channel;
    float64 out;
    QMutex *lock;
    bool voltageChanged;
};


class DAQAnalogIn
{
public:
    DAQAnalogIn();
    DAQAnalogIn(unsigned int internalHandle, QString channel, unsigned int averageBufferSize);

    unsigned int internalHandle;
    TaskHandle taskHandle;
    QString channel;

    double lastValue;
    double lastAverage;

    boost::circular_buffer<float64> buffer;
};


class DAQDigitalOutHelper
{
public:
    DAQDigitalOutHelper(unsigned int internalHandle, unsigned int channelNumber, bool enabled, bool inverted);
    unsigned int internalHandle;
    unsigned int channelNumber;
    bool enabled;
    bool inverted;
};


class DAQDigitalOut
{
public:
    DAQDigitalOut();
    DAQDigitalOut(QString dev, QString port);

    QMutex *lock;
    QString dev;
    QString port;
    //internal handle - channel number;
    //QMap<unsigned int, unsigned int> handleChannelMap;
    TaskHandle taskHandle;
    bool enabled;
    bool changed;
    uInt32 out[1];
    QList<DAQDigitalOutHelper> list;
};

#endif // DAQIO_H
