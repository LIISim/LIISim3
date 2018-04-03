#include "baseplotcurve.h"


#include <qwt_scale_map.h>

BasePlotCurve::BasePlotCurve(
        const QString &title)  : QwtPlotCurve(title)
{
    m_fixedStyle = false;
}


BasePlotCurve::~BasePlotCurve() {}


/**
 * @brief BasePlotCurve::setFixedStyle if fixed style is true,
 * appearance of this curve is never changed (f.e. by context menu-> change style)
 * @param state
 */
void BasePlotCurve::setFixedStyle(bool state)
{
    m_fixedStyle = state;
}


bool BasePlotCurve::isFixedStyle()
{
    return  m_fixedStyle;
}
