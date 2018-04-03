#include "signalplotcurve.h"

SignalPlotCurve::SignalPlotCurve(
        const QString &title,
        const Signal &signal)  : BasePlotCurve(title)
{
    m_chid = signal.channelID;    
}


SignalPlotCurve::~SignalPlotCurve() {}
