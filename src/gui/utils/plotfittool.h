#ifndef PLOTFITTOOL_H
#define PLOTFITTOOL_H


#include "plotanalysistool.h"

class PlotFitTool : public PlotAnalysisTool
{
    Q_OBJECT

    public:
        explicit PlotFitTool(QwtPlot* plot,
                             QList<BasePlotCurve *> *curves,
                             QObject *parent = 0);
        ~PlotFitTool();
        void updatePlot(double xStart, double xEnd);
        void hideCustom();

        QList<BasePlotCurve*> fit_curves;

};

#endif // PLOTFITTOOL_H
