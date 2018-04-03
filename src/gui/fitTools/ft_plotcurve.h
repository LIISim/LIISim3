#ifndef FT_PLOTCURVE_H
#define FT_PLOTCURVE_H

#include "../utils/customQwtPlot/signalplotcurve.h"
//#include "../utils/colorgenerator.h"

class SignalPlotWidgetQwt;
class ColorGenerator;

class FT_PlotCurve : public SignalPlotCurve
{
public:
    explicit FT_PlotCurve(const QString &title, SignalPlotWidgetQwt *plot, const Signal &signal);

    void setSignal(const Signal &signal);

    bool isAttached() {return mAttached;}

    void enable();
    void disable();

    static ColorGenerator* getCGInstance();

private:
    SignalPlotWidgetQwt *mPlot;

    bool mAttached;

    static ColorGenerator *s_cgInstance;
};

#endif // FT_PLOTCURVE_H
