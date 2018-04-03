#ifndef CUSTOMLOGSCALEENGINE_H
#define CUSTOMLOGSCALEENGINE_H

#include <qwt_scale_engine.h>
#include <qwt_global.h>

/**
 * @brief The CustomLogScaleEngine class provides a custom logarithmic
 * axis scaling. Difference to QwtLogScaleEngine: Uses CustomLogTransformation,
 * Values < 0 are set to a minimum value before
 * log transformation (=> no NaN/inf values in plots!)
 * @ingroup GUI-Utilities
 */
class CustomLogScaleEngine: public QwtScaleEngine
{
public:
    CustomLogScaleEngine( uint base = 10 );
    virtual ~CustomLogScaleEngine();

    virtual void autoScale( int maxSteps,
        double &x1, double &x2, double &stepSize ) const;

    virtual QwtScaleDiv divideScale( double x1, double x2,
        int numMajorSteps, int numMinorSteps,
        double stepSize = 0.0 ) const;


    static inline double qwtLog( double base, double value )
    {
        return log( value ) / log( base );
    }

    static inline QwtInterval qwtLogInterval( double base, const QwtInterval &interval )
    {
        return QwtInterval( qwtLog( base, interval.minValue() ),
                qwtLog( base, interval.maxValue() ) );
    }

    static inline QwtInterval qwtPowInterval( double base, const QwtInterval &interval )
    {
        return QwtInterval( pow( base, interval.minValue() ),
                pow( base, interval.maxValue() ) );
    }


protected:
    QwtInterval align( const QwtInterval&, double stepSize ) const;

    void buildTicks(
        const QwtInterval &, double stepSize, int maxMinSteps,
        QList<double> ticks[QwtScaleDiv::NTickTypes] ) const;

    QList<double> buildMajorTicks(
        const QwtInterval &interval, double stepSize ) const;

    void buildMinorTicks( const QList<double>& majorTicks,
        int maxMinorSteps, double stepSize,
        QList<double> &minorTicks, QList<double> &mediumTicks ) const;
};

#endif // CUSTOMLOGSCALEENGINE_H
