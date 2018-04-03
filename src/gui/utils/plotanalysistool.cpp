#include "plotanalysistool.h"

#include <QDebug>

#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoneitem.h>
#include <qwt_plot_marker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>

#include "customQwtPlot/baseplotcurve.h"

PlotAnalysisTool::PlotAnalysisTool(QwtPlot *plot,
                                   QList<BasePlotCurve*>* curves,
                                   bool updateOnMove,
                                   QObject *parent)
              : QObject(parent)
              , plot(plot)
              , curves(curves)
              , updateOnMove(updateOnMove)
              , m_enabled(false)
              , m_visible(false)
{    

    // default signal type
    m_stype = Signal::RAW;


    // setup the picker tool for selection of a rectangular region
    rectPicker = new QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
                                        QwtPlotPicker::NoRubberBand,
                                        QwtPicker::AlwaysOff,
                                        plot->canvas());



    rectPicker->setStateMachine( new QwtPickerDragRectMachine());

    if(!updateOnMove)
    {
        connect(rectPicker,SIGNAL(activated(bool)),SLOT(onMouseClickUpdate()));
        connect(rectPicker,SIGNAL(selected(QRectF)),SLOT(onMouseReleaseUpdate()));
    }

    // onMove update cursor and update if updateOnMove == true
    connect(rectPicker,SIGNAL(moved(QPointF)), SLOT(onRectPickerMoved(QPointF)));


    rectPicker->setRubberBandPen( QColor(Qt::gray));

    // disable keyboard control
    rectPicker->setKeyPattern(QwtEventPattern::KeyLeft,0);
    rectPicker->setKeyPattern(QwtEventPattern::KeyRight,0);
    rectPicker->setKeyPattern(QwtEventPattern::KeyUp,0);
    rectPicker->setKeyPattern(QwtEventPattern::KeyDown,0);

    // setup the two vertical marker on the x-axis
    xMarker1 = new QwtPlotMarker("x1");
    xMarker1->setLineStyle(QwtPlotMarker::VLine);
    xMarker1->setXAxis(QwtPlot::xBottom);
    xMarker1->setLinePen(QColor(77,77,153),1);
    xMarker1->attach(plot);
    xMarker1->setVisible(m_visible);

    xMarker2 = new QwtPlotMarker("x2");
    xMarker2->setLineStyle(QwtPlotMarker::VLine);
    xMarker2->setXAxis(QwtPlot::xBottom);
    xMarker2->setLinePen(QColor(77,77,153),1);
    xMarker2->attach(plot);
    xMarker2->setLabelAlignment(Qt::AlignBottom | Qt::AlignRight);
    xMarker2->setVisible(m_visible);

    hide = new QPushButton("[hide]",plot->canvas());
    hide->setStyleSheet("background-color:rgba(185,185,215,100);color:rgb(77,77,153);border: 0;");
    hide->resize(35,20);
    hide->setToolTip("hide the averaging tool");
    hide->setVisible(false);
    connect(hide,SIGNAL(released()),SLOT(onHideButtonReleased()));

    zone = new QwtPlotZoneItem();
    zone->attach(plot);
    zone->setVisible(false);
    zone->setAxes(QwtPlot::xBottom,QwtPlot::yLeft);
    zone->setBrush(QBrush(QColor(245,245,255)));

    QwtScaleWidget* scaleWx = plot->axisWidget(QwtPlot::xBottom);
    connect(scaleWx,SIGNAL(scaleDivChanged()),this,SLOT(onXscaleChanged()));

    QwtScaleWidget* scaleWy = plot->axisWidget(QwtPlot::yLeft);
    connect(scaleWy,SIGNAL(scaleDivChanged()),this,SLOT(onYscaleChanged()));
}

/**
 * @brief PlotAnalysisTool::~PlotAnalysisTool Destructor
 */
PlotAnalysisTool::~PlotAnalysisTool()
{
}


/**
 * @brief PlotAnalysisTool::setMarkers manual setting of marker position
 * @param xStart [seconds]
 * @param xEnd [seconds]
 */
void PlotAnalysisTool::setMarkers(double xStart, double xEnd)
{
    // show tool within plot
    setVisible(true);

    // convert from s to ns
    xStart  = xStart * 1E9;
    xEnd    = xEnd * 1E9;

    xMarker1->setXValue(xStart);
    xMarker2->setXValue(xEnd);
    zone->setInterval(xStart,xEnd);

    // update hide button position
    int posEnd   = plot->transform(QwtPlot::xBottom, xEnd);
    hide->move(posEnd,0);

    // pass to other classes and update plot
    updateData();

    // refresh plot to make changes visible
    plot->replot();
}


void PlotAnalysisTool::setUpdateOnMove(bool state)
{
    updateOnMove = state;

    // disconnect all slots
    rectPicker->disconnect();

    rectPicker->setStateMachine( new QwtPickerDragRectMachine());

    if(!updateOnMove)
    {
        connect(rectPicker,SIGNAL(activated(bool)),SLOT(onMouseClickUpdate()));
        connect(rectPicker,SIGNAL(selected(QRectF)),SLOT(onMouseReleaseUpdate()));
    }

    // onMove update cursor and update if updateOnMove == true
    connect(rectPicker,SIGNAL(moved(QPointF)), SLOT(onRectPickerMoved(QPointF)));
}

/**
 * @brief PlotAnalysisTool::setSignalType changes the current signal type.
 * The signal type is only needed to provide a proper text-representation
 * of data values.
 * @param stype
 */
void PlotAnalysisTool::setSignalType(Signal::SType stype)
{
    m_stype = stype;
}


/**
 * @brief PlotAnalysisTool::setEnabled Enables/Disables the mouse control
 * for ROI-Selection. Remark: The tool is still visible in disabled state
 * (also see: PlotAnalysisTool::setVisible(bool)).
 * @param enabled
 */
void PlotAnalysisTool::setEnabled(bool enabled)
{
    m_enabled = enabled;
    rectPicker->setEnabled(enabled);
}


/**
 * @brief PlotAnalysisTool::setVisible changes the visibility of
 * the tool within the plot
 * @param visible
 */
void PlotAnalysisTool::setVisible(bool visible)
{
    m_visible = visible;

    if(m_visible && updateOnMove)
        updateData();
    else
    {
        while(!indicators.isEmpty())
        {
            indicators.front()->detach();
            delete indicators[0];
            indicators.pop_front();
        }
        hideCustom();
    }
    xMarker1->setVisible(m_visible);
    xMarker2->setVisible(m_visible);
    zone->setVisible(m_visible);
    hide->setVisible(m_visible);
}


/**
 * @brief PlotAnalysisTool::onMouseClickUpdate if(updateOnMove == false) indicates start of selection
 */
void PlotAnalysisTool::onMouseClickUpdate()
{
    //qDebug() << "PlotAnalysisTool: onMouseClickUpdate";
    updateInfoBox(QString(""));
}


/**
 * @brief PlotAnalysisTool::onMouseReleaseUpdate if(updateOnMove == false) indicates end of selection
 */
void PlotAnalysisTool::onMouseReleaseUpdate()
{
    //qDebug() << "PlotAnalysisTool: onMouseReleaseUpdate";

    // update zone information
    onRectPickerMoved(QPointF());

    // update GUI
    updateData();
    plot->replot();
}


/**
 * @brief PlotAnalysisTool::onRectPickerMoved This slot is executed when
 * the rectangular mouse selection has been edited. Updates the GUI.
 * @param point
 */
void PlotAnalysisTool::onRectPickerMoved(const QPointF &point)
{
    //qDebug() << "PlotAnalysisTool: onRectPickerMoved";
    if(!m_enabled|| rectPicker->selection().size() < 2)
        return;

    int pos1 = rectPicker->selection().first().x();
    int pos2 = rectPicker->selection().last().x();

    int tmp;
    if(pos1 > pos2)
    {
        tmp = pos1;
        pos1 = pos2;
        pos2 = tmp;
    }

    // these coords are saved in "zone" even when zoom or pan is used:
    double xStart = plot->invTransform(QwtPlot::xBottom, pos1);
    double xEnd   = plot->invTransform(QwtPlot::xBottom, pos2);

    xMarker1->setXValue(xStart);
    xMarker2->setXValue(xEnd);

    zone->setInterval(xStart,xEnd);

    hide->move(pos2,0);

    if(!m_visible)
        setVisible(true);
    else if(updateOnMove == true)
        updateData();

    plot->replot();
}


/**
 * @brief PlotAnalysisTool::updateData This method passes position to other classes and updates plot
 * (due to efficiency reasons: no replot here!)
 */
void PlotAnalysisTool::updateData()
{
    if(!m_visible)
        return;

    if(zone->interval().isNull())
        return;

    double xStart = zone->interval().minValue();
    double xEnd   = zone->interval().maxValue();

    //qDebug() << "pixel: " << pos1 << pos2;
    //qDebug() << "coords: " << xStart << xEnd;

    // emit signal with coords for other classes (atoolcalibration.cpp)
    emit dataSelected(xStart, xEnd);

    // updatePlot is individual function for each tool
    updatePlot(xStart, xEnd);
}


/**
 * @brief PlotAnalysisTool::updatePlot virtual function is overwritten by child classes
 * @param xStart
 * @param xEnd
 */
void PlotAnalysisTool::updatePlot(double xStart, double xEnd)
{
    // default: return selected time interval
    QString labelText = QString("");

    if(!objectName().isEmpty())
        labelText.append(QString("<b> %0 </b><br>").arg(objectName()));

    labelText.append(QString("<b>dt = %0ns</b> <br> (%1 ns to %2 ns) ")
            .arg(QString::number(xEnd-xStart,'f',1))
            .arg(QString::number(xStart,'f',1))
            .arg(QString::number(xEnd,'f',1)));

    updateInfoBox(labelText);
}


/**
 * @brief PlotAnalysisTools::updateInfoBox using information provided by individual tool
 * @param labelText
 */
void PlotAnalysisTool::updateInfoBox(QString labelText)
{
    QwtText txt(labelText,QwtText::RichText);
    txt.setBackgroundBrush(QBrush(QColor( 185,185,215,100 )));
    txt.setRenderFlags(Qt::AlignLeft);
    xMarker2->setLabel(txt);
}


/**
 * @brief PlotAnalysisTool::onHideButtonReleased This slot is executed when the
 * hide button has been clicked.
 * Hides the tool.
 */
void PlotAnalysisTool::onHideButtonReleased()
{
    setVisible(false);
    plot->replot();

    emit hidden();
}


/**
 * @brief PlotAnalysisTool::onCurveDataChanged This slot should be called from
 * the outside if the curve data of the plot has been modified
 */
void PlotAnalysisTool::onCurveDataChanged()
{ 
    updateData();
}


/**
 * @brief PlotAnalysisTool::onXscaleChanged This slot is called when the
 * plot's x-axis-scale has changed.
 * Moves the hide-button to new position.
 */
void PlotAnalysisTool::onXscaleChanged()
{

    hide->move(plot->transform(QwtPlot::xBottom,
               zone->boundingRect().right()),0);
}


/**
 * @brief PlotAnalysisTool::onYscaleChanged This slot is called when the
 * plot's x-axis-scale has changed.
 * Does nothing
 */
void PlotAnalysisTool::onYscaleChanged()
{
}

/**
 * @brief PlotAnalysisTool::onPlotResized This slot should be called
 * when the plot has been resized.
 * Moves the hide-button to new position.
 * @param event
 */
void PlotAnalysisTool::onPlotResized(QResizeEvent *event)
{
    hide->move(plot->transform(QwtPlot::xBottom,
               zone->boundingRect().right()),0);
}


