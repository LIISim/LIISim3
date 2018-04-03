#include "signalplotintervalcurve.h"

SignalPlotIntervalCurve::SignalPlotIntervalCurve(
        const QString &title,
        const Signal &signal)  : QwtPlotIntervalCurve(title)
{
 m_chid = signal.channelID;
}

SignalPlotIntervalCurve::~SignalPlotIntervalCurve() {}
