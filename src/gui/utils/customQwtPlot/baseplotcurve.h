#ifndef BASEPLOTCURVE_H
#define BASEPLOTCURVE_H

#include <QObject>
#include <qwt_plot_curve.h>


/**
 * @brief The BasePlotCurve class: custom QwtPlotCurve class
 */
class BasePlotCurve : public QwtPlotCurve
{
    public:
        BasePlotCurve(const QString & title);
        ~BasePlotCurve();

        void setFixedStyle(bool state);
        bool isFixedStyle();

    protected:
        bool m_fixedStyle;


};

#endif // BASEPLOTCURVE_H
