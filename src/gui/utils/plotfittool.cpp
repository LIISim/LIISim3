#include "plotfittool.h"

#include <qwt_plot.h>

#include "customQwtPlot/baseplotcurve.h"

PlotFitTool::PlotFitTool(QwtPlot *plot,
                         QList<BasePlotCurve*>* curves,
                         QObject *parent)
                        : PlotAnalysisTool(plot, curves, false, parent)
{
}

/**
 * @brief PlotFitTool::~PlotFitTool Destructor
 */
PlotFitTool::~PlotFitTool()
{
}

void PlotFitTool::hideCustom()
{
    while(!fit_curves.isEmpty())
    {
        fit_curves.front()->detach();
        delete fit_curves[0];
        fit_curves.pop_front();
    }
}

/**
 * @brief PlotFitTool::updatePlot
 * http://www.codewithc.com/c-program-for-linear-exponential-curve-fitting/
 * @param xStart
 * @param xEnd
 */
void PlotFitTool::updatePlot(double xStart, double xEnd)
{

    // reset fit curves from previous call
    while(!fit_curves.isEmpty())
    {
        fit_curves.front()->detach();
        delete fit_curves[0];
        fit_curves.pop_front();
    }

    QList<BasePlotCurve*>* active_curves = getCurves();

    QString labelText = QString("<b>Fit: </b> (dt = %0ns, n=%1) <br> start: %2 ns; end: %3 ns ")
            .arg(QString::number(xEnd-xStart,'f',1))
            .arg(QString::number(active_curves->size(),'f',0))
            .arg(QString::number(xStart,'f',1))
            .arg(QString::number(xEnd,'f',1));

    int count = 0;
    double A, a,b, sumx=0.0,sumy=0.0,sumxy=0.0,sumx2=0.0;
    QVector<double> xData, yData, yFit;

    for(int i = 0; i < active_curves->size(); i++)
    {
        BasePlotCurve* curve = active_curves->at(i);

        // clear temp vars
        a = 0.0;
        b = 0.0;
        xData.clear();
        yData.clear();
        yFit.clear();

        sumx=0.0;
        sumy=0.0;
        sumxy=0.0;
        sumx2=0.0;
        count = 0;

        for(int j = 0; j < curve->dataSize(); j++)
        {
            double x = curve->sample(j).x();

            // check if data are in range
            if(x < xStart)
                continue;
            if(x > xEnd)
                break;

            double y = curve->sample(j).y();

            // fill vectors
            xData.append(x);
            yData.append(y);

            sumx    = sumx + x;
            sumx2   = sumx2 + x*x;
            sumy    = sumy + log(y);
            sumxy   = sumxy + x*log(y);
            count++;
        }

        // calculate parameters
        A=((sumx2*sumy -sumx*sumxy)*1.0/(count*sumx2-sumx*sumx)*1.0);
        b=((count*sumxy-sumx*sumy)*1.0/(count*sumx2-sumx*sumx)*1.0);
        a = exp(A);

        for(int k=0; k < xData.size(); k++)
        {
            yFit.append(a*exp(b*xData[k]));
        }

        BasePlotCurve* curve_fit = new BasePlotCurve("PlotFitTool");
        curve_fit->setSamples(xData,yFit);
        curve_fit->attach(plot);

        // save curves so that they can be found later
        fit_curves << curve_fit;

        labelText.append(QString("<br>&nbsp;<font color='%0'>&#9632;</font>&nbsp;%1: &nbsp;&nbsp;&nbsp;y = %2 * exp(%3 * x)")
                .arg(curve->pen().color().name())
                .arg(curve->title().text())
                .arg(QString::number(a,'f',4))
                .arg(QString::number(b,'f',4)));

    }

    // display created text
    updateInfoBox(labelText);
}
