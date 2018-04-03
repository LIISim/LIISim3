#ifndef CUSTOMPLOTMAGNIFIER_H
#define CUSTOMPLOTMAGNIFIER_H

#include <qwt_plot_magnifier.h>

/**
 * @brief The CustomPlotMagnifier class extends the functionality of
 * the QwtPlotMagnifier class. It provides correct rescaling
 * for logarithmic axis scales.
 * @ingroup GUI-Utilities
 */
class CustomPlotMagnifier : public QwtPlotMagnifier
{
  //  Q_OBJECT

    bool m_logRescale_x;
    bool m_logRescale_y;
public:
    explicit CustomPlotMagnifier(QWidget *parent = 0)
        : QwtPlotMagnifier(parent),
          m_logRescale_x(false),
          m_logRescale_y(false){}

    void setLogRescaling(bool state,int axisId);

protected:
    virtual void rescale( double factor );

};


#endif // CUSTOMPLOTMAGNIFIER_H
