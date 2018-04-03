#ifndef COLORMAPSPECTROGRAM_H
#define COLORMAPSPECTROGRAM_H

#include <qwt_color_map.h>

class ColorMapSpectrogram : public QwtLinearColorMap
{
public:
    ColorMapSpectrogram();
    QRgb rgb(const QwtInterval &interval, double value) const
        {
            return QwtLinearColorMap::rgb(QwtInterval(std::log(interval.minValue()),
                                                      std::log(interval.maxValue())),
                                          std::log(value));
        }

};

#endif // COLORMAPSPECTROGRAM_H
