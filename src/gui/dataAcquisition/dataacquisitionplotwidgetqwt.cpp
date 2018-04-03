#include "dataacquisitionplotwidgetqwt.h"

#include <QMenu>
#include <QDebug>

#include "core.h"


// data table
#include "../utils/datatablewidget.h"

// marker
#include <qwt_symbol.h>

// mouse controls
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_picker_machine.h>

#include "../utils/customQwtPlot/customplotmagnifier.h"
#include "../utils/customQwtPlot/customplotpicker.h"

// custom for this class
#include "../utils/customQwtPlot/signalplotcurve.h"

#include "signal/streampoint.h"
#include "da_referencewidget.h"

DataAcquisitionPlotWidget::DataAcquisitionPlotWidget(QWidget *parent) : BasePlotWidgetQwt(parent)
{
    noChannels = 1;
    channelIDs << 1;

    curveLineWidth = 0;

    triggerMark = NULL;

    viewMode = STATIC;
    streamPlotOrder << 1 << 2 << 3 << 4;

    // channel bottom box
    actionShowChannelCBs_visible = true;
    channelCBlayout = new QGridLayout;
    channelCBlayout->setMargin(0);
    bottomBarLayout->addLayout(channelCBlayout);

    // assign signal type for number conversion/axis titles
    avgTool->setSignalType(stype);
    fitTool->setSignalType(stype);

    actionShowChannelCBs = new QAction("Show channel checkboxes",this);
    actionShowChannelCBs->setCheckable(true);
    actionShowChannelCBs->setChecked(false);
    connect(actionShowChannelCBs,
            SIGNAL(toggled(bool)),
            SLOT(onActionShowChannelCBs(bool)));

    onActionShowChannelCBs(actionShowChannelCBs->isChecked());
}


/******************
 *  PUBLIC SLOTS  *
 ******************/


/**
 * @brief DataAcquisitionPlotWidget::setSignalType Renames axis titles dependent on signal type
 * @param stype
 */
void DataAcquisitionPlotWidget::setSignalType(Signal::SType stype)
{
    this->stype = stype;
    QString s;

    QList<QColor> cmapColors;

    z_dataCursor->setSignalType(stype);
    avgTool->setSignalType(stype);
    fitTool->setSignalType(stype);


    if(stype == Signal::ABS)
    {
        s = "Absolute ";
        QString test = tr("Absolute intensity / arb. unit");
        test = test;
        setPlotTitle("Absolute Signal");
        setPlotAxisTitles(tr("Time / ns"),test);

        cmapColors.append(QColor(Qt::red));
        cmapColors.append(QColor(Qt::blue));
        cmapColors.append(QColor(Qt::green));
    }
    else if(stype == Signal::RAW)
    {
        s = "Raw ";
        setPlotTitle("Raw Signal");
        setPlotAxisTitles(tr("Time / ns"),tr("Voltage signal / V"));

        cmapColors.append(QColor(Qt::red));
        cmapColors.append(QColor(Qt::blue));
        cmapColors.append(QColor(Qt::green));
    }
    else if(stype == Signal::TEMPERATURE)
    {
        s = "Temperature";
        setPlotTitle("Temperature Signal");
        setPlotAxisTitles(tr("Time / ns"),tr("Temperature / K"));

        cmapColors.append(QColor(Qt::darkGreen));
        cmapColors.append(QColor(Qt::darkBlue));
        //cmapColors.append(QColor(Qt::green));
    }
    colmap.setToLinearGradient(cmapColors);
}


/**
 * @brief DataAcquisitionPlotWidget::setChannels set channel ids
 * @param chids list of channel ids
 */
void DataAcquisitionPlotWidget::setChannels(QList<int> chids)
{
    if(chids.size() < 1) chids << 1;

    noChannels = chids.size();
    channelIDs = chids;
    setMaxLegendColumns(noChannels);
    onActionShowChannelCBs(actionShowChannelCBs->isChecked());
}


/**
 * @brief DataAcquisitionPlotWidget::addSignal add signal data to plot
 * @param signal Signal which should be plotted
 * @param curveName name of signal curve
 * @param autoColor default value: generate color automatically. If set to false the currentColor is used (see setCurrentColor())
 * @details registers a new signal curve to the plot if no curve with
 * given name is registered yet. If a curve with given name is already registered,
 * only the signal data is updated.
 */
void DataAcquisitionPlotWidget::addSignal(const Signal &signal,const QString & curveName, bool autoColor)
{
    // add new curve plot if no curve with given name exists
    SignalPlotCurve* curve;
    QColor curveColor;

    if(!autoColor)
    {
        curveColor = this->currentColor;
    }

    bool inList = false;
    for(int i=0; i < curves.size(); i++)
    {
        if(curves.at(i)->title() == curveName)
        {
            inList = true;

            if(autoColor)
                curveColor = curves.at(i)->pen().color();
            curve = dynamic_cast<SignalPlotCurve*>(curves.at(i));
        }
    }

    if(!inList)
    {
        curve = new SignalPlotCurve(curveName,signal);

        curves.push_back(curve);

        if(autoColor)
            curveColor = generateNewColor(signal);

        if(actionShowDataPoints->isChecked())
        {
            QwtSymbol* s = new QwtSymbol(QwtSymbol::Cross);
            s->setColor(curveColor);
            s->setSize(12);
            curve->setSymbol(s);
        }
       // curve->setRenderHint(QwtPlotItem::RenderAntialiased);
        if(viewMode == STATIC)
            curve->attach(qwtPlot);
    }

    // apply curve properties
    curve->setPen( curveColor, curveLineWidth );
    if(curveRenderAntialiased)
        curve->setRenderHint(QwtPlotItem::RenderAntialiased);

    // generate time axis!
    QVector<double> xData;
    for(int i=0;i<signal.data.size(); i++)
    {
        xData.push_back((signal.start_time+i*signal.dt)*1e9);
    }
    curve->setSamples(xData,signal.data);

    // set visibility state of channel based on channel checkbox selection
    for(int i = 0; i < channelCBs.size(); i++)
        if(!channelCBs[i]->isChecked() &&
           signal.channelID == channelCBs[i]->property("chID").toInt())
               curve->setVisible(false);


   // qwtPlot->setAxisAutoScale(QwtPlot::yLeft);
   // qwtPlot->setAxisAutoScale(QwtPlot::xBottom);
    handleCurveDataChanged();

    if(dataTableWindow != NULL)
        dataTableWindow->updateView();
}


/**
 * @brief BasePlotWidgetQwt::detachAllCurves Removes all curves from plot and
 * deletes cuve data.
 */
void DataAcquisitionPlotWidget::detachAllCurves()
{
    selectedCurve = 0;
    dataCursorSampleIndex = -1;
    pointMarker->setVisible(false);
    while(!curves.isEmpty())
    {
        curves.first()->detach();
        delete curves.first();
        curves.removeFirst();
    }

    // remove trigger mark
    if(triggerMark != NULL)
        triggerMark->detach();

    qwtPlot->replot();
}


void DataAcquisitionPlotWidget::setTriggerMarker(double xValue, double yValue)
{
    if(triggerMark == NULL)
    {
        triggerMark = new QwtPlotMarker("Trigger Point");

        QwtSymbol *symbol = new QwtSymbol(QwtSymbol::Style::Diamond);
        symbol->setSize(10);
        symbol->setColor(QColor("yellow"));

        triggerMark->setSymbol(symbol);
        triggerMark->setLineStyle(QwtPlotMarker::LineStyle::NoLine);
    }

    triggerMark->setXValue(xValue);
    triggerMark->setYValue(yValue);
    if(viewMode == STATIC)
        triggerMark->attach(qwtPlot);
}


/************************
 *  PRIVATE SLOTS  *
 ************************/

/**
 * @brief DataAcquisitionPlotWidget::generateNewColor generates color
 * @return QColor
 * @details generate a new color dependent on the number of registered signals
 * and the number of channels.
 * TODO: actually generate colors !!!
 */
QColor DataAcquisitionPlotWidget::generateNewColor(const Signal& signal)
{
    QColor c;
    int noPlots = curves.size();

    c = colmap.color( signal.channelID, 1.0, noChannels );

    if(noPlots>noChannels && (stype != Signal::TEMPERATURE))
        c = c.darker(100+ 40* (noPlots/2));

    return c;
}

/**
 * @brief DataAcquisitionPlotWidget::onContextMenuEvent creates context menu on right click
 * @param pos position of event
 */
void DataAcquisitionPlotWidget::onContextMenuEvent(QPoint pos)
{
    QMenu menu(this);
    menu.addAction(actionShowDataTable);
    menu.addSeparator();
    menu.addAction(actionChangeScaleTypeLogX);
    menu.addAction(actionChangeScaleTypeLogY);
    menu.addSeparator();
    QMenu plotMenu("Plot type");
    menu.addMenu(&plotMenu);
    plotMenu.addActions(plotTypeActions->actions());

    menu.addAction(actionShowLegend);

    if(actionShowChannelCBs_visible)
        menu.addAction(actionShowChannelCBs);
    menu.exec( QCursor::pos());
}


/**
 * @brief DataAcquisitionPlotWidget::setShowChannelCheckBoxes changes
 * action checked state, toggles visibility of channel checkboxes
 * @param state
 */
void DataAcquisitionPlotWidget::setShowChannelCheckBoxes(bool state)
{
    actionShowChannelCBs->setChecked(state);
}


/**
 * @brief DataAcquisitionPlotWidget::setActionShowChannelCBsVisible
 * changes visibility of "Show Channel Checkboxes" in context menu.
 * This can be used to hide the action from the user!
 * @param state
 */
void DataAcquisitionPlotWidget::setActionShowChannelCBsVisible(bool state)
{
    actionShowChannelCBs_visible = false;
}


/**
 * @brief DataAcquisitionPlotWidget::onActionShowChannelCBs this slot is executed when
 * the corresponding view action has been triggered. Shows/hides checkboxes
 * for channel visibility
 * @param state
 */
void DataAcquisitionPlotWidget::onActionShowChannelCBs(bool state)
{
    while(!channelCBs.isEmpty())
        delete channelCBs.takeFirst();

    if(state && channelCBs.size() != noChannels)
        for(int i = 0; i < noChannels; i++)
        {
            QCheckBox* cb = new QCheckBox(QString("ch %0").arg(channelIDs[i]),qwtPlot->canvas());
            cb->setChecked(true);
            cb->setProperty("chID",channelIDs[i]);
            connect(cb,SIGNAL(toggled(bool)),
                    SLOT(onChannelVisCheckBoxToggled(bool)));
            channelCBs << cb;
            channelCBlayout->addWidget(cb,0,i);
        }
}


/**
 * @brief DataAcquisitionPlotWidget::onChannelVisCheckBoxToggled This slot is
 * executed when a channel checkbox has been toggled. Modifies visibility
 * of plot curves with same channel ID as sender QCheckBox.
 * @param state new checked state
 */
void DataAcquisitionPlotWidget::onChannelVisCheckBoxToggled(bool state)
{
    QObject* s = QObject::sender();
    int cid = s->property("chID").toInt();



    for(int i = 0; i < curves.size(); i++)
    {
        SignalPlotCurve* s_curve = dynamic_cast<SignalPlotCurve*>(curves[i]);
        if(!s_curve)
            continue;

        if(s_curve->chID() == cid)
            s_curve->setVisible(state);

    }

    handleCurveDataChanged();
}


void DataAcquisitionPlotWidget::addStreamSignal(const Signal &signal, const QString &curveName, QColor curveColor)
{
    SignalPlotCurve *curve = new SignalPlotCurve(curveName, signal);
    stream_curves.push_back(curve);

    if(actionShowDataPoints->isChecked())
    {
        QwtSymbol* s = new QwtSymbol(QwtSymbol::Cross);
        s->setColor(curveColor);
        s->setSize(12);
        curve->setSymbol(s);
    }

    curve->attach(qwtPlot);

    //apply curve properties
    curve->setPen(curveColor, curveLineWidth);
    if(curveRenderAntialiased)
        curve->setRenderHint(QwtPlotItem::RenderAntialiased);

    //generate time axis!
    QVector<double> xData;
    for(int i=0;i<signal.data.size(); i++)
    {
        xData.push_back((signal.start_time+i*signal.dt)*1e9);
    }
    curve->setSamples(xData,signal.data);
    curve->setPaintAttribute(QwtPlotCurve::FilterPoints);
}


void DataAcquisitionPlotWidget::updateStreamPoint(StreamPoint *point)
{
    if(viewMode == STREAM)
    {
        while(!stream_curves.isEmpty())
        {
            stream_curves.first()->detach();
            delete stream_curves.first();
            stream_curves.removeFirst();
        }

        for(int i = 0; i < streamPlotOrder.size(); i++)
        {
            switch(streamPlotOrder[i])
            {
            case 1:
            {
                //Add channel A to plot
                if(Core::instance()->guiSettings->value("picoscopewidget", "channelAvisible", true).toBool())
                {
                    if(point->single.contains(1) && point->average.contains(1))
                    {
                        addStreamSignal(point->single.value(1), "Channel A", QColor("blue").lighter());
                        addStreamSignal(point->average.value(1), "Average Channel A", QColor("blue"));
                    }
                    else
                    {
                        if(point->single.contains(1))
                            addStreamSignal(point->single.value(1), "Channel A", QColor("blue").lighter());
                        if(point->average.contains(1))
                            addStreamSignal(point->average.value(1), "Average Channel A", QColor("blue").lighter());
                    }
                }
            } break;
            case 2:
            {
                //Add channel B to plot
                if(Core::instance()->guiSettings->value("picoscopewidget", "channelBvisible", true).toBool())
                {
                    if(point->single.contains(2) && point->average.contains(2))
                    {
                        addStreamSignal(point->single.value(2), "Channel B", QColor("red").lighter());
                        addStreamSignal(point->average.value(2), "Average Channel B", QColor("red"));
                    }
                    else
                    {
                        if(point->single.contains(2))
                            addStreamSignal(point->single.value(2), "Channel B", QColor("red").lighter());
                        if(point->average.contains(2))
                            addStreamSignal(point->average.value(2), "Average Channel B", QColor("red").lighter());
                    }
                }
            } break;
            case 3:
            {
                //Add channel C to plot
                if(Core::instance()->guiSettings->value("picoscopewidget", "channelCvisible", true).toBool())
                {
                    if(point->single.contains(3) && point->average.contains(3))
                    {
                        addStreamSignal(point->single.value(3), "Channel C", QColor("green").lighter());
                        addStreamSignal(point->average.value(3), "Average Channel C", QColor("green"));
                    }
                    else
                    {
                        if(point->single.contains(3))
                            addStreamSignal(point->single.value(3), "Channel C", QColor("green").lighter());
                        if(point->average.contains(3))
                            addStreamSignal(point->average.value(3), "Average Channel C", QColor("green").lighter());
                    }
                }
            } break;
            case 4:
            {
                //Add channel D to plot
                if(Core::instance()->guiSettings->value("picoscopewidget", "channelDvisible", true).toBool())
                {
                    if(point->single.contains(4) && point->average.contains(4))
                    {
                        addStreamSignal(point->single.value(4), "Channel D", QColor("yellow"));
                        addStreamSignal(point->average.value(4), "Average Channel D", QColor("yellow").dark(150));
                    }
                    else
                    {
                        if(point->single.contains(4))
                            addStreamSignal(point->single.value(4), "Channel D", QColor("yellow"));
                        if(point->average.contains(4))
                            addStreamSignal(point->average.value(4), "Average Channel D", QColor("yellow"));
                    }
                }
            } break;
            }
        }

        handleCurveDataChanged();

        if(dataTableWindow != NULL)
            dataTableWindow->updateView();
    }
}


void DataAcquisitionPlotWidget::setViewMode(ViewMode mode)
{
    viewMode = mode;

    if(mode == STATIC)
    {
        while(!stream_curves.isEmpty())
        {
            stream_curves.first()->detach();
            delete stream_curves.first();
            stream_curves.removeFirst();
        }

        for(int i = 0; i < reference_curves.size(); i++)
        {
            reference_curves.at(i)->detach();
        }

        for(int i = 0; i < static_curves.size(); i++)
        {
            static_curves.at(i)->detach();
        }

        for(int i = 0; i < curves.size(); i++)
        {
            curves.at(i)->attach(qwtPlot);
        }

        if(triggerMark != NULL)
            triggerMark->attach(qwtPlot);

        handleCurveDataChanged();

        if(dataTableWindow != NULL)
            dataTableWindow->updateView();
    }
    else if(mode == STREAM)
    {
        for(int i = 0; i < curves.size(); i++)
        {
            curves.at(i)->detach();
        }

        if(triggerMark != NULL)
            triggerMark->detach();

        for(int i = 0; i < reference_curves.size(); i++)
        {
            reference_curves.at(i)->attach(qwtPlot);
        }

        for(int i = 0; i < static_curves.size(); i++)
        {
            static_curves.at(i)->attach(qwtPlot);
        }

        handleCurveDataChanged();

        if(dataTableWindow != NULL)
            dataTableWindow->updateView();
    }
}


void DataAcquisitionPlotWidget::switchStreamSignalLayer(unsigned int curve, bool up)
{
    int position = -1;
    for(int i = 0; i < streamPlotOrder.size(); i++)
        if(streamPlotOrder[i] == curve)
            position = i;

    if(up && position != -1)
    {
        if(position+1 < streamPlotOrder.size())
            streamPlotOrder.swap(position, position+1);
    }
    if(!up && position != -1)
    {
        if(position-1 >= 0)
            streamPlotOrder.swap(position, position-1);
    }
}


void DataAcquisitionPlotWidget::addReferenceSignal(const Signal &signal, const QString &curveName, QColor curveColor)
{
    SignalPlotCurve *curve = new SignalPlotCurve(curveName, signal);
    reference_curves.push_back(curve);

    if(actionShowDataPoints->isChecked())
    {
        QwtSymbol* s = new QwtSymbol(QwtSymbol::Cross);
        s->setColor(curveColor);
        s->setSize(12);
        curve->setSymbol(s);
    }

    if(viewMode == STREAM)
        curve->attach(qwtPlot);

    //apply curve properties
    curve->setPen(curveColor, curveLineWidth);
    if(curveRenderAntialiased)
        curve->setRenderHint(QwtPlotItem::RenderAntialiased);

    //generate time axis!
    QVector<double> xData;
    for(int i=0;i<signal.data.size(); i++)
    {
        xData.push_back((signal.start_time+i*signal.dt)*1e9);
    }
    curve->setSamples(xData,signal.data);
    curve->setPaintAttribute(QwtPlotCurve::FilterPoints);
}


void DataAcquisitionPlotWidget::updateReferences(const QList<QPair<DA_ReferenceElementWidget*, Signal>> references)
{
    while(!reference_curves.isEmpty())
    {
        reference_curves.first()->detach();
        delete reference_curves.first();
        reference_curves.removeFirst();
    }

    if(references.isEmpty())
        return;

    int colorFactor = 120;

    int channelACount = 0;
    int channelBCount = 0;
    int channelCCount = 0;
    int channelDCount = 0;

    for(int i = 0; i < references.size(); i++)
    {
        if(references.at(i).first->getCheckedState() == Qt::Checked)
        {
            switch(references.at(i).first->channel)
            {
            case 1: channelACount++; break;
            case 2: channelBCount++; break;
            case 3: channelCCount++; break;
            case 4: channelDCount++; break;
            }
        }
    }

    int colorAddA = 0;
    int colorAddB = 0;
    int colorAddC = 0;
    int colorAddD = 0;

    if(channelACount != 0)
        colorAddA = (280 - colorFactor) / channelACount;
    if(channelBCount != 0)
        colorAddB = (280 - colorFactor) / channelBCount;
    if(channelCCount != 0)
        colorAddC = (280 - colorFactor) / channelCCount;
    if(channelDCount != 0)
        colorAddD = (280 - colorFactor) / channelDCount;

    channelACount = 0;
    channelBCount = 0;
    channelCCount = 0;
    channelDCount = 0;

    for(int i = 0; i < references.size(); i++)
    {
        if(references.at(i).first->getCheckedState() == Qt::Checked)
        {
            QColor color;

            switch(references.at(i).first->channel)
            {
            case 1: color = QColor("blue").darker(colorFactor + colorAddA * channelACount); channelACount++; break;
            case 2: color = QColor("red").darker(colorFactor + colorAddB * channelBCount); channelBCount++; break;
            case 3: color = QColor("green").darker(colorFactor + colorAddC * channelCCount); channelCCount++; break;
            case 4: color = QColor("yellow").darker(colorFactor + colorAddD * channelDCount); channelDCount++; break;
            }

            addReferenceSignal(references.at(i).second, references.at(i).first->getText().append(" - ").append(references.at(i).first->getTooltip()), color);
        }
    }

    handleCurveDataChanged();

    if(dataTableWindow != NULL)
        dataTableWindow->updateView();
}


void DataAcquisitionPlotWidget::addStaticSignal(const Signal &signal, const QString &curveName, bool autoColor)
{
    QColor curveColor;
    if(!autoColor)
    {
        curveColor = this->currentColor;
    }
    else
    {
        int noPlots = static_curves.size();

        curveColor = colmap.color( signal.channelID, 1.0, noChannels );

        if(noPlots > noChannels && (stype != Signal::TEMPERATURE))
            curveColor = curveColor.darker(100+ 40* (noPlots/2));
    }

    SignalPlotCurve *curve = new SignalPlotCurve(curveName, signal);
    static_curves.push_back(curve);

    if(actionShowDataPoints->isChecked())
    {
        QwtSymbol* s = new QwtSymbol(QwtSymbol::Cross);
        s->setColor(curveColor);
        s->setSize(12);
        curve->setSymbol(s);
    }

    if(viewMode == STREAM)
        curve->attach(qwtPlot);

    //apply curve properties
    curve->setPen(curveColor, curveLineWidth);
    if(curveRenderAntialiased)
        curve->setRenderHint(QwtPlotItem::RenderAntialiased);

    //generate time axis!
    QVector<double> xData;
    for(int i=0;i<signal.data.size(); i++)
    {
        xData.push_back((signal.start_time+i*signal.dt)*1e9);
    }
    curve->setSamples(xData,signal.data);
    curve->setPaintAttribute(QwtPlotCurve::FilterPoints);
    curve->setZ(-5.0);

    handleCurveDataChanged();

    if(dataTableWindow != NULL)
        dataTableWindow->updateView();
}


void DataAcquisitionPlotWidget::detachStaticCurves()
{
    while(!static_curves.isEmpty())
    {
        static_curves.first()->detach();
        delete static_curves.first();
        static_curves.removeFirst();
    }
}










