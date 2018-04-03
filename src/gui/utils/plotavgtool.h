#ifndef PLOTAVGTOOL_H
#define PLOTAVGTOOL_H

#include "plotanalysistool.h"

/**
 * @brief The PlotAvgTool class adds an interactive tool for
 * the seledtion of a ROI (region of interest)on the x-axis to
 * a QwtPlot.
 * For all curves within the ROI the average
 * y-value as well as the standard deviation (in %) are calculated and displayed
 * in realtime.
 */
class PlotAvgTool : public PlotAnalysisTool
{
    Q_OBJECT
    public:
        explicit PlotAvgTool(QwtPlot* plot,
                              QList<BasePlotCurve*>* curves,
                              QObject *parent = 0);
        ~PlotAvgTool();

        void updatePlot(double xStart, double xEnd);
        void hideCustom() {} // do nothing
};

#endif // PLOTAVGTOOL_H
