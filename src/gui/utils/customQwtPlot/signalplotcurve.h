#ifndef SIGNALPLOTCURVE_H
#define SIGNALPLOTCURVE_H

#include "baseplotcurve.h"
#include "../../signal/signal.h"


/**
 * @brief The SignalPlotCurve class: custom QwtPlotCurve class
 * This class is used for visualization of signal data
 */
class SignalPlotCurve : public BasePlotCurve
{

    public:
        SignalPlotCurve(const QString & title, const Signal &signal = Signal());
        ~SignalPlotCurve();
        inline int chID(){return m_chid;}

    private:
        int m_chid;

};

#endif // SIGNALPLOTCURVE_H
