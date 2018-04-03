#include "customlogscaleengine.h"

#include <qwt_scale_div.h>
#include <qwt_interval.h>
#include <qwt_math.h>

#include "customlogtransform.h"

/**
 * @brief CustomLogScaleEngine::CustomLogScaleEngine Constructor
 * @param base Base of Logarithm
 */
CustomLogScaleEngine::CustomLogScaleEngine(uint base ): QwtScaleEngine( base )
{
    setTransformation( new CustomLogTransform() );
}


/**
 * @brief CustomLogScaleEngine::~CustomLogScaleEngine Destructor
 */
CustomLogScaleEngine::~CustomLogScaleEngine()
{
}


/**
 * @brief CustomLogScaleEngine::autoScale Align and divide an interval
 * @param maxNumSteps Max. number of steps
 * @param x1 First limit of the interval (In/Out)
 * @param x2 Second limit of the interval (In/Out)
 * @param stepSize Step size (Out)
 */
void CustomLogScaleEngine::autoScale(int maxNumSteps,
    double &x1, double &x2, double &stepSize ) const
{
    // reset negative values
    if(x1 <= 0.0)
        x1 = 1e-15;
    if(x2 <= 0.0)
        x2 = 1e-15;

    if ( x1 > x2 )
        qSwap( x1, x2 );

    const double logBase = base();

    QwtInterval interval( x1 / pow( logBase, lowerMargin() ),
        x2 * pow( logBase, upperMargin() ) );

    if ( interval.maxValue() / interval.minValue() < logBase )
    {
        // scale width is less than one step -> try to build a linear scale

        QwtLinearScaleEngine linearScaler;
        linearScaler.setAttributes( attributes() );
        linearScaler.setReference( reference() );
        linearScaler.setMargins( lowerMargin(), upperMargin() );

        linearScaler.autoScale( maxNumSteps, x1, x2, stepSize );

        QwtInterval linearInterval = QwtInterval( x1, x2 ).normalized();
        linearInterval = linearInterval.limited( LOG_MIN, LOG_MAX );

        if ( linearInterval.maxValue() / linearInterval.minValue() < logBase )
        {
            // the aligned scale is still less than one step
            if ( stepSize < 0.0 )
                stepSize = -qwtLog( logBase, qAbs( stepSize ) );
            else
                stepSize = qwtLog( logBase, stepSize );

            return;
        }
    }

    double logRef = 1.0;
    if ( reference() > LOG_MIN / 2 )
        logRef = qMin( reference(), LOG_MAX / 2 );

    if ( testAttribute( QwtScaleEngine::Symmetric ) )
    {
        const double delta = qMax( interval.maxValue() / logRef,
            logRef / interval.minValue() );
        interval.setInterval( logRef / delta, logRef * delta );
    }

    if ( testAttribute( QwtScaleEngine::IncludeReference ) )
        interval = interval.extend( logRef );

    interval = interval.limited( LOG_MIN, LOG_MAX );

    if ( interval.width() == 0.0 )
        interval = buildInterval( interval.minValue() );

    stepSize = divideInterval( qwtLogInterval( logBase, interval ).width(),
        qMax( maxNumSteps, 1 ) );
    if ( stepSize < 1.0 )
        stepSize = 1.0;

    if ( !testAttribute( QwtScaleEngine::Floating ) )
        interval = align( interval, stepSize );

    x1 = interval.minValue();
    x2 = interval.maxValue();

    if ( testAttribute( QwtScaleEngine::Inverted ) )
    {
        qSwap( x1, x2 );
        stepSize = -stepSize;
    }
}


/**
 * @brief CustomLogScaleEngine::divideScale
 * Calculate a scale division for an interval
 * @param x1 First interval limit
 * @param x2 Second interval limit
 * @param maxMajorSteps Maximum for the number of major steps
 * @param maxMinorSteps Maximum number of minor steps
 * @param stepSize Step size. If stepSize == 0, the engine calculates one.
 * @return Calculated scale division
 */
QwtScaleDiv CustomLogScaleEngine::divideScale(double x1, double x2,
    int maxMajorSteps, int maxMinorSteps, double stepSize ) const
{
    //reset negative values
    if(x1 <= 0.0)
        x1 = 1e-15;
    if(x2 <= 0.0)
        x2 = 1e-15;

    QwtInterval interval = QwtInterval( x1, x2 ).normalized();
    interval = interval.limited( LOG_MIN, LOG_MAX );

    if ( interval.width() <= 0 )
        return QwtScaleDiv();

    const double logBase = base();

    if ( interval.maxValue() / interval.minValue() < logBase )
    {
        // scale width is less than one decade -> build linear scale

        QwtLinearScaleEngine linearScaler;
        linearScaler.setAttributes( attributes() );
        linearScaler.setReference( reference() );
        linearScaler.setMargins( lowerMargin(), upperMargin() );

        if ( stepSize != 0.0 )
        {
            if ( stepSize < 0.0 )
                stepSize = -qPow( logBase, -stepSize );
            else
                stepSize = qPow( logBase, stepSize );
        }

        return linearScaler.divideScale( x1, x2,
            maxMajorSteps, maxMinorSteps, stepSize );
    }

    stepSize = qAbs( stepSize );
    if ( stepSize == 0.0 )
    {
        if ( maxMajorSteps < 1 )
            maxMajorSteps = 1;

        stepSize = divideInterval(
            qwtLogInterval( logBase, interval ).width(), maxMajorSteps );
        if ( stepSize < 1.0 )
            stepSize = 1.0; // major step must be >= 1 decade
    }

    QwtScaleDiv scaleDiv;
    if ( stepSize != 0.0 )
    {
        QList<double> ticks[QwtScaleDiv::NTickTypes];
        buildTicks( interval, stepSize, maxMinorSteps, ticks );

        scaleDiv = QwtScaleDiv( interval, ticks );
    }

    if ( x1 > x2 )
        scaleDiv.invert();

    return scaleDiv;
}


/**
 * @brief CustomLogScaleEngine::buildTicks
 * Calculate ticks for an interval
 * @param interval Interval
 * @param stepSize Step size
 * @param maxMinorSteps Maximum number of minor steps
 * @param ticks Arrays to be filled with the calculated ticks
 */
void CustomLogScaleEngine::buildTicks(const QwtInterval& interval,
    double stepSize, int maxMinorSteps,
    QList<double> ticks[QwtScaleDiv::NTickTypes] ) const
{
    const QwtInterval boundingInterval = align( interval, stepSize );

    ticks[QwtScaleDiv::MajorTick] =
        buildMajorTicks( boundingInterval, stepSize );

    if ( maxMinorSteps > 0 )
    {
        buildMinorTicks( ticks[QwtScaleDiv::MajorTick], maxMinorSteps, stepSize,
            ticks[QwtScaleDiv::MinorTick], ticks[QwtScaleDiv::MediumTick] );
    }

    for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
        ticks[i] = strip( ticks[i], interval );
}


/**
 * @brief CustomLogScaleEngine::buildMajorTicks
 * Calculate major ticks for an interval
 * @param interval Interval
 * @param stepSize Step size
 * @return Calculated ticks
 */
QList<double> CustomLogScaleEngine::buildMajorTicks(const QwtInterval &interval,
    double stepSize ) const
{
    double width = qwtLogInterval( base(), interval ).width();

    int numTicks = qRound( width / stepSize ) + 1;
    if ( numTicks > 10000 )
        numTicks = 10000;

    const double lxmin = ::log( interval.minValue() );
    const double lxmax = ::log( interval.maxValue() );
    const double lstep = ( lxmax - lxmin ) / double( numTicks - 1 );

    QList<double> ticks;

    ticks += interval.minValue();

    for ( int i = 1; i < numTicks - 1; i++ )
        ticks += qExp( lxmin + double( i ) * lstep );

    ticks += interval.maxValue();

    return ticks;
}


/**
 * @brief CustomLogScaleEngine::buildMinorTicks minor/medium ticks for major ticks
 * @param majorTicks Major ticks
 * @param maxMinorSteps Maximum number of minor steps
 * @param stepSize Step size
 * @param minorTicks Array to be filled with the calculated minor ticks
 * @param mediumTicks Array to be filled with the calculated medium ticks
 */
void CustomLogScaleEngine::buildMinorTicks(
    const QList<double> &majorTicks,
    int maxMinorSteps, double stepSize,
    QList<double> &minorTicks,
    QList<double> &mediumTicks ) const
{
    const double logBase = base();

    if ( stepSize < 1.1 )          // major step width is one base
    {
        double minStep = divideInterval( stepSize, maxMinorSteps + 1 );
        if ( minStep == 0.0 )
            return;

        const int numSteps = qRound( stepSize / minStep );

        int mediumTickIndex = -1;
        if ( ( numSteps > 2 ) && ( numSteps % 2 == 0 ) )
            mediumTickIndex = numSteps / 2;

        for ( int i = 0; i < majorTicks.count() - 1; i++ )
        {
            const double v = majorTicks[i];
            const double s = logBase / numSteps;

            if ( s >= 1.0 )
            {
                for ( int j = 2; j < numSteps; j++ )
                {
                    minorTicks += v * j * s;
                }
            }
            else
            {
                for ( int j = 1; j < numSteps; j++ )
                {
                    const double tick = v + j * v * ( logBase - 1 ) / numSteps;
                    if ( j == mediumTickIndex )
                        mediumTicks += tick;
                    else
                        minorTicks += tick;
                }
            }
        }
    }
    else
    {
        double minStep = divideInterval( stepSize, maxMinorSteps );
        if ( minStep == 0.0 )
            return;

        if ( minStep < 1.0 )
            minStep = 1.0;

        // # subticks per interval
        int numTicks = qRound( stepSize / minStep ) - 1;

        // Do the minor steps fit into the interval?
        if ( qwtFuzzyCompare( ( numTicks +  1 ) * minStep,
            stepSize, stepSize ) > 0 )
        {
            numTicks = 0;
        }

        if ( numTicks < 1 )
            return;

        int mediumTickIndex = -1;
        if ( ( numTicks > 2 ) && ( numTicks % 2 ) )
            mediumTickIndex = numTicks / 2;

        // substep factor = base^substeps
        const qreal minFactor = qMax( qPow( logBase, minStep ), qreal( logBase ) );

        for ( int i = 0; i < majorTicks.count(); i++ )
        {
            double tick = majorTicks[i];
            for ( int j = 0; j < numTicks; j++ )
            {
                tick *= minFactor;

                if ( j == mediumTickIndex )
                    mediumTicks += tick;
                else
                    minorTicks += tick;
            }
        }
    }
}


/**
 * @brief CustomLogScaleEngine::align Align an interval to a step size
 * The limits of an interval are aligned that both are integer
 * multiples of the step size.
 * @param interval Interval
 * @param stepSize Step size
 * @return Aligned interval
 */
QwtInterval CustomLogScaleEngine::align(
    const QwtInterval &interval, double stepSize ) const
{
    const QwtInterval intv = qwtLogInterval( base(), interval );

    double x1 = QwtScaleArithmetic::floorEps( intv.minValue(), stepSize );
    if ( qwtFuzzyCompare( interval.minValue(), x1, stepSize ) == 0 )
        x1 = interval.minValue();

    double x2 = QwtScaleArithmetic::ceilEps( intv.maxValue(), stepSize );
    if ( qwtFuzzyCompare( interval.maxValue(), x2, stepSize ) == 0 )
        x2 = interval.maxValue();

    return qwtPowInterval( base(), QwtInterval( x1, x2 ) );
}

