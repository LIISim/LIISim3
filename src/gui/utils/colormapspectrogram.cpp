#include "colormapspectrogram.h"

ColorMapSpectrogram::ColorMapSpectrogram() : QwtLinearColorMap( Qt::color0, Qt::darkRed)
{
    addColorStop(-1.0, Qt::transparent);
    addColorStop(0.0, Qt::black);
    addColorStop(0.02, Qt::darkBlue);
    addColorStop(0.1, Qt::blue);
    addColorStop(0.2, Qt::cyan);
    addColorStop(0.3, Qt::yellow);
    addColorStop(0.8, Qt::red);

}

