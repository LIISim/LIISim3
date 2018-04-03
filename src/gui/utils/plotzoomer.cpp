#include "plotzoomer.h"

PlotZoomer::PlotZoomer(QWidget *canvas, bool doReplot) : QwtPlotZoomer(canvas, doReplot)
{

}


PlotZoomer::PlotZoomer(int xAxis, int yAxis, QWidget *canvas, bool doReplot) : QwtPlotZoomer(xAxis, yAxis, canvas, doReplot)
{

}


QSizeF PlotZoomer::minZoomSize() const
{
    return QSizeF(zoomBase().width() / 10e30, zoomBase().height() / 10e30);
}
