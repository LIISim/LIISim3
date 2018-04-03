#ifndef SIGNALPLOTINTERVALCURVE_H
#define SIGNALPLOTINTERVALCURVE_H

#include <qwt_plot_intervalcurve.h>
#include "../../signal/signal.h"

class SignalPlotIntervalCurve : public QwtPlotIntervalCurve
{
    public:

        SignalPlotIntervalCurve(const QString & title, const Signal &signal = Signal());
        ~SignalPlotIntervalCurve();
        inline int chID(){ return m_chid; }

    private:
        int m_chid;
};

#endif // SIGNALPLOTINTERVALCURVE_H
