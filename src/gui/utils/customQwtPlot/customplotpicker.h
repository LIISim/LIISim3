#ifndef CUSTOMPLOTPICKER_H
#define CUSTOMPLOTPICKER_H

#include <qwt_plot_picker.h>
#include "../../../signal/signal.h"

/**
 * @brief The CustomPlotPicker class extends QwtPlotPicker and
 * provides a custom trackerText() method.
 * @ingroup GUI-Utilities
 */
class CustomPlotPicker : public QwtPlotPicker
{
    public:
        explicit CustomPlotPicker(int xAxis,
                int yAxis, RubberBand rubberBand,
                DisplayMode trackerMode,
                QWidget *canvas )
            :QwtPlotPicker(xAxis, yAxis, rubberBand, trackerMode, canvas){}

        inline void setSignalType(Signal::SType type){stype = type;}
        inline void setZMode(){ zMode = true; }

    protected:

        QwtText trackerTextF(const QPointF &pos)const;

    private:
        Signal::SType stype;
        bool zMode = false;
};


#endif // CUSTOMPLOTPICKER_H
