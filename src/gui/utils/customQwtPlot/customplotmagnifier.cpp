#include "customplotmagnifier.h"

#include <qwt_plot.h>
#include <qwt_scale_div.h>

/**
 * @brief CustomPlotMagnifier::setLogRescaling enables or disables
 * logarithmic rescaling for given axisId (only QwtPlot::yLeft and
 * QwtPlot::xBottom are implemented!)
 * @param state new logaritmich rescaling state
 * @param axisId axis id
 */
void CustomPlotMagnifier::setLogRescaling(bool state, int axisId)
{
    if(axisId == QwtPlot::xBottom)
        m_logRescale_x = state;
    if(axisId == QwtPlot::yLeft)
        m_logRescale_y = state;
}


/**
 * @brief CustomPlotMagnifier::rescale overrides QwtPlotMagnifier::rescale(double factor).
 * If logarithmic rescaling is enabled, the new range
 * will be scaled linearly in the logarithmic domain by factor.
 * @param factor
 */
void CustomPlotMagnifier::rescale(double factor)
{
    QwtPlot* plt = plot();
    if ( plt == NULL )
        return;

    factor = qAbs( factor );
    if ( factor == 1.0 || factor == 0.0 )
        return;

    bool doReplot = false;

    const bool autoReplot = plt->autoReplot();
    plt->setAutoReplot( false );

    for ( int axisId = 0; axisId < QwtPlot::axisCnt; axisId++ )
    {

        const QwtScaleDiv &scaleDiv = plt->axisScaleDiv( axisId );
        if ( isAxisEnabled( axisId ) )
        {
            // check if log-rescaling should be done for x/y axis
            if( (axisId == QwtPlot::xBottom && m_logRescale_x) ||
                (axisId == QwtPlot::yLeft && m_logRescale_y) )
            {
                //get bounds
                double low = scaleDiv.lowerBound();
                double up = scaleDiv.upperBound();

                //find logarithmic center
                double log_width_2 = (log10(up)-log10(low))/2.0;
                double log_cen = log10(low) + log_width_2;

                //calculate new bounds by linear scaling in log domain.
                double new_low = pow(10, log_cen - log_width_2 * factor );
                double new_up = pow(10, log_cen + log_width_2 * factor );

                plt->setAxisScale( axisId, new_low, new_up );
                doReplot = true;
            }
            else // linear rescaling as suggested in QwtPlotMagnifier::rescale()
            {
                const double center =  scaleDiv.lowerBound() + scaleDiv.range() / 2;
                const double width_2 = scaleDiv.range() / 2 * factor;

                plt->setAxisScale( axisId, center - width_2, center + width_2 );
                doReplot = true;
            }
        }
    }

    plt->setAutoReplot( autoReplot );

    if ( doReplot )
        plt->replot();
}
