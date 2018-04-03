#include "ft_plotcurve.h"

#include "../utils/signalplotwidgetqwt.h"
#include "../utils/colorgenerator.h"

ColorGenerator* FT_PlotCurve::s_cgInstance = 0;

FT_PlotCurve::FT_PlotCurve(const QString &title, SignalPlotWidgetQwt *plot, const Signal &signal) : SignalPlotCurve(title, signal)
{
    mPlot = plot;
    setSignal(signal);
}


void FT_PlotCurve::setSignal(const Signal &signal)
{
    QVector<double> xData;
    QVector<double> yData;
    for(int i = 0; i < signal.data.size(); i++)
    {
        xData.push_back((signal.start_time + i * signal.dt) * 1.e9);
        yData.push_back(signal.data.at(i));
    }
    setSamples(xData, yData);
}


void FT_PlotCurve::enable()
{
    mPlot->registerCurve(this); //FIXME: needs to optimized...
    mAttached = true;
}


void FT_PlotCurve::disable()
{
    mPlot->unregisterCurve(this);
    mAttached = false;
}


ColorGenerator* FT_PlotCurve::getCGInstance()
{
    if(!s_cgInstance)
    {
        s_cgInstance = new ColorGenerator(ColorGenerator::STATIC_RUN_COLOR);
        s_cgInstance->setRunColor(0, QColor(Qt::darkGreen));    //data curve
        s_cgInstance->setRunColor(1, QColor(Qt::magenta));      //temperature curve
        s_cgInstance->setRunColor(2, QColor(Qt::magenta));      //particle diameter curve
        s_cgInstance->setRunColor(3, QColor(Qt::magenta));      //particle start diameter curve
        s_cgInstance->setRunColor(4, QColor(Qt::magenta));      //gas temperature curve
        s_cgInstance->setRunColor(5, QColor(Qt::magenta));      //fit error curve

        s_cgInstance->setRunColor(6, QColor(Qt::cyan));         //sim curve (general)
        s_cgInstance->setRunColor(61, QColor(Qt::red));         //sim curve (EULER)
        s_cgInstance->setRunColor(62, QColor(Qt::green));       //sim curve (RKD5_OPT)
        s_cgInstance->setRunColor(63, QColor(Qt::blue));        //sim curve (RKD5)

        s_cgInstance->setRunColor(7, QColor(Qt::red));          //conduction curve
        s_cgInstance->setRunColor(8, QColor(Qt::green));        //evaporation curve
        s_cgInstance->setRunColor(9, QColor(Qt::blue));         //radiation curve
    }
    return s_cgInstance;
}
