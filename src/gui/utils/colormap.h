#ifndef COLORGEN_H
#define COLORGEN_H

#include <QObject>
#include <QColor>
#include <QList>
#include <QPointF>

/**
 * @brief The ColorMap class provides functions for generating interpolated colors based
 * on predefined colors or a custon linear color gradient
 * @ingroup GUI-Utilities
 */
class ColorMap : public QObject
{
    Q_OBJECT
public:

    enum class STYLE {
        GRAY,
        CUBEHELIX,
        OTHER
    };

    explicit ColorMap(QObject* parent = 0);

    void setToSingleColor(QColor color);
    void setToJetStyle();
    void setToHotStyle();
    void setToGrayStyle();
    void setToCubehelixColorMap();
    void setToLinearGradient(QList<QColor> colors);

    void setToGradient_VioletGreen();
    void setToGradient_highContrast();

    QColor color(double v, double vmin = 0.0, double vmax = 1.0);

    static ColorMap* jetColorMap();
    static ColorMap* hotColorMap();
    static ColorMap* grayColorMap();
    static ColorMap* cubehelixColorMap();

    QColor generateChannelColor(int channelID, QColor baseColor);
    QColor generateChannelColor(int channelID, int channelCount, QColor baseColor);

    bool gray;

    STYLE getCurrentStyle();

signals:

    /**
     * @brief modified This signal is emitted if any changes have been made to the colormap
     */
    void modified();

private:

    STYLE currentStyle;

    QList<QPointF> r_breaks;
    QList<QPointF> g_breaks;
    QList<QPointF> b_breaks;

};

#endif // COLORGEN_H
