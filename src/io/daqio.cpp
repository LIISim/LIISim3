#include "daqio.h"

//--- Analog OUT

DAQAnalogOut::DAQAnalogOut()
{
    internalHandle = 0;
    taskHandle = 0;
    channel = QString("");
    out = 0;
    lock = new QMutex();
    voltageChanged = true;
}


DAQAnalogOut::DAQAnalogOut(unsigned int internalHandle, QString channel, float64 out)
{
    this->internalHandle = internalHandle;
    this->channel = channel;
    this->out = out;
    taskHandle = 0;
    lock = new QMutex();
    voltageChanged = true;
}


//--- Analog IN

DAQAnalogIn::DAQAnalogIn(unsigned int internalHandle, QString channel, unsigned int averageBufferSize)
{
    this->internalHandle = internalHandle;
    this->channel = channel;
    taskHandle = 0;
    buffer.set_capacity(averageBufferSize);
}


DAQAnalogIn::DAQAnalogIn()
{
    internalHandle = 0;
    channel = QString("");
    taskHandle = 0;
    lastValue = 0.0;
    lastAverage = 0.0;
    buffer.set_capacity(1);
}


//--- Digital OUT

DAQDigitalOut::DAQDigitalOut()
{
    lock = new QMutex();
    dev = QString("");
    port = QString("");
    taskHandle = 0;
    enabled = false;
    changed = false;
    out[0] = 0;
}


DAQDigitalOut::DAQDigitalOut(QString dev, QString port)
{
    lock = new QMutex();
    this->dev = dev;
    this->port = port;
    taskHandle = 0;
    enabled = false;
    changed = false;
    out[0] = 0;
}


DAQDigitalOutHelper::DAQDigitalOutHelper(unsigned int internalHandle, unsigned int channelNumber, bool enabled, bool inverted)
{
    this->internalHandle = internalHandle;
    this->channelNumber = channelNumber;
    this->enabled = enabled;
    this->inverted = inverted;
}
