#ifndef COLORGENERATOR_H
#define COLORGENERATOR_H

#include <QColor>
#include <QMap>

#include "../fitTools/ft_plotcurve.h"

class ColorGenerator
{
public:
    enum Mode {
        DYNAMIC_RUN_COLOR,
        STATIC_RUN_COLOR,
    };

    ColorGenerator(Mode mode);

    void setParameter(int runCount, int channelCount);
    void setChannelCount(int channelCount);
    void setRunColor(int run, QColor color);

    QColor getColor(int run, int channel);

    void registerCurve(FT_PlotCurve *curve, int channel);
    void unregisterCurve(FT_PlotCurve *curve, int channel);

    void updateCurveColors();
    void updateCurveColors(int channel);

private:
    int mRunCount;
    int mChannelCount;

    Mode mMode;

    QMap<int, QColor> mRunColors;

    QMap<int, QList<FT_PlotCurve*>*> registeredCurves;

};

#endif // COLORGENERATOR_H
