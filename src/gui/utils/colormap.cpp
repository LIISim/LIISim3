#include "colormap.h"

#include <QList>
#include <QPointF>
#include <QDebug>


/**
 * @brief ColorMap::ColorMap Constructor. Creates empty (black) colormap
 * @param parent
 */
ColorMap::ColorMap(QObject* parent) : QObject(parent)
{
 //   r_breaks.append(QPointF(0,0));
 //   g_breaks.append(QPointF(0,0));
 //   b_breaks.append(QPointF(0,0));
    gray = false;
}


/**
 * @brief ColorMap::setJetStyle transforms this colormap to a 'jet' style colormap
 */
void ColorMap::setToJetStyle()
{
    gray = false;

    currentStyle = STYLE::OTHER;

    r_breaks.clear();
    g_breaks.clear();
    b_breaks.clear();

    ColorMap* cmap = ColorMap::jetColorMap();
    r_breaks = cmap->r_breaks;
    g_breaks = cmap->g_breaks;
    b_breaks = cmap->b_breaks;
    delete cmap;

    emit modified();
}


/**
 * @brief ColorMap::setToHotStyle transforms this colormap to a 'hot' style colormap
 */
void ColorMap::setToHotStyle()
{
    gray = false;

    currentStyle = STYLE::OTHER;

    r_breaks.clear();
    g_breaks.clear();
    b_breaks.clear();

    ColorMap* cmap = ColorMap::hotColorMap();
    r_breaks = cmap->r_breaks;
    g_breaks = cmap->g_breaks;
    b_breaks = cmap->b_breaks;
    delete cmap;

    emit modified();
}


void ColorMap::setToGrayStyle()
{
    gray = true;

    currentStyle = STYLE::GRAY;

    r_breaks.clear();
    g_breaks.clear();
    b_breaks.clear();

    ColorMap *cmap = ColorMap::grayColorMap();

    r_breaks = cmap->r_breaks;
    g_breaks = cmap->g_breaks;
    b_breaks = cmap->b_breaks;

    delete cmap;

    emit modified();
}


void ColorMap::setToCubehelixColorMap()
{
    gray = false;

    currentStyle = STYLE::CUBEHELIX;

    r_breaks.clear();
    g_breaks.clear();
    b_breaks.clear();

    ColorMap* cmap = ColorMap::cubehelixColorMap();

    r_breaks = cmap->r_breaks;
    g_breaks = cmap->g_breaks;
    b_breaks = cmap->b_breaks;
    delete cmap;

    emit modified();
}


void ColorMap::setToSingleColor(QColor color)
{
    gray = false;

    currentStyle = STYLE::OTHER;

    r_breaks.clear();
    g_breaks.clear();
    b_breaks.clear();

    r_breaks << QPointF(0.0, ((double)color.red())/255.0 );
    g_breaks << QPointF(0.0, ((double)color.green())/255.0 );
    b_breaks << QPointF(0.0, ((double)color.blue())/255.0 );

    emit modified();
}


/**
 * @brief ColorMap::setToLinearGradient setup a linear color gradient
 * @param colors list of colors (at least two)
 */
void ColorMap::setToLinearGradient(QList<QColor> colors)
{
    gray = false;

    currentStyle = STYLE::OTHER;

    int n = colors.size();
    if( n < 2)
        return;

    r_breaks.clear();
    g_breaks.clear();
    b_breaks.clear();

    double step = 1.0 / ((double)(n-1));

    double curstep = 0.0;
    for(int i = 0; i < n; i++)
    {
        if(i == n-1) //make sure that last value is exactly 1
            curstep = 1.0;

        QColor curColor = colors.at(i);
        r_breaks.append(QPointF( curstep, ((double)curColor.red())/255.0 ));
        g_breaks.append(QPointF( curstep, ((double)curColor.green())/255.0 ));
        b_breaks.append(QPointF( curstep, ((double)curColor.blue())/255.0 ));

        curstep += step;
    }

    emit modified();
}


/**
 * @brief ColorMap::gradient_VioletGreen nice violet to green gradient
 */
void ColorMap::setToGradient_VioletGreen()
{
    gray = false;

    QColor c1;
    QColor c2;
    QColor c3;

    c1.setNamedColor("#CC00F0");
    c2.setNamedColor("#0AFFBA");
    c3.setNamedColor("#D4F000");

    QList<QColor> colors;
    colors << c1 << c2 << c3;
    setToLinearGradient(colors);
}


/**
 * @brief ColorMap::gradient_highContrast somehow ugly but high contrast gradient
 */
void ColorMap::setToGradient_highContrast()
{
    gray = false;

    QList<QColor> colors;
    colors << QColor(  0,  0,  0);
    colors << QColor(  0,255,255);

    colors << QColor(  0,  0,255);
    colors << QColor(255,255,  0);

    colors << QColor(  0,255,  0);
    colors << QColor(255,  0,255);

    colors << QColor(255,  0,  0);
    colors << QColor(240,240,240);

    setToLinearGradient(colors);
}


/**
 * @brief ColorMap::color get interpolated color value
 * @param v value position (should be between vmin and vmax)
 * @param vmin
 * @param vmax
 * @return
 */
QColor ColorMap::color(double v, double vmin, double vmax)
{
    if(r_breaks.size() == 1 &&
       g_breaks.size() == 1 &&
       b_breaks.size() == 1)
    {
        return QColor(  r_breaks[0].y()*255,
                        g_breaks[0].y()*255,
                        b_breaks[0].y()*255);
    }


    double dv;

    if (v < vmin)
      v = vmin;
    if (v > vmax)
      v = vmax;
    dv = vmax - vmin;
    if(dv <= 0.0)
        dv = 1e-20;

    double iv = (v-vmin)/dv;

    // determine 2 color breaks for each color channel for lin. interp.
    QPointF r1,r2,g1,g2,b1,b2;

    for(int i = 0; i < r_breaks.size(); i++)
    {
        QPointF cp = r_breaks.at(i);
        if(iv <= cp.x() )
        {
            r2 = cp;
            if(i > 0)
                r1 = r_breaks.at(i-1);
            else
                r1 = r2;
            break;
        }
    }

    for(int i = 0; i < g_breaks.size(); i++)
    {
        QPointF cp = g_breaks.at(i);
        if(iv <= cp.x() )
        {
            g2 = cp;
            if(i > 0)
                g1 = g_breaks.at(i-1);
            else
                g1 = g2;
            break;
        }
    }

    for(int i = 0; i < b_breaks.size(); i++)
    {
        QPointF cp = b_breaks.at(i);
        if(iv <= cp.x() )
        {
            b2 = cp;
            if(i > 0)
                b1 = b_breaks.at(i-1);
            else
                b1 = b2;
            break;
        }
    }

    // get interpolated r,g,b values
    double r,g,b,alpha;

    if(r2.x() - r1.x() <= 0)
        r2.setX( r2.x() + 1e-20 );

    alpha = ( iv-r1.x() ) / (r2.x() - r1.x());
    r = ( 1 - alpha) * r1.y() + alpha * r2.y();


    if(g2.x() - g1.x() <= 0)
        g2.setX( g2.x() + 1e-20 );

    alpha = ( iv-g1.x() ) / (g2.x() - g1.x());
    g = ( 1 - alpha) * g1.y() + alpha * g2.y();


    if(b2.x() - b1.x() <= 0)
        b2.setX( b2.x() + 1e-20 );

    alpha = ( iv-b1.x() ) / (b2.x() - b1.x());

    b = ( 1 - alpha) * b1.y() + alpha * b2.y();

    QColor res(r*255,g*255,b*255);
    return res;
}


/**
 * @brief ColorMap::jetColorMap generates a jet style colormap
 * @return
 */
ColorMap* ColorMap::jetColorMap()
{
    ColorMap* cmap = new ColorMap;

    cmap->b_breaks.append(QPointF( 0.00 , 0.5 ));
    cmap->b_breaks.append(QPointF( 0.17 , 1.0 ));
    cmap->b_breaks.append(QPointF( 0.38 , 1.0 ));
    cmap->b_breaks.append(QPointF( 0.62 , 0.0 ));
    cmap->b_breaks.append(QPointF( 1.00 , 0.0 ));

    cmap->r_breaks.append(QPointF( 0.00 , 0.0 ));
    cmap->r_breaks.append(QPointF( 0.38 , 0.0 ));
    cmap->r_breaks.append(QPointF( 0.62 , 1.0 ));
    cmap->r_breaks.append(QPointF( 0.90 , 1.0 ));
    cmap->r_breaks.append(QPointF( 1.00 , 0.5 ));

    cmap->g_breaks.append(QPointF( 0.00 , 0.0 ));
    cmap->g_breaks.append(QPointF( 0.17 , 0.0 ));
    cmap->g_breaks.append(QPointF( 0.38 , 1.0 ));
    cmap->g_breaks.append(QPointF( 0.62 , 1.0 ));
    cmap->g_breaks.append(QPointF( 0.90 , 0.0 ));
    cmap->g_breaks.append(QPointF( 1.00 , 0.0 ));
    return cmap;
}


/**
 * @brief ColorMap::hotColorMap generates a colormap in 'hot' style
 * @return
 */
ColorMap* ColorMap::hotColorMap()
{
    ColorMap* cmap = new ColorMap;

    cmap->b_breaks.append(QPointF( 0.00 , 0.0 ));
    cmap->b_breaks.append(QPointF( 0.75 , 0.0 ));
    cmap->b_breaks.append(QPointF( 1.00 , 0.8 ));

    cmap->r_breaks.append(QPointF( 0.00 , 0.0 ));
    cmap->r_breaks.append(QPointF( 0.4 ,  0.9 ));
    cmap->r_breaks.append(QPointF( 1.00 , 0.9 ));

    cmap->g_breaks.append(QPointF( 0.00 , 0.0 ));
    cmap->g_breaks.append(QPointF( 0.40 , 0.0 ));
    cmap->g_breaks.append(QPointF( 0.70 , 0.9 ));
    cmap->g_breaks.append(QPointF( 1.00 , 0.9 ));
    return cmap;
}


ColorMap* ColorMap::grayColorMap()
{
    ColorMap *cmap = new ColorMap;

    cmap->r_breaks.append(QPointF(0.0 , 0.0));
    cmap->r_breaks.append(QPointF(0.1 , 0.1));
    cmap->r_breaks.append(QPointF(0.2 , 0.2));
    cmap->r_breaks.append(QPointF(0.3 , 0.3));
    cmap->r_breaks.append(QPointF(0.4 , 0.4));
    cmap->r_breaks.append(QPointF(0.5 , 0.5));
    cmap->r_breaks.append(QPointF(0.6 , 0.6));
    cmap->r_breaks.append(QPointF(0.7 , 0.7));
    cmap->r_breaks.append(QPointF(0.8 , 0.8));
    cmap->r_breaks.append(QPointF(0.9 , 0.9));
    cmap->r_breaks.append(QPointF(1.0 , 1.0));

    cmap->g_breaks.append(QPointF(0.0 , 0.0));
    cmap->g_breaks.append(QPointF(0.1 , 0.1));
    cmap->g_breaks.append(QPointF(0.2 , 0.2));
    cmap->g_breaks.append(QPointF(0.3 , 0.3));
    cmap->g_breaks.append(QPointF(0.4 , 0.4));
    cmap->g_breaks.append(QPointF(0.5 , 0.5));
    cmap->g_breaks.append(QPointF(0.6 , 0.6));
    cmap->g_breaks.append(QPointF(0.7 , 0.7));
    cmap->g_breaks.append(QPointF(0.8 , 0.8));
    cmap->g_breaks.append(QPointF(0.9 , 0.9));
    cmap->g_breaks.append(QPointF(1.0 , 1.0));

    cmap->b_breaks.append(QPointF(0.0 , 0.0));
    cmap->b_breaks.append(QPointF(0.1 , 0.1));
    cmap->b_breaks.append(QPointF(0.2 , 0.2));
    cmap->b_breaks.append(QPointF(0.3 , 0.3));
    cmap->b_breaks.append(QPointF(0.4 , 0.4));
    cmap->b_breaks.append(QPointF(0.5 , 0.5));
    cmap->b_breaks.append(QPointF(0.6 , 0.6));
    cmap->b_breaks.append(QPointF(0.7 , 0.7));
    cmap->b_breaks.append(QPointF(0.8 , 0.8));
    cmap->b_breaks.append(QPointF(0.9 , 0.9));
    cmap->b_breaks.append(QPointF(1.0 , 1.0));

    cmap->gray = true;

    return cmap;
}


ColorMap* ColorMap::cubehelixColorMap()
{
    ColorMap *cmap = new ColorMap;

    cmap->r_breaks.append(QPointF(0.000, 0.000));
//    cmap->r_breaks.append(QPointF(0.010, 0.128));
//    cmap->r_breaks.append(QPointF(0.020, 0.182));
//    cmap->r_breaks.append(QPointF(0.030, 0.219));
//    cmap->r_breaks.append(QPointF(0.040, 0.243));
//    cmap->r_breaks.append(QPointF(0.050, 0.259));
//    cmap->r_breaks.append(QPointF(0.060, 0.269));
//    cmap->r_breaks.append(QPointF(0.070, 0.272));
//    cmap->r_breaks.append(QPointF(0.080, 0.271));
//    cmap->r_breaks.append(QPointF(0.090, 0.266));
//    cmap->r_breaks.append(QPointF(0.100, 0.257));
//    cmap->r_breaks.append(QPointF(0.110, 0.246));
    cmap->r_breaks.append(QPointF(0.120, 0.232));
    cmap->r_breaks.append(QPointF(0.130, 0.217));
    cmap->r_breaks.append(QPointF(0.140, 0.201));
    cmap->r_breaks.append(QPointF(0.150, 0.185));
    cmap->r_breaks.append(QPointF(0.160, 0.169));
    cmap->r_breaks.append(QPointF(0.170, 0.153));
    cmap->r_breaks.append(QPointF(0.180, 0.138));
    cmap->r_breaks.append(QPointF(0.190, 0.124));
    cmap->r_breaks.append(QPointF(0.200, 0.112));
    cmap->r_breaks.append(QPointF(0.210, 0.102));
    cmap->r_breaks.append(QPointF(0.220, 0.095));
    cmap->r_breaks.append(QPointF(0.230, 0.090));
    cmap->r_breaks.append(QPointF(0.240, 0.088));
    cmap->r_breaks.append(QPointF(0.250, 0.089));
    cmap->r_breaks.append(QPointF(0.260, 0.093));
    cmap->r_breaks.append(QPointF(0.270, 0.101));
    cmap->r_breaks.append(QPointF(0.280, 0.112));
    cmap->r_breaks.append(QPointF(0.290, 0.126));
    cmap->r_breaks.append(QPointF(0.300, 0.143));
    cmap->r_breaks.append(QPointF(0.310, 0.164));
    cmap->r_breaks.append(QPointF(0.320, 0.188));
    cmap->r_breaks.append(QPointF(0.330, 0.214));
    cmap->r_breaks.append(QPointF(0.340, 0.244));
    cmap->r_breaks.append(QPointF(0.350, 0.275));
    cmap->r_breaks.append(QPointF(0.360, 0.309));
    cmap->r_breaks.append(QPointF(0.370, 0.345));
    cmap->r_breaks.append(QPointF(0.380, 0.383));
    cmap->r_breaks.append(QPointF(0.390, 0.422));
    cmap->r_breaks.append(QPointF(0.400, 0.462));
    cmap->r_breaks.append(QPointF(0.410, 0.503));
    cmap->r_breaks.append(QPointF(0.420, 0.544));
    cmap->r_breaks.append(QPointF(0.430, 0.585));
    cmap->r_breaks.append(QPointF(0.440, 0.625));
    cmap->r_breaks.append(QPointF(0.450, 0.665));
    cmap->r_breaks.append(QPointF(0.460, 0.704));
    cmap->r_breaks.append(QPointF(0.470, 0.741));
    cmap->r_breaks.append(QPointF(0.480, 0.777));
    cmap->r_breaks.append(QPointF(0.490, 0.811));
    cmap->r_breaks.append(QPointF(0.500, 0.843));
    cmap->r_breaks.append(QPointF(0.510, 0.872));
    cmap->r_breaks.append(QPointF(0.520, 0.899));
    cmap->r_breaks.append(QPointF(0.530, 0.924));
    cmap->r_breaks.append(QPointF(0.540, 0.945));
    cmap->r_breaks.append(QPointF(0.550, 0.964));
    cmap->r_breaks.append(QPointF(0.560, 0.980));
    cmap->r_breaks.append(QPointF(0.570, 0.993));
    cmap->r_breaks.append(QPointF(0.580, 1.000));
    cmap->r_breaks.append(QPointF(0.590, 1.000));
    cmap->r_breaks.append(QPointF(0.600, 1.000));
    cmap->r_breaks.append(QPointF(0.610, 1.000));
    cmap->r_breaks.append(QPointF(0.620, 1.000));
    cmap->r_breaks.append(QPointF(0.630, 1.000));
    cmap->r_breaks.append(QPointF(0.640, 1.000));
    cmap->r_breaks.append(QPointF(0.650, 0.999));
    cmap->r_breaks.append(QPointF(0.660, 0.990));
    cmap->r_breaks.append(QPointF(0.670, 0.979));
    cmap->r_breaks.append(QPointF(0.680, 0.968));
    cmap->r_breaks.append(QPointF(0.690, 0.955));
    cmap->r_breaks.append(QPointF(0.700, 0.942));
    cmap->r_breaks.append(QPointF(0.710, 0.928));
    cmap->r_breaks.append(QPointF(0.720, 0.915));
    cmap->r_breaks.append(QPointF(0.730, 0.901));
    cmap->r_breaks.append(QPointF(0.740, 0.888));
    cmap->r_breaks.append(QPointF(0.750, 0.875));
    cmap->r_breaks.append(QPointF(0.760, 0.864));
    cmap->r_breaks.append(QPointF(0.770, 0.853));
    cmap->r_breaks.append(QPointF(0.780, 0.844));
    cmap->r_breaks.append(QPointF(0.790, 0.836));
    cmap->r_breaks.append(QPointF(0.800, 0.829));
    cmap->r_breaks.append(QPointF(0.810, 0.825));
    cmap->r_breaks.append(QPointF(0.820, 0.822));
    cmap->r_breaks.append(QPointF(0.830, 0.821));
    cmap->r_breaks.append(QPointF(0.840, 0.821));
    cmap->r_breaks.append(QPointF(0.850, 0.824));
    cmap->r_breaks.append(QPointF(0.860, 0.828));
    cmap->r_breaks.append(QPointF(0.870, 0.834));
    cmap->r_breaks.append(QPointF(0.880, 0.841));
    cmap->r_breaks.append(QPointF(0.890, 0.851));
    cmap->r_breaks.append(QPointF(0.900, 0.861));
    cmap->r_breaks.append(QPointF(0.910, 0.873));
    cmap->r_breaks.append(QPointF(0.920, 0.885));
    cmap->r_breaks.append(QPointF(0.930, 0.899));
    cmap->r_breaks.append(QPointF(0.940, 0.913));
    cmap->r_breaks.append(QPointF(0.950, 0.928));
    cmap->r_breaks.append(QPointF(0.960, 0.943));
    cmap->r_breaks.append(QPointF(0.970, 0.958));
    cmap->r_breaks.append(QPointF(0.980, 0.972));
    cmap->r_breaks.append(QPointF(0.990, 0.987));
    cmap->r_breaks.append(QPointF(1.000, 1.000));

    cmap->g_breaks.append(QPointF(0.000, 0.000));
//    cmap->g_breaks.append(QPointF(0.010, 0.020));
//    cmap->g_breaks.append(QPointF(0.020, 0.035));
//    cmap->g_breaks.append(QPointF(0.030, 0.050));
//    cmap->g_breaks.append(QPointF(0.040, 0.066));
//    cmap->g_breaks.append(QPointF(0.050, 0.084));
//    cmap->g_breaks.append(QPointF(0.060, 0.103));
//    cmap->g_breaks.append(QPointF(0.070, 0.123));
//    cmap->g_breaks.append(QPointF(0.080, 0.145));
//    cmap->g_breaks.append(QPointF(0.090, 0.169));
//    cmap->g_breaks.append(QPointF(0.100, 0.193));
//    cmap->g_breaks.append(QPointF(0.110, 0.219));
    cmap->g_breaks.append(QPointF(0.120, 0.245));
    cmap->g_breaks.append(QPointF(0.130, 0.273));
    cmap->g_breaks.append(QPointF(0.140, 0.301));
    cmap->g_breaks.append(QPointF(0.150, 0.329));
    cmap->g_breaks.append(QPointF(0.160, 0.358));
    cmap->g_breaks.append(QPointF(0.170, 0.387));
    cmap->g_breaks.append(QPointF(0.180, 0.415));
    cmap->g_breaks.append(QPointF(0.190, 0.443));
    cmap->g_breaks.append(QPointF(0.200, 0.471));
    cmap->g_breaks.append(QPointF(0.210, 0.498));
    cmap->g_breaks.append(QPointF(0.220, 0.523));
    cmap->g_breaks.append(QPointF(0.230, 0.548));
    cmap->g_breaks.append(QPointF(0.240, 0.571));
    cmap->g_breaks.append(QPointF(0.250, 0.593));
    cmap->g_breaks.append(QPointF(0.260, 0.613));
    cmap->g_breaks.append(QPointF(0.270, 0.631));
    cmap->g_breaks.append(QPointF(0.280, 0.648));
    cmap->g_breaks.append(QPointF(0.290, 0.662));
    cmap->g_breaks.append(QPointF(0.300, 0.675));
    cmap->g_breaks.append(QPointF(0.310, 0.686));
    cmap->g_breaks.append(QPointF(0.320, 0.695));
    cmap->g_breaks.append(QPointF(0.330, 0.701));
    cmap->g_breaks.append(QPointF(0.340, 0.706));
    cmap->g_breaks.append(QPointF(0.350, 0.710));
    cmap->g_breaks.append(QPointF(0.360, 0.711));
    cmap->g_breaks.append(QPointF(0.370, 0.711));
    cmap->g_breaks.append(QPointF(0.380, 0.709));
    cmap->g_breaks.append(QPointF(0.390, 0.706));
    cmap->g_breaks.append(QPointF(0.400, 0.701));
    cmap->g_breaks.append(QPointF(0.410, 0.696));
    cmap->g_breaks.append(QPointF(0.420, 0.689));
    cmap->g_breaks.append(QPointF(0.430, 0.682));
    cmap->g_breaks.append(QPointF(0.440, 0.674));
    cmap->g_breaks.append(QPointF(0.450, 0.665));
    cmap->g_breaks.append(QPointF(0.460, 0.657));
    cmap->g_breaks.append(QPointF(0.470, 0.648));
    cmap->g_breaks.append(QPointF(0.480, 0.640));
    cmap->g_breaks.append(QPointF(0.490, 0.632));
    cmap->g_breaks.append(QPointF(0.500, 0.624));
    cmap->g_breaks.append(QPointF(0.510, 0.617));
    cmap->g_breaks.append(QPointF(0.520, 0.610));
    cmap->g_breaks.append(QPointF(0.530, 0.605));
    cmap->g_breaks.append(QPointF(0.540, 0.601));
    cmap->g_breaks.append(QPointF(0.550, 0.597));
    cmap->g_breaks.append(QPointF(0.560, 0.595));
    cmap->g_breaks.append(QPointF(0.570, 0.595));
    cmap->g_breaks.append(QPointF(0.580, 0.595));
    cmap->g_breaks.append(QPointF(0.590, 0.597));
    cmap->g_breaks.append(QPointF(0.600, 0.601));
    cmap->g_breaks.append(QPointF(0.610, 0.605));
    cmap->g_breaks.append(QPointF(0.620, 0.612));
    cmap->g_breaks.append(QPointF(0.630, 0.619));
    cmap->g_breaks.append(QPointF(0.640, 0.628));
    cmap->g_breaks.append(QPointF(0.650, 0.638));
    cmap->g_breaks.append(QPointF(0.660, 0.650));
    cmap->g_breaks.append(QPointF(0.670, 0.662));
    cmap->g_breaks.append(QPointF(0.680, 0.676));
    cmap->g_breaks.append(QPointF(0.690, 0.690));
    cmap->g_breaks.append(QPointF(0.700, 0.705));
    cmap->g_breaks.append(QPointF(0.710, 0.720));
    cmap->g_breaks.append(QPointF(0.720, 0.737));
    cmap->g_breaks.append(QPointF(0.730, 0.753));
    cmap->g_breaks.append(QPointF(0.740, 0.770));
    cmap->g_breaks.append(QPointF(0.750, 0.786));
    cmap->g_breaks.append(QPointF(0.760, 0.803));
    cmap->g_breaks.append(QPointF(0.770, 0.819));
    cmap->g_breaks.append(QPointF(0.780, 0.835));
    cmap->g_breaks.append(QPointF(0.790, 0.851));
    cmap->g_breaks.append(QPointF(0.800, 0.866));
    cmap->g_breaks.append(QPointF(0.810, 0.881));
    cmap->g_breaks.append(QPointF(0.820, 0.894));
    cmap->g_breaks.append(QPointF(0.830, 0.907));
    cmap->g_breaks.append(QPointF(0.840, 0.919));
    cmap->g_breaks.append(QPointF(0.850, 0.930));
    cmap->g_breaks.append(QPointF(0.860, 0.940));
    cmap->g_breaks.append(QPointF(0.870, 0.950));
    cmap->g_breaks.append(QPointF(0.880, 0.958));
    cmap->g_breaks.append(QPointF(0.890, 0.965));
    cmap->g_breaks.append(QPointF(0.900, 0.971));
    cmap->g_breaks.append(QPointF(0.910, 0.977));
    cmap->g_breaks.append(QPointF(0.920, 0.981));
    cmap->g_breaks.append(QPointF(0.930, 0.985));
    cmap->g_breaks.append(QPointF(0.940, 0.989));
    cmap->g_breaks.append(QPointF(0.950, 0.991));
    cmap->g_breaks.append(QPointF(0.960, 0.994));
    cmap->g_breaks.append(QPointF(0.970, 0.995));
    cmap->g_breaks.append(QPointF(0.980, 0.997));
    cmap->g_breaks.append(QPointF(0.990, 0.999));
    cmap->g_breaks.append(QPointF(1.000, 1.000));

    cmap->b_breaks.append(QPointF(0.000, 0.000));
//    cmap->b_breaks.append(QPointF(0.010, 0.117));
//    cmap->b_breaks.append(QPointF(0.020, 0.185));
//    cmap->b_breaks.append(QPointF(0.030, 0.244));
//    cmap->b_breaks.append(QPointF(0.040, 0.298));
//    cmap->b_breaks.append(QPointF(0.050, 0.349));
//    cmap->b_breaks.append(QPointF(0.060, 0.396));
//    cmap->b_breaks.append(QPointF(0.070, 0.439));
//    cmap->b_breaks.append(QPointF(0.080, 0.479));
//    cmap->b_breaks.append(QPointF(0.090, 0.515));
//    cmap->b_breaks.append(QPointF(0.100, 0.546));
//    cmap->b_breaks.append(QPointF(0.110, 0.574));
    cmap->b_breaks.append(QPointF(0.120, 0.597));
    cmap->b_breaks.append(QPointF(0.130, 0.616));
    cmap->b_breaks.append(QPointF(0.140, 0.631));
    cmap->b_breaks.append(QPointF(0.150, 0.641));
    cmap->b_breaks.append(QPointF(0.160, 0.647));
    cmap->b_breaks.append(QPointF(0.170, 0.649));
    cmap->b_breaks.append(QPointF(0.180, 0.646));
    cmap->b_breaks.append(QPointF(0.190, 0.640));
    cmap->b_breaks.append(QPointF(0.200, 0.630));
    cmap->b_breaks.append(QPointF(0.210, 0.616));
    cmap->b_breaks.append(QPointF(0.220, 0.600));
    cmap->b_breaks.append(QPointF(0.230, 0.581));
    cmap->b_breaks.append(QPointF(0.240, 0.559));
    cmap->b_breaks.append(QPointF(0.250, 0.536));
    cmap->b_breaks.append(QPointF(0.260, 0.511));
    cmap->b_breaks.append(QPointF(0.270, 0.485));
    cmap->b_breaks.append(QPointF(0.280, 0.458));
    cmap->b_breaks.append(QPointF(0.290, 0.431));
    cmap->b_breaks.append(QPointF(0.300, 0.404));
    cmap->b_breaks.append(QPointF(0.310, 0.377));
    cmap->b_breaks.append(QPointF(0.320, 0.352));
    cmap->b_breaks.append(QPointF(0.330, 0.328));
    cmap->b_breaks.append(QPointF(0.340, 0.306));
    cmap->b_breaks.append(QPointF(0.350, 0.285));
    cmap->b_breaks.append(QPointF(0.360, 0.268));
    cmap->b_breaks.append(QPointF(0.370, 0.252));
    cmap->b_breaks.append(QPointF(0.380, 0.240));
    cmap->b_breaks.append(QPointF(0.390, 0.231));
    cmap->b_breaks.append(QPointF(0.400, 0.225));
    cmap->b_breaks.append(QPointF(0.410, 0.223));
    cmap->b_breaks.append(QPointF(0.420, 0.224));
    cmap->b_breaks.append(QPointF(0.430, 0.228));
    cmap->b_breaks.append(QPointF(0.440, 0.236));
    cmap->b_breaks.append(QPointF(0.450, 0.248));
    cmap->b_breaks.append(QPointF(0.460, 0.263));
    cmap->b_breaks.append(QPointF(0.470, 0.281));
    cmap->b_breaks.append(QPointF(0.480, 0.302));
    cmap->b_breaks.append(QPointF(0.490, 0.326));
    cmap->b_breaks.append(QPointF(0.500, 0.353));
    cmap->b_breaks.append(QPointF(0.510, 0.382));
    cmap->b_breaks.append(QPointF(0.520, 0.414));
    cmap->b_breaks.append(QPointF(0.530, 0.447));
    cmap->b_breaks.append(QPointF(0.540, 0.482));
    cmap->b_breaks.append(QPointF(0.550, 0.518));
    cmap->b_breaks.append(QPointF(0.560, 0.554));
    cmap->b_breaks.append(QPointF(0.570, 0.592));
    cmap->b_breaks.append(QPointF(0.580, 0.629));
    cmap->b_breaks.append(QPointF(0.590, 0.667));
    cmap->b_breaks.append(QPointF(0.600, 0.704));
    cmap->b_breaks.append(QPointF(0.610, 0.740));
    cmap->b_breaks.append(QPointF(0.620, 0.775));
    cmap->b_breaks.append(QPointF(0.630, 0.809));
    cmap->b_breaks.append(QPointF(0.640, 0.841));
    cmap->b_breaks.append(QPointF(0.650, 0.872));
    cmap->b_breaks.append(QPointF(0.660, 0.900));
    cmap->b_breaks.append(QPointF(0.670, 0.926));
    cmap->b_breaks.append(QPointF(0.680, 0.950));
    cmap->b_breaks.append(QPointF(0.690, 0.971));
    cmap->b_breaks.append(QPointF(0.700, 0.990));
    cmap->b_breaks.append(QPointF(0.710, 1.000));
    cmap->b_breaks.append(QPointF(0.720, 1.000));
    cmap->b_breaks.append(QPointF(0.730, 1.000));
    cmap->b_breaks.append(QPointF(0.740, 1.000));
    cmap->b_breaks.append(QPointF(0.750, 1.000));
    cmap->b_breaks.append(QPointF(0.760, 1.000));
    cmap->b_breaks.append(QPointF(0.770, 1.000));
    cmap->b_breaks.append(QPointF(0.780, 1.000));
    cmap->b_breaks.append(QPointF(0.790, 1.000));
    cmap->b_breaks.append(QPointF(0.800, 1.000));
    cmap->b_breaks.append(QPointF(0.810, 1.000));
    cmap->b_breaks.append(QPointF(0.820, 1.000));
    cmap->b_breaks.append(QPointF(0.830, 1.000));
    cmap->b_breaks.append(QPointF(0.840, 1.000));
    cmap->b_breaks.append(QPointF(0.850, 1.000));
    cmap->b_breaks.append(QPointF(0.860, 1.000));
    cmap->b_breaks.append(QPointF(0.870, 0.995));
    cmap->b_breaks.append(QPointF(0.880, 0.988));
    cmap->b_breaks.append(QPointF(0.890, 0.981));
    cmap->b_breaks.append(QPointF(0.900, 0.976));
    cmap->b_breaks.append(QPointF(0.910, 0.971));
    cmap->b_breaks.append(QPointF(0.920, 0.968));
    cmap->b_breaks.append(QPointF(0.930, 0.966));
    cmap->b_breaks.append(QPointF(0.940, 0.966));
    cmap->b_breaks.append(QPointF(0.950, 0.967));
    cmap->b_breaks.append(QPointF(0.960, 0.970));
    cmap->b_breaks.append(QPointF(0.970, 0.975));
    cmap->b_breaks.append(QPointF(0.980, 0.982));
    cmap->b_breaks.append(QPointF(0.990, 0.990));
    cmap->b_breaks.append(QPointF(1.000, 1.000));

    return cmap;
}


QColor ColorMap::generateChannelColor(int channelID, QColor baseColor)
{
    if(gray)
    {
        QColor color;

        switch(channelID)
        {
        case 1: color.setHsv(0, 255 - baseColor.green(), 190); break;
        case 2: color.setHsv(120, 255 - baseColor.green(), 190); break;
        case 3: color.setHsv(240, 255 - baseColor.green(), 190); break;
        case 4: color.setHsv(60, 255 - baseColor.green(), 190); break;
        default: color = baseColor; break;
        }

        return color;
    }
    else
        return baseColor;
}


QColor ColorMap::generateChannelColor(int channelID, int channelCount, QColor baseColor)
{
    if(currentStyle == STYLE::CUBEHELIX)
    {
        //qDebug() << baseColor.redF() << baseColor.greenF() << baseColor.blueF() << (baseColor.redF() + baseColor.greenF() + baseColor.blueF());

        if((baseColor.redF() + baseColor.greenF() + baseColor.blueF()) > 2.0)
            return baseColor.darker(100 + float(channelID-1) * 85 / float(channelCount));
        else if((baseColor.redF() + baseColor.greenF() + baseColor.blueF()) > 1.0)
            return baseColor.lighter(70 + float(channelID-1) * 85 / float(channelCount));
        else if((baseColor.redF() + baseColor.greenF() + baseColor.blueF()) > 0.01)
            return baseColor.lighter(70 + float(channelID-1) * 150 / float(channelCount));
        else
        {
            QColor temp;
            baseColor.toHsv();
            temp.setHsv(baseColor.hsvHue(), baseColor.hsvSaturation(), 70 + float(channelID-1) * 150 / float(channelCount));
            return temp;
        }
    }
    else if(currentStyle == STYLE::GRAY)
    {
        QColor color;

        switch(channelID)
        {
            case 1: color.setHsv(0, 255 - baseColor.green(), 190); break;
            case 2: color.setHsv(120, 255 - baseColor.green(), 190); break;
            case 3: color.setHsv(240, 255 - baseColor.green(), 190); break;
            case 4: color.setHsv(60, 255 - baseColor.green(), 190); break;
            default: color = baseColor; break;
        }

        return color;
    }
    else
    {
        // vary color for different channel ids
        float range = 85.0f; // higher values => darker color for higher channel-ids
        return baseColor.darker(100 + float(channelID-1) * range/float(channelCount));
    }
}


ColorMap::STYLE ColorMap::getCurrentStyle()
{
    return currentStyle;
}

