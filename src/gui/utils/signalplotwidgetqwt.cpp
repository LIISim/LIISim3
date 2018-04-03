#include "signalplotwidgetqwt.h"

#include <QMenu>
#include <QDebug>


// data table
#include "datatablewidget.h"

// marker
#include <qwt_symbol.h>

// mouse controls
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_picker_machine.h>

#include "customQwtPlot/customplotmagnifier.h"
#include "customQwtPlot/customplotpicker.h"

// custom for this class
#include "customQwtPlot/signalplotcurve.h"
#include "customQwtPlot/signalplotintervalcurve.h"

/**
 * @brief SignalPlotWidgetQwt::SignalPlotWidgetQwt Constructor
 * @param parent parent widget (default = 0)
 */
SignalPlotWidgetQwt::SignalPlotWidgetQwt(QWidget *parent) : BasePlotWidgetQwt(parent)
{  
    noChannels = 1;
    channelIDs << 1;
    activeChannels << 1;

    cgChannelCounter = 0;

    // these colors are too light
    //predefinedColors.push_back(QColor(158,1,66)); // red
    //predefinedColors.push_back(QColor(244,109,67)); // orange
    //predefinedColors.push_back(QColor(254,224,139)); // yellow
    //predefinedColors.push_back(QColor(171,221,164)); // green
    //predefinedColors.push_back(QColor(50,136,189)); // blue
    // predefinedColorsSignal.push_back(QColor(158,1,66)); // red
    // predefinedColorsSignal.push_back(QColor(213,62,79));
    // predefinedColorsSignal.push_back(QColor(253,174,97));
    // predefinedColorsSignal.push_back(QColor(230,245,152));
    // predefinedColorsSignal.push_back(QColor(102,194,165));
    // predefinedColorsSignal.push_back(QColor(94,79,162));

    // inspired by http://colorbrewer2.org
    predefinedColorsSignal.push_back(QColor(0,0,0)); // black
    predefinedColorsSignal.push_back(QColor(2,118,189)); // blue
    predefinedColorsSignal.push_back(QColor(82,221,62)); // green
    predefinedColorsSignal.push_back(QColor(243,198,22)); // yellow
    predefinedColorsSignal.push_back(QColor(244,69,15)); // orange
    predefinedColorsSignal.push_back(QColor(158,1,66)); // red
    predefinedColorsSignal.push_back(QColor(213,62,79));
    predefinedColorsSignal.push_back(QColor(253,174,97));
    predefinedColorsSignal.push_back(QColor(230,245,152));
    predefinedColorsSignal.push_back(QColor(102,194,165));
    predefinedColorsSignal.push_back(QColor(94,79,162));

    predefinedColorsTemp.push_back(QColor(0,0,0)); // black
    predefinedColorsTemp.push_back(QColor(2,118,189)); // blue
    predefinedColorsTemp.push_back(QColor(82,221,62)); // green
    predefinedColorsTemp.push_back(QColor(243,198,22)); // yellow
    predefinedColorsTemp.push_back(QColor(244,69,15)); // orange
    predefinedColorsTemp.push_back(QColor(158,1,66)); // red
    predefinedColorsTemp.push_back(QColor(94,11,76)); // purple
    predefinedColorsTemp.push_back(QColor(253,174,97));
    predefinedColorsTemp.push_back(QColor(230,245,152));
    predefinedColorsTemp.push_back(QColor(102,194,165));
    predefinedColorsTemp.push_back(QColor(94,79,162));

    triggerMark = NULL;

    // channel bottom box
    actionShowChannelCBs_visible = true;
    channelCBlayout = new QGridLayout;
    channelCBlayout->setMargin(0);
    bottomBarLayout->addLayout(channelCBlayout);

    // assign signal type for number conversion/axis titles
    avgTool->setSignalType(stype);
    fitTool->setSignalType(stype);

    // visualize standard deviation
    actionShowStdev = new QAction("Show standard deviation envelope",this);
    actionShowStdev->setCheckable(true);
    actionShowStdev->setChecked(true);

    connect(actionShowStdev,
            SIGNAL(toggled(bool)),
            SLOT(onActionShowStdev(bool)));

    // channel visibility
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


QColor SignalPlotWidgetQwt::getSignalColor(int idx)
{
    if(idx >= predefinedColorsSignal.size())
        return predefinedColorsSignal.at(0); // black

    return predefinedColorsSignal.at(idx);
}


QColor SignalPlotWidgetQwt::getTemperatureColor(int idx)
{
    if(idx >= predefinedColorsTemp.size())
        return predefinedColorsTemp.at(0); // black

    return predefinedColorsTemp.at(idx);
}


/******************
 *  PUBLIC SLOTS  *
 ******************/

/**
 * @brief SignalPlotWidgetQwt::setSignalType Renames axis titles dependent on signal type
 * @param stype
 */
void SignalPlotWidgetQwt::setSignalType(Signal::SType stype)
{
    this->stype = stype;
    QString s;

    // currently not used anymore, replaced by predefinedColorsTemp and predefinedColorsSignal
    QList<QColor> cmapColors;

    z_dataCursor->setSignalType(stype);
    avgTool->setSignalType(stype);
    fitTool->setSignalType(stype);


    if(stype == Signal::ABS)
    {
        s = "Absolute ";                
        setPlotTitle("Absolute Signal");
        setPlotAxisTitles(tr("Time / ns"), tr("Absolute intensity / arb. unit"));

        cmapColors.append(QColor(Qt::red));
        cmapColors.append(QColor(Qt::blue));
        cmapColors.append(QColor(Qt::green));
    }
    else if(stype == Signal::RAW)
    {
        s = "Raw ";
        setPlotTitle("Raw Signal");
        setPlotAxisTitles(tr("Time / ns"), tr("Voltage signal / V"));

        cmapColors.append(QColor(Qt::red));
        cmapColors.append(QColor(Qt::blue));
        cmapColors.append(QColor(Qt::green));
    }
    else if(stype == Signal::TEMPERATURE)
    {
        s = "Temperature";
        setPlotTitle("Temperature Trace");
        setPlotAxisTitles(tr("Time / ns"), tr("Temperature / K"));

        cmapColors.append(QColor(Qt::darkGreen));        
        cmapColors.append(QColor(Qt::darkMagenta));
        cmapColors.append(QColor(Qt::darkGray));
        cmapColors.append(QColor(Qt::darkYellow));
        cmapColors.append(QColor(Qt::darkBlue));
        cmapColors.append(QColor(Qt::darkRed));
        //cmapColors.append(QColor(Qt::green));
    }
    colmap.setToLinearGradient(cmapColors);
}


/**
 * @brief SignalPlotWidgetQwt::setChannels set channel ids
 * @param chids list of channel ids
 */
void SignalPlotWidgetQwt::setChannels(QList<int> chids)
{
    if(chids.size() < 1) chids << 1;

    noChannels = chids.size();
    channelIDs = chids;

    if(actionShowChannelCBs->isChecked())
    {
        for(int i = 0; i < activeChannels.size();)
        {
            if(!chids.contains(activeChannels.at(i)))
                activeChannels.removeAt(i);
            else
                i++;
        }
    }
    else
        activeChannels = chids;

    setMaxLegendColumns(noChannels);
    onActionShowChannelCBs(actionShowChannelCBs->isChecked());
}


/**
 * @brief SignalPlotWidgetQwt::addSignal add signal data to plot
 * @param signal Signal which should be plotted
 * @param curveName name of signal curve
 * @param autoColor default value: generate color automatically. If set to false the currentColor is used (see setCurrentColor())
 * @details registers a new signal curve to the plot if no curve with
 * given name is registered yet. If a curve with given name is already registered,
 * only the signal data is updated.
 */
void SignalPlotWidgetQwt::addSignal(const Signal &signal,const QString & curveName, bool autoColor)
{
    // add new curve plot if no curve with given name exists
    SignalPlotCurve* curve;
    SignalPlotIntervalCurve *intervalCurve = nullptr;
    QColor curveColor;

    if(!autoColor)
    {
        curveColor = this->currentColor;
    }

    bool inList = false;

    for(int i = 0; i < curves.size(); i++)
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
        curve = new SignalPlotCurve(curveName, signal);

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
        curve->attach(qwtPlot);
    }

    // apply curve properties
    curve->setPen( curveColor, curveLineWidth );
    if(curveRenderAntialiased)
        curve->setRenderHint(QwtPlotItem::RenderAntialiased);

    // generate time axis!
    QVector<double> xData;
    QVector<QwtIntervalSample> rangeData;

    for(int i = 0; i < signal.data.size(); i++)
    {
        xData.push_back((signal.start_time + i * signal.dt) * 1e9);

        if(signal.stdev.size() > 0)
        {
            rangeData.push_back(QwtIntervalSample( xData.at(i),
                                               QwtInterval( signal.data.at(i) - signal.stdev.at(i),
                                                            signal.data.at(i) + signal.stdev.at(i)) ));
        }
    }

    // draw evelope function of standard deviation
    if(signal.stdev.size() > 0)
    {
      intervalCurve = new SignalPlotIntervalCurve(curveName + " Standard Deviation ", signal);
      intervalCurve->setRenderHint( QwtPlotItem::RenderAntialiased );

      QColor bg(curveColor);
      bg.setAlpha( 120 );
      intervalCurve->setPen(bg);
      intervalCurve->setBrush( QBrush( bg ) );
      intervalCurve->setStyle( QwtPlotIntervalCurve::Tube );

      intervalCurve->setSamples(rangeData);
      intervalCurve->setVisible(actionShowStdev->isChecked());
      intervalCurve->setItemAttribute(QwtPlotItem::Legend, actionShowStdev->isChecked());
      intervalCurve->attach(qwtPlot);

      stdev_curves.append(intervalCurve);
    }

    // attach data
    curve->setSamples(xData,signal.data);

    // set visibility state of channel based on channel checkbox selection
    for(int i = 0; i < channelCBs.size(); i++)
        if(!channelCBs[i]->isChecked() && signal.channelID == channelCBs[i]->property("chID").toInt())
        {
            curve->setVisible(false);
            if(intervalCurve)
                intervalCurve->setVisible(false);
        }

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
void SignalPlotWidgetQwt::detachAllCurves()
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

    while(!stdev_curves.isEmpty())
    {
        stdev_curves.first()->detach();
        delete stdev_curves.first();
        stdev_curves.removeFirst();
    }

    // remove trigger mark
    if(triggerMark != NULL)
        triggerMark->detach();

    qwtPlot->replot();
}


void SignalPlotWidgetQwt::setTriggerMarker(double xValue, double yValue)
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
    triggerMark->attach(qwtPlot);
}


/************************
 *  PRIVATE SLOTS  *
 ************************/


/**
 * @brief SignalPlotWidgetQwt::generateNewColor generates color
 * @return QColor
 * @details generate a new color dependent on the number of registered signals
 * and the number of channels.
 */
QColor SignalPlotWidgetQwt::generateNewColor(const Signal& signal)
{
    int noPlots = curves.size();

    //qDebug() << "channelID" << signal.channelID << "noChannels" << noChannels << "noPlots" << noPlots;

    QColor color;

    if(stype == Signal::TEMPERATURE)
    {
        if(signal.channelID > predefinedColorsTemp.size()-1)
        {
            if(cgChannelCounter > predefinedColorsTemp.size()-1)
                cgChannelCounter = 0;
            color = predefinedColorsTemp.at(cgChannelCounter);
            cgChannelCounter++;
        }
        else
        {
            cgChannelCounter = 0;
            color = predefinedColorsTemp.at(signal.channelID);
        }
    }
    else
    {
        if(signal.channelID > predefinedColorsSignal.size()-1)
        {
            if(cgChannelCounter > predefinedColorsSignal.size()-1)
                cgChannelCounter = 0;
            color = predefinedColorsSignal.at(cgChannelCounter);
            cgChannelCounter++;
        }
        else
        {
            cgChannelCounter = 0;
            color = predefinedColorsSignal.at(signal.channelID);
        }
    }

    if(noPlots > noChannels && (stype != Signal::TEMPERATURE))
        color = color.darker(70 + 20 * (noPlots / 2));

    return color;

    /*QColor c;
    int noPlots = curves.size();

    c = colmap.color( signal.channelID, 1.0, noChannels );

    if(noPlots > noChannels && (stype != Signal::TEMPERATURE))
        c = c.darker(100+ 40* (noPlots/2));

    return c;*/
}


/**
 * @brief SignalPlotWidgetQwt::onContextMenuEvent creates context menu on right click
 * @param pos position of event
 */
void SignalPlotWidgetQwt::onContextMenuEvent(QPoint pos)
{
    QMenu menu(this);
    menu.addAction(actionShowDataTable);
    menu.addSeparator();
    menu.addAction(actionChangeScaleTypeLogX);
    menu.addAction(actionChangeScaleTypeLogY);
    menu.addSeparator();
    menu.addAction(actionShowStdev);
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
 * @brief SignalPlotWidgetQwt::onActionShowStdev toogle visibility of standard deviation envelope function
 * @param state
 */
void SignalPlotWidgetQwt::onActionShowStdev(bool state)
{
    for(int i = 0; i < stdev_curves.size(); i++)
    {
        SignalPlotIntervalCurve* s_curve = dynamic_cast<SignalPlotIntervalCurve*>(stdev_curves[i]);
        if(!s_curve)
            continue;

        if(activeChannels.contains(s_curve->chID()) && state)
        {
            s_curve->setVisible(state);
            s_curve->setItemAttribute(QwtPlotItem::Legend, state);
        }
        else if(!state)
        {
            s_curve->setVisible(state);
            s_curve->setItemAttribute(QwtPlotItem::Legend, state);
        }
    }
    handleCurveDataChanged();
}


/**
 * @brief SignalPlotWidgetQwt::setShowChannelCheckBoxes changes
 * action checked state, toggles visibility of channel checkboxes
 * @param state
 */
void SignalPlotWidgetQwt::setShowChannelCheckBoxes(bool state)
{
    actionShowChannelCBs->setChecked(state);
}


/**
 * @brief SignalPlotWidgetQwt::setActionShowChannelCBsVisible
 * changes visibility of "Show Channel Checkboxes" in context menu.
 * This can be used to hide the action from the user!
 * @param state
 */
void SignalPlotWidgetQwt::setActionShowChannelCBsVisible(bool state)
{
    actionShowChannelCBs_visible = false;
}


/**
 * @brief SignalPlotWidgetQwt::onActionShowChannelCBs this slot is executed when
 * the corresponding view action has been triggered. Shows/hides checkboxes
 * for channel visibility
 * @param state
 */
void SignalPlotWidgetQwt::onActionShowChannelCBs(bool state)
{
    //if checkboxes are hidden, enable all channels to prevent confusion
    if(!state)
    {
        for(int i = 0; i < channelCBs.size(); i++)
            channelCBs.at(i)->setChecked(true);
    }

    while(!channelCBs.isEmpty())
        delete channelCBs.takeFirst();

    if(state && channelCBs.size() != noChannels)
    {
        for(int i = 0; i < noChannels; i++)
        {
            QString cbText;
            if(stype == Signal::TEMPERATURE)
                cbText = QString("T%0").arg(channelIDs.at(i));
            else
                cbText = QString("Channel %0").arg(channelIDs.at(i));
            QCheckBox* cb = new QCheckBox(cbText, qwtPlot->canvas());
            if(activeChannels.contains(channelIDs[i]))
                cb->setChecked(true);
            cb->setProperty("chID",channelIDs[i]);
            connect(cb,SIGNAL(toggled(bool)),
                    SLOT(onChannelVisCheckBoxToggled(bool)));
            channelCBs << cb;
            channelCBlayout->addWidget(cb,0,i);
        }
    }
}


/**
 * @brief SignalPlotWidgetQwt::onChannelVisCheckBoxToggled This slot is
 * executed when a channel checkbox has been toggled. Modifies visibility
 * of plot curves with same channel ID as sender QCheckBox.
 * @param state new checked state
 */
void SignalPlotWidgetQwt::onChannelVisCheckBoxToggled(bool state)
{
    QObject* s = QObject::sender();
    int cid = s->property("chID").toInt();

    if(!state && activeChannels.contains(cid))
        activeChannels.removeAll(cid);
    if(state && !activeChannels.contains(cid))
        activeChannels.push_back(cid);

    for(int i = 0; i < curves.size(); i++)
    {
        SignalPlotCurve* s_curve = dynamic_cast<SignalPlotCurve*>(curves[i]);
        if(!s_curve)
            continue;

        if(s_curve->chID() == cid)
            s_curve->setVisible(state);
    }

    for(int i = 0; i < stdev_curves.size(); i++)
    {
        SignalPlotIntervalCurve* s_curve = dynamic_cast<SignalPlotIntervalCurve*>(stdev_curves[i]);
        if(!s_curve)
            continue;

        if(s_curve->chID() == cid && !state)
            s_curve->setVisible(state);
        else if(s_curve->chID() == cid && actionShowStdev->isChecked())
            s_curve->setVisible(state);
    }

    handleCurveDataChanged();
}


void SignalPlotWidgetQwt::onShowDataTable()
{
    if(dataTableWindow == NULL)
    {
        dataTableWindow = new DataTableWidget(&curves, &stdev_curves);
        dataTableWindow->show();
    }
    QString windowTitle = "";
    if(!dataTableRunName.isEmpty())
        windowTitle.append(dataTableRunName + " - ");
    if(!dataTableToolName.isEmpty())
        windowTitle.append(dataTableToolName + " - ");
    if(!qwtPlot->title().text().isEmpty())
        windowTitle.append(qwtPlot->title().text() + " Data");
    dataTableWindow->setWindowTitle(windowTitle);
    dataTableWindow->show();
    dataTableWindow->updateView();
}
