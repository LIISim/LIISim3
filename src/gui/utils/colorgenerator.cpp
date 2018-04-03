#include "colorgenerator.h"

ColorGenerator::ColorGenerator(Mode mode)
{
    mChannelCount = 4;
    mRunCount = 1;

    mMode = mode;
}


void ColorGenerator::setParameter(int runCount, int channelCount)
{
    if(runCount > 0)
        mRunCount = runCount;
    else
        mRunCount = 1;

    if(channelCount > 0)
        mChannelCount = channelCount;
    else
        channelCount = 1;
}

void ColorGenerator::setChannelCount(int channelCount)
{
    if(channelCount > 0)
        mChannelCount = channelCount;
    else
        channelCount = 1;
}


void ColorGenerator::setRunColor(int run, QColor color)
{
    mRunColors.insert(run, color);
}


QColor ColorGenerator::getColor(int run, int channel)
{
    if(mMode == DYNAMIC_RUN_COLOR)
    {
        int hue = (340 / mChannelCount) * channel + 10;
        int sat = (250 / mRunCount) * run + 5;

        QColor ret(QColor::fromHsv(hue, sat, 220, 180));
        return ret;
    }
    else //STATIC_RUN_COLOR
    {
        int sat = (250 / mChannelCount) * channel + 5;
        QColor baseColor = mRunColors.value(run);

        QColor ret(QColor::fromHsv(baseColor.hue(), sat, 255, 180));
        return ret;
    }
    return QColor(Qt::black);
}


void ColorGenerator::registerCurve(FT_PlotCurve *curve, int channel)
{
    if(!registeredCurves.contains(channel))
        registeredCurves.insert(channel, new QList<FT_PlotCurve*>());
    registeredCurves.value(channel)->push_back(curve);
}


void ColorGenerator::unregisterCurve(FT_PlotCurve *curve, int channel)
{
    if(!registeredCurves.contains(channel))
        return;
    if(registeredCurves.value(channel)->contains(curve))
        registeredCurves.value(channel)->removeAll(curve);
}


void ColorGenerator::updateCurveColors()
{
    for(int key : registeredCurves.keys())
        updateCurveColors(key);
}


void ColorGenerator::updateCurveColors(int channel)
{
    if(!registeredCurves.contains(channel))
        return;

    QList<FT_PlotCurve*> *curves = registeredCurves.value(channel);

    QList<FT_PlotCurve*> attachedCurves;

    for(FT_PlotCurve *curve : *curves)
        if(curve->isAttached())
            attachedCurves.push_back(curve);

    QColor baseColor;
    if(!mRunColors.contains(channel))
        baseColor = QColor(Qt::black);
    else
        baseColor = mRunColors.value(channel);

    for(int i = 0; i < attachedCurves.size(); i++)
    {
        int sat = (250 / attachedCurves.size()) * (i+1) + 5;
        int value = 200;
        if(baseColor.value() > 180)
            value = ((baseColor.value() / 2) / attachedCurves.size()) * (attachedCurves.size() - i) + (baseColor.value() / 2);
        else
            value = ((255 / 2) / attachedCurves.size()) * (attachedCurves.size() - i) + (255 / 2);
        QColor curveColor(QColor::fromHsv(baseColor.hue(), sat, value, 180));

        QPen pen = attachedCurves.at(i)->pen();
        pen.setColor(curveColor);
        attachedCurves.at(i)->setPen(pen);
    }
}
