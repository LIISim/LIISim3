#include "plotavgtool.h"

#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>

#include "customQwtPlot/baseplotcurve.h"

#include <QClipboard>
#include <QApplication>

/**
 * @brief PlotAvgTool::PlotAvgTool Constructor (updateOnMove = true)
 * @param plot The parent plot
 * @param curves Pointer to List of curves within the plot
 * @param parent Parent Object (default = 0)
 */
PlotAvgTool::PlotAvgTool(QwtPlot *plot,
                         QList<BasePlotCurve *> *curves,
                         QObject *parent)
                        : PlotAnalysisTool(plot, curves, true, parent)
{
}


/**
 * @brief PlotAvgTool::~PlotAvgTool Destructor
 */
PlotAvgTool::~PlotAvgTool()
{
}


/**
 * @brief PlotAvgTool::getBoxContent get tool-specific information for box
 * @param xStart
 * @param xEnd
 * @return
 */
void PlotAvgTool::updatePlot(double xStart, double xEnd)
{
    QList<BasePlotCurve*>* active_curves = getCurves();

    QString labelText = QString("<b>Average: </b> (dt = %0ns, n=%1) <br> start: %2 ns; end: %3 ns ")
            .arg(QString::number(xEnd-xStart,'f',1))
            .arg(QString::number(active_curves->size(),'f',0))
            .arg(QString::number(xStart,'f',1))
            .arg(QString::number(xEnd,'f',1));

    QString copyTxt;

    for(int i = 0; i < active_curves->size(); i++)
    {        
        BasePlotCurve* curve = active_curves->at(i);

        double avg = 0;
        int count = 0;
        int jstart = -1;

        double min = 1e100; // start values
        double max = -1e100;

        //labelText.append(QString::number(curve->dataSize()));
        //labelText.append(curve->title().text());


        for(int j = 0; j < curve->dataSize(); j++)
        {
            double x = curve->sample(j).x();

            // skip all values left of the selection range
            if(x < xStart)
                continue;

            if(jstart < 0)
                jstart = j;

            // stop when reaching the right of the selection range
            if(x > xEnd)
            {

                break;
            }
            double y = curve->sample(j).y();

            if(y < min) min = y;
            if(y > max) max = y;
            avg += y;
            count++;
        }

        if(count == 0)
            continue;

        avg /= double(count);

        double stdd = 0.0;

        for(int j = jstart; j < jstart+count; j++)
        {
            stdd += pow(curve->sample(j).y()- avg ,2) ;
        }
        stdd =  sqrt(1.0/double(count) * stdd) / abs(avg) * 100;

        QString avg_str;
        QString min_str;
        QString max_str;

        switch(m_stype)
        {
            case Signal::RAW:

                if(avg > 1)
                {
                    avg_str=QString::number(avg,'f',3);
                    min_str=QString::number(min,'f',3);
                    max_str=QString::number(max,'f',3);
                }
                else
                {
                    avg_str=QString::number(avg,'e',6);
                    min_str=QString::number(min,'e',6);
                    max_str=QString::number(max,'e',6);
                }
                break;

            case Signal::ABS:

                if(avg > 10000)
                {
                    avg_str=QString::number(avg,'e',3);
                    min_str=QString::number(min,'e',3);
                    max_str=QString::number(max,'e',3);
                }
                else if(avg < 2)
                {
                    avg_str=QString::number(avg,'g',5);
                    min_str=QString::number(min,'g',5);
                    max_str=QString::number(max,'g',5);
                }
                else
                {
                    avg_str=QString::number(avg,'f',3);
                    min_str=QString::number(min,'f',3);
                    max_str=QString::number(max,'f',3);
                }
                break;

            case Signal::TEMPERATURE:

                avg_str=QString::number(avg,'f',1);
                min_str=QString::number(min,'f',1);
                max_str=QString::number(max,'f',1);
                break;

            default:

                avg_str=QString::number(avg,'g',6);
                min_str=QString::number(min,'g',6);
                max_str=QString::number(max,'g',6);
        }


        labelText.append(QString("<br>&nbsp;<font color='%0'>&#9632;</font>&nbsp;%1: &nbsp;&nbsp;&nbsp; %2 &#177; %3 % [min = %4  max = %5]")
                .arg(curve->pen().color().name())
                .arg(curve->title().text())
                .arg(avg_str)
                .arg(QString::number(stdd,'f',2))
                .arg(min_str)
                .arg(max_str));


        // copy this text to clipboard
        copyTxt.append(QString("%0\t%1\t%2\t%3\t%4\n").arg(curve->title().text())
                                            .arg(avg_str)
                                            .arg(QString::number(stdd,'f',2))
                                            .arg(min_str)
                                            .arg(max_str));



        // reset indicator lines
        while(!indicators.isEmpty())
        {
            indicators.front()->detach();
            delete indicators[0];
            indicators.pop_front();
        }

        // show dotted line to visualize average value
        QwtPlotMarker* indic = new QwtPlotMarker();
        QwtSymbol* indicSymbol = new QwtSymbol;
        indicSymbol->setStyle(QwtSymbol::HLine);
        indicSymbol->setPen(curve->pen().color(),2.0);
        indicSymbol->setSize(8);

        indic->setSymbol(indicSymbol);
        indic->setLineStyle(QwtPlotMarker::HLine);
        indic->setYAxis(QwtPlot::yLeft);
        indic->setValue(xEnd ,avg); // position of dotted line
        indic->setLinePen(curve->pen().color(),0.0,Qt::DotLine);

        // save indicators so that they can be found later
        indicators << indic;
        indic->attach(plot);

    } // for active curves

    // copy to clipboard
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(copyTxt);

    // display created text
    updateInfoBox(labelText);
}
