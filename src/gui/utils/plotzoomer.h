#ifndef PLOTZOOMER_H
#define PLOTZOOMER_H

#include <qwt_plot_zoomer.h>

class PlotZoomer : public QwtPlotZoomer
{
public:
    PlotZoomer(QWidget *canvas, bool doReplot = true);
    PlotZoomer(int xAxis, int yAxis, QWidget *canvas, bool doReplot = true);

protected:
    virtual QSizeF minZoomSize () const;

};

#endif // PLOTZOOMER_H
