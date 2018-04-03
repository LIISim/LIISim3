#include "customplotpicker.h"


#include <qwt_plot.h>
#include <qwt_plot_spectrogram.h>

/**
 * @brief CustomPlotPicker::trackerTextF generates a custom tracker text
 * based on the currently selected signal type
 * @param pos
 * @return
 */
QwtText CustomPlotPicker::trackerTextF(const QPointF &pos) const
{
    // WORKAROUND, USE zMode ONLY FOR QWTPLOTSPECTROGRAM!!!
    if(zMode)
    {
        QwtPlotItemList plotItems = plot()->itemList();

        if(plotItems.size() == 0)
        {
            qDebug() << "CustomPlotPicker: no plot items";

            return QwtText("no plot items");
        }
        else if(plotItems.at(0) == NULL)
        {
            return QwtText("no plot");
        }
        else
        {
            QwtPlotSpectrogram *plotItem = dynamic_cast<QwtPlotSpectrogram *>(plotItems.at(0));

            double z = plotItem->data()->value(pos.x(), pos.y());

            return QwtText(QString("x = %0, y = %1, z = %2")
                       .arg(QString::number(pos.x(),'e',4))
                       .arg(QString::number(pos.y(),'e',4))
                       .arg(QString::number(z,'e',4)));
        }
    }

    switch(stype)
    {
        case Signal::RAW:
            if(pos.y() > 1)
                return QwtText(QString("x = %0, y = %1")
                           .arg(QString::number(pos.x(),'f',2))
                           .arg(QString::number(pos.y(),'f',4)));
            else
                return QwtText(QString("x = %0, y = %1")
                           .arg(QString::number(pos.x(),'f',1))
                           .arg(QString::number(pos.y(),'f',6)));

        case Signal::ABS:
            if(pos.y() > 10000)
                return QwtText(QString("x = %0, y = %1")
                           .arg(QString::number(pos.x(),'f',1))
                           .arg(QString::number(pos.y(),'e',3)));
            else if (pos.y() < 2.0)
                return  QwtText(QString("x = %0, y = %1")
                            .arg(QString::number(pos.x(),'f',1))
                            .arg(QString::number(pos.y(),'g',5)));
            else
                return QwtText(QString("x = %0, y = %1")
                           .arg(QString::number(pos.x(),'f',1))
                           .arg(QString::number(pos.y(),'f',3)));
        case Signal::TEMPERATURE:
            return QwtText(QString("x = %0, y = %1")
                           .arg(QString::number(pos.x(),'f',1))
                           .arg(QString::number(pos.y(),'f',2)));
        default:
            return QwtText(QString("x = %0, y = %1")
                           .arg(QString::number(pos.x(),'g',6))
                           .arg(QString::number(pos.y(),'g',6)));
    }
}
