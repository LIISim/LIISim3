#ifndef STREAMPOINT_H
#define STREAMPOINT_H

#include <QMap>

#include "signal.h"

class SignalPlotWidgetQwt;

class StreamPoint
{
public:
    StreamPoint();

    void draw(SignalPlotWidgetQwt *plot);
    void draw(SignalPlotWidgetQwt *plot, QList<unsigned int> signalPlotOrder);

    unsigned int averageBufferFilling;

    bool overflowA;
    bool overflowB;
    bool overflowC;
    bool overflowD;

    //Contains the signal data, consecutively numbered according to channels, starting with 1
    QMap<unsigned int, Signal> single;
    QMap<unsigned int, Signal> average;
};

#endif // STREAMPOINT_H
