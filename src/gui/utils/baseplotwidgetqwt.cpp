#include "baseplotwidgetqwt.h"

#include "../../core.h";

#include <QSplitter>
#include <QMenu>
#include <QContextMenuEvent>

#include <qwt_plot.h>
#include "customQwtPlot/baseplotcurve.h"

// legend
#include <qwt_plot_legenditem.h>

// axis
#include <qwt_scale_draw.h>
#include <qwt_plot_scaleitem.h>

// options
#include "customQwtPlot/customplotoptionswidget.h"

// scaling
#include <qwt_scale_engine.h>
#include "customQwtPlot/customlogscaleengine.h"
#include <qwt_scale_widget.h>

// canvas
#include <qwt_plot_canvas.h>

// grid
#include <qwt_plot_grid.h>

// data table
#include "datatablewidget.h"

// mouse controls
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_picker_machine.h>
#include "plotzoomer.h"

// marker
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>

// text
#include <qwt_plot_textlabel.h>

#include "customQwtPlot/customplotmagnifier.h"
#include "customQwtPlot/customplotpicker.h"


BasePlotWidgetQwt::BasePlotWidgetQwt(QWidget *parent) : QWidget(parent)
{

    /*********
     * PLOT
     *********/

    qwtPlot = new QwtPlot(this);
    selectedCurve = 0;
    dataTableWindow = NULL;
    xAxisNonTimeType = false;

    // default settings
    zmode = PLOT_PAN;

    currentColor = Qt::black;
    colmap.setToJetStyle();

    // set standard axis scale
    qwtPlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
    qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);

    // set miniumun height to 50 px
    setMinimumHeight(50);

    // set up plot canvas
    QwtPlotCanvas *canvas = new QwtPlotCanvas();
    canvas->setFocusIndicator( QwtPlotCanvas::CanvasFocusIndicator );
    canvas->setFocusPolicy( Qt::StrongFocus );
    canvas->setPalette( Qt::white );
    canvas->setFrameShadow(QwtPlotCanvas::Plain);
    canvas->setFrameShape(QwtPlotCanvas::StyledPanel);

    qwtPlot->setCanvas( canvas );

    // set grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin( true );
    grid->setMajorPen( Qt::gray, 0, Qt::DotLine );
    grid->setMinorPen( Qt::darkGray, 0, Qt::DotLine );
    grid->attach( qwtPlot );

    // text item
    titleItem = new QwtPlotTextLabel();

    /*************
     * PLOT: FONT
     *************/

    // setup title font
    if (Core::LIISIM_LARGE_FONTS)
        titleFont.setPointSize(10); // enlarged text
    else
        titleFont.setPointSize(8);
    titleFont.setBold(true);

    // setup axes font
    if (Core::LIISIM_LARGE_FONTS)
        axisFont.setPointSize(12); // enlarged text
    else
        axisFont.setPointSize(8);

    qwtPlot->setAxisFont(QwtPlot::yLeft,axisFont);
    qwtPlot->setAxisFont(QwtPlot::xBottom,axisFont);


    // curve properties
    curveLineWidth = 1.5;
    curveRenderAntialiased = false;

    /******************
     * PLOT: LEGEND
     ******************/

    // setup legend properties
    legendItem = new QwtPlotLegendItem;
    legendItem->setAlignment(Qt::AlignRight | Qt::AlignTop);
    legendItem->setMaxColumns(2);

    if (Core::LIISIM_LARGE_FONTS)
        legendItem->setFont(axisFont); // enlarged text

    legendItem->setRenderHint(QwtPlotItem::RenderAntialiased);
    QColor legendTxtColor(Qt::black);
    legendItem->setTextPen(legendTxtColor);
    legendItem->setBorderPen(legendTxtColor);
    legendItem->setBorderPen(legendTxtColor);
    legendItem->setBorderRadius(2);
    QColor legendBgColor(Qt::gray);
    legendBgColor.setAlpha(10);
    legendItem->setBackgroundBrush(legendBgColor);
    legendItem->setBackgroundMode(QwtPlotLegendItem::LegendBackground);
    legendItem->setBorderRadius( 8 );
    legendItem->setMargin( 4 );
    legendItem->setSpacing( 2 );
    legendItem->setItemMargin( 0 );

    legendItem->attach(qwtPlot);

    /******************
     * PLOT: AXES
     ******************/
    // y axis: move scale lables inside plot (TODO find another solution)
    QwtPlotScaleItem* scaleItemY = new QwtPlotScaleItem(QwtScaleDraw::RightScale,0.0);
    scaleItemY->setFont(axisFont);
    scaleItemY->attach(qwtPlot);
    scaleItemY->setBorderDistance(1);

    // hide labels,ticks and backbone of y axis (only the title remains)
    // otherwise yaxis appears inside AND outside of plot window
    qwtPlot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Labels, false );
    qwtPlot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false );
    qwtPlot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, false );

    linkXscale = false;
    linkYscale = false;

    QwtScaleWidget* scaleWx = qwtPlot->axisWidget(QwtPlot::xBottom);
    connect(scaleWx,SIGNAL(scaleDivChanged()),this,SLOT(onXscaleChanged()));

    QwtScaleWidget* scaleWy = qwtPlot->axisWidget(QwtPlot::yLeft);
    connect(scaleWy,SIGNAL(scaleDivChanged()),this,SLOT(onYscaleChanged()));

    /******************
     * STANDARD LAYOUT
     ******************/

    layMain = new QGridLayout;
    this->setLayout(layMain);
    layMain->setMargin(2);

    // main horizontal splitter (contains plot)
    QSplitter* mainHSplit = new QSplitter(Qt::Horizontal);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainHSplit->setSizePolicy(sizePolicy);

    mainHSplit->addWidget(qwtPlot);

    layMain->addWidget(mainHSplit,0,0,1,2);


    // TODO: optionsWidget can be used to show plot configuration in QTreeWidget next to plot
    // Currently NOT used:
    /*
    optionsWidget = new CustomPlotOptionsWidget(this);
    optionsWidget->setEnabled(false);
    optionsWidget->setVisible(false);
    mainHSplit->addWidget(optionsWidget);
    layMain->addWidget(optionsWidget,0,1);
    */

    bottomBarLayout = new QHBoxLayout;
    layMain->addLayout(bottomBarLayout, 1,0);


    /*************************
     * PLOT: DATA CURSOR BOX (contains plot color and coordinated below plot)
     ************************/


    dataCursorEnabled = false;
    dataCursorSampleIndex = -1;

    QHBoxLayout* dataCursorLayout = new QHBoxLayout;

    dataCursorColorLabel = new QLabel;
    dataCursorColorLabel->setMinimumWidth(15);
    dataCursorColorLabel->setMaximumWidth(15);
    dataCursorColorLabel->setVisible(dataCursorEnabled);

    dataCursorLayout->addWidget(dataCursorColorLabel);

    dataCursorTextLabel = new QLabel;
    dataCursorLayout->addWidget(dataCursorTextLabel);
    dataCursorTextLabel->setVisible(dataCursorEnabled);

    bottomBarLayout->addLayout(dataCursorLayout);
    bottomBarLayout->addWidget(new QWidget);

    /*************************************************
     * PLOT: MARKER (DATA CURSOR TOOL and TRIGGER)
     ************************************************/

    pointMarker = new QwtPlotMarker("");

    QwtSymbol* pointMarkerSymbol = new QwtSymbol;
    pointMarkerSymbol->setStyle(QwtSymbol::Cross);
    pointMarkerSymbol->setSize(20);

    pointMarker->setSymbol(pointMarkerSymbol);
    pointMarker->attach(qwtPlot);


    /*************************
     * PLOT: MOUSE CONTROLS
     *************************/

    // panning with the left mouse button
    z_panner = new QwtPlotPanner( canvas );

    // data cursor
    z_dataCursor = new CustomPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
                                  QwtPlotPicker::CrossRubberBand,
                                  QwtPicker::AlwaysOn,
                                  canvas);

    z_dataCursor->setStateMachine( new QwtPickerDragPointMachine() );
    z_dataCursor->setRubberBandPen( QColor( Qt::gray ) );
    z_dataCursor->setRubberBand( QwtPicker::CrossRubberBand);
    z_dataCursor->setTrackerPen( QColor( Qt::black ));

    // disable keyboard control for plot marker
    z_dataCursor->setKeyPattern(QwtEventPattern::KeyLeft,0);
    z_dataCursor->setKeyPattern(QwtEventPattern::KeyRight,0);
    z_dataCursor->setKeyPattern(QwtEventPattern::KeyUp,0);
    z_dataCursor->setKeyPattern(QwtEventPattern::KeyDown,0);

    connect(z_dataCursor,SIGNAL(appended(QPoint)),SLOT(onDataCursorSelection(QPoint)));

    // rectangular zoom tool
    //z_rectZoomer = new QwtPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, canvas);
    z_rectZoomer = new PlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, canvas);
    z_rectZoomer->setRubberBand( QwtPicker::RectRubberBand );
    z_rectZoomer->setRubberBandPen( QColor( Qt::red ) );
    z_rectZoomer->setTrackerMode( QwtPicker::ActiveOnly );
    z_rectZoomer->setTrackerPen( QColor( Qt::gray ) );

    // zoom in/out with the wheel
    z_mag = new CustomPlotMagnifier( canvas );

    z_mag_x = new CustomPlotMagnifier( canvas );
    z_mag_y = new CustomPlotMagnifier( canvas );
    z_mag_x->setWheelModifiers(Qt::ControlModifier);
    z_mag_x->setAxisEnabled(QwtPlot::xBottom, true);
    z_mag_x->setAxisEnabled(QwtPlot::yLeft,false);
    z_mag_y->setWheelModifiers(Qt::ShiftModifier);
    z_mag_y->setAxisEnabled(QwtPlot::xBottom,false);
    z_mag_y->setAxisEnabled(QwtPlot::yLeft,true);

    // disable the "right click hold" zoom
    z_mag->setMouseButton(Qt::NoButton, Qt::NoModifier);
    z_mag_x->setMouseButton(Qt::NoButton, Qt::NoModifier);
    z_mag_y->setMouseButton(Qt::NoButton, Qt::NoModifier);


    // averaging tool
    avgTool = new PlotAvgTool(qwtPlot,&curves);

    // fit tool
    fitTool = new PlotFitTool(qwtPlot,&curves);

    // plot analysis tool (normally hidden)
    plotAnalysisTool = new PlotAnalysisTool(qwtPlot,&curves, true);

    z_panner->setEnabled(true);
    z_rectZoomer->setEnabled(false);

    z_mag->setEnabled(false);
    z_mag_x->setEnabled(false);
    z_mag_y->setEnabled(false);

    avgTool->setEnabled(false);
    fitTool->setEnabled(false);

    /**********************
     * PLOT: CONTEXT MENU
     **********************/

    // general
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(onContextMenuEvent(QPoint)));

    // data table
    actionShowDataTable = new QAction(tr("Show data table"),this);
    connect(actionShowDataTable,SIGNAL(triggered()),this,SLOT(onShowDataTable()));

    // scaling options
    actionChangeScaleTypeLogX = new QAction(tr("Set X-axis log scale"),this);
    actionChangeScaleTypeLogX->setCheckable(true);
    actionChangeScaleTypeLogY = new QAction(tr("Set Y-axis log scale"),this);
    actionChangeScaleTypeLogY->setCheckable(true);

    connect(actionChangeScaleTypeLogX,SIGNAL(toggled(bool)),
            this,SLOT(onChangeScaleTypeLogX(bool)));

    connect(actionChangeScaleTypeLogY,SIGNAL(toggled(bool)),
            this,SLOT(onChangeScaleTypeLogY(bool)));

    // plot type selection (lines, dots, dots+lines)
     plotTypeActions = new QActionGroup(this);
     plotTypeActions->setExclusive(true);
     connect(plotTypeActions,
             SIGNAL(triggered(QAction*)),
             SLOT(onPlotTypeActionTriggerd(QAction*)));

     actionShowDataLine = new QAction("line",this);
     actionShowDataLine->setCheckable(true);
     actionShowDataLine->setChecked(true);
     plotTypeActions->addAction(actionShowDataLine);

     actionShowDataPoints = new QAction("data points + line",this);
     actionShowDataPoints->setCheckable(true);
     plotTypeActions->addAction(actionShowDataPoints);

     actionShowDataDotsSize1 = new QAction("dots (small)",this);
     actionShowDataDotsSize1->setCheckable(true);
     plotTypeActions->addAction(actionShowDataDotsSize1);

     actionShowDataDotsSize2 = new QAction("dots (medium)",this);
     actionShowDataDotsSize2->setCheckable(true);
     plotTypeActions->addAction(actionShowDataDotsSize2);

     actionShowDataDotsSize3 = new QAction("dots (large)",this);
     actionShowDataDotsSize3->setCheckable(true);
     plotTypeActions->addAction(actionShowDataDotsSize3);

     // legend
     actionShowLegend = new QAction("Show legend",this);
     actionShowLegend->setCheckable(true);
     actionShowLegend->setChecked(true);
     connect(actionShowLegend,SIGNAL(toggled(bool)),SLOT(onActionShowLegend(bool)));


    /**********************
     * PLOT: ACTIONS
     **********************/

    toolActionGroup = new QActionGroup(this);

    toolAction_PlotPan = new QAction("plot pan",this);
    toolAction_FitTool = new QAction("fit",this);
    toolAction_AvgTool = new QAction("avg",this);
    toolAction_RectZoom = new QAction("zoom rect",this);
    toolAction_ZoomReset = new QAction("reset zoom",this);
    toolAction_DataCursor = new QAction("data cursor",this);

    toolActionGroup->addAction(toolAction_PlotPan);
    toolActionGroup->addAction(toolAction_FitTool);
    toolActionGroup->addAction(toolAction_AvgTool);
    toolActionGroup->addAction(toolAction_RectZoom);

    toolAction_PlotPan->setCheckable(true);
    toolAction_PlotPan->setChecked(true);

    toolAction_PlotAnalysisTool = new QAction("no name yet",this);

    toolAction_PlotPan->setToolTip("Plot panning: Left Mouse Button \nZoom: Wheel\nZoom x-Axis only: Wheel + Ctrl\nZoom y-Axis only: Wheel + Shift");
    toolAction_FitTool->setToolTip("Fit curve withing rectangular selection");
    toolAction_AvgTool->setToolTip("Calculate average withing rectangular selection");
    toolAction_RectZoom->setToolTip("Select zoom-rectangle: Left Mouse Button \nZoom: Wheel\nZomm x-Axis only: Wheel + Ctrl\nZoom y-Axis only: Wheel + Shift");
    toolAction_ZoomReset->setToolTip("Reset zoom");
    toolAction_DataCursor->setToolTip("Allows the selection of single datapoints\n(move marker with right/left keys)");

    // set default actions
    toolAction_PlotPan->setChecked(true);
    toolAction_DataCursor->setChecked(dataCursorEnabled);

    addToolAction(toolAction_PlotPan, true);
    addToolAction(toolAction_FitTool, true);
    addToolAction(toolAction_AvgTool, true);
    addToolAction(toolAction_RectZoom, true);
    addToolAction(toolAction_ZoomReset, false);
    addToolAction(toolAction_DataCursor, true);

    plotType = LINE;

    //Plot text label

    plotLabel = new QwtPlotTextLabel;
    plotlabelText = new QwtText;

    QFont font = plotlabelText->font();
    font.setPointSize(11);
    plotlabelText->setFont(font);
    plotlabelText->setRenderFlags(Qt::TextWordWrap);
    plotLabel->setText(*plotlabelText);
}


BasePlotWidgetQwt::~BasePlotWidgetQwt()
{
    if(dataTableWindow != NULL)
        delete dataTableWindow;

    delete avgTool;
    delete fitTool;
    delete plotAnalysisTool;
}


/**************************
 *  TOOL PUBLIC FUNCTIONS *
 **************************/

/**
 * @brief BasePlotWidgetQwt::toolActions returns a
 * list of Actions for Zoom/Plot Tools
 * @return
 */
QList<QAction*> BasePlotWidgetQwt::toolActions()
{
    return m_actions;
}


void BasePlotWidgetQwt::addPlotAnalyzer(QString name, QString tooltip, bool enabled, bool updateOnMove)
{

    plotAnalysisTool->setUpdateOnMove(updateOnMove);

    toolAction_PlotAnalysisTool->setText(name);
    toolAction_PlotAnalysisTool->setToolTip(tooltip);
    toolAction_PlotAnalysisTool->setCheckable(true);

    m_actions << toolAction_PlotAnalysisTool;

    connect(toolAction_PlotAnalysisTool,SIGNAL(triggered()),SLOT(onActionPlotAnalysisTool()));

    // forward signal
    connect(plotAnalysisTool, SIGNAL(dataSelected(double,double)),SIGNAL(rangeSelected(double, double)));

    // if enabled on start
    if(enabled)
    {
        toolAction_PlotAnalysisTool->setChecked(true);

        plotAnalysisTool->setEnabled(true);

        z_panner->setEnabled(false);
        z_rectZoomer->setEnabled(false);

        z_mag->setEnabled(true);
        z_mag_x->setEnabled(true);
        z_mag_y->setEnabled(true);

        avgTool->setEnabled(false);
        fitTool->setEnabled(false);
    }
}


PlotAnalysisTool* BasePlotWidgetQwt::addPlotAnalysisTool(QString name, QString tooltip, bool updateonMove)
{
    PlotAnalysisTool *pat = new PlotAnalysisTool(qwtPlot, &curves, updateonMove);
    pat->setObjectName(name);
    listPlotAnalysisTools.push_back(pat);

    QAction *toolAction = new QAction(name, pat);
    toolAction->setToolTip(tooltip);
    toolAction->setCheckable(true);

    toolActionGroup->addAction(toolAction);

    listPlotAnalysisToolActions << toolAction;
    m_actions << toolAction;

    connect(toolAction, SIGNAL(triggered(bool)), SLOT(onActionPlotAnalysisToolTriggered()));

    return pat;
}


void BasePlotWidgetQwt::onActionPlotAnalysisToolTriggered()
{
    for(int i = 0; i < listPlotAnalysisTools.size(); i++)
        listPlotAnalysisTools.at(i)->setEnabled(false);

    if(dynamic_cast<QAction*>(QObject::sender())->isChecked())
    {
        dynamic_cast<PlotAnalysisTool*>(QObject::sender()->parent())->setEnabled(true);

        z_panner->setEnabled(false);
        z_rectZoomer->setEnabled(false);

        z_mag->setEnabled(true);
        z_mag_x->setEnabled(true);
        z_mag_y->setEnabled(true);

        avgTool->setEnabled(false);
        fitTool->setEnabled(false);

        toolAction_PlotPan->setChecked(false);
        toolAction_RectZoom->setChecked(false);
        toolAction_AvgTool->setChecked(false);
        toolAction_FitTool->setChecked(false);
    }
    else
    {
        dynamic_cast<PlotAnalysisTool*>(QObject::sender()->parent())->setEnabled(false);
        setZoomMode(PLOT_PAN);
        toolAction_PlotPan->setChecked(true);
    }
}


/**
 * @brief BasePlotWidgetQwt::toolAction gets the action
 * of a certain zoom/plot tool by PlotZoomMode
 * @param mode
 * @return
 */
QAction* BasePlotWidgetQwt::toolAction(PlotZoomMode mode)
{
    if(mode == PLOT_PAN)
        return toolAction_PlotPan;
    else if( mode == DATA_CURSOR)
        return toolAction_DataCursor;
    else if( mode == ZOOM_RECT )
        return toolAction_RectZoom;
    else if( mode == AVG_RECT)
        return toolAction_AvgTool;
    else if( mode == FIT_RECT)
        return toolAction_FitTool;
    else if( mode == ZOOM_RESET)
        return toolAction_ZoomReset;
    return 0;
}


/**
 * @brief BasePlotWidgetQwt::toolActionDataCursor get
 * tool action of plot marker tool (for manual trigger: toolActionDataCursor()->triggered(true);
 * @return
 */
QAction* BasePlotWidgetQwt::toolActionDataCursor()
{
    return toolAction_DataCursor;
}


void BasePlotWidgetQwt::addText(QString text)
{
    QwtText qwt_text(text);
    qwt_text.setRenderFlags( Qt::AlignLeft | Qt::AlignTop );


    QFont font;
    font.setBold( true );
    font.setPointSize(10);
    qwt_text.setFont( font );

    titleItem->setMargin(10);
    titleItem->setText(qwt_text);
    titleItem->attach(qwtPlot);
}


void BasePlotWidgetQwt::appendText(QString text)
{

    text = QString(titleItem->text().text()) + "\n" + text;

    addText(text);
}


void BasePlotWidgetQwt::clearText()
{
    addText("");
}


/***************************************
 *  PROTECTED FUNCTIONS:
 ***************************************/

void BasePlotWidgetQwt::addToolAction(QAction *action, bool checkable)
{
    action->setCheckable(checkable);

    m_actions << action;

    connect(action, SIGNAL(triggered()),this, SLOT(onToolActionTriggered()));
}


/**
 * @brief BasePlotWidgetQwt::selectClosestCurve find the curve which is closest to a certain point
 * @param point position in plot
 * @return true on success
 */
bool BasePlotWidgetQwt::selectClosestCurve(const QPoint &point)
{
    BasePlotCurve* curve = 0;
    double shortestDist = 100e10;
    for(int i = 0; i < curves.size(); i++)
    {
        double tmpdist;
        int pos = curves.at(i)->closestPoint(point,&tmpdist);
        if( tmpdist < shortestDist && pos > -1 )
        {
            shortestDist = tmpdist;
            curve = curves.at(i);
        }
    }
    selectedCurve = curve;

    if(curve)
        return true;
    return false;
}


/**
 * @brief BasePlotWidgetQwt::setDataCursorPosition changes position of data sample marker
 * @param position
 */
void BasePlotWidgetQwt::setDataCursorPosition(const QPointF &position)
{
    if(!selectedCurve)
        return;

    pointMarker->setValue(position);
    qwtPlot->replot();

    // update marker labels
    QString style;
    QColor curveColor = selectedCurve->pen().color();
    style.sprintf("background-color: rgb(%d, %d, %d);",
                  curveColor.red(),
                  curveColor.green(),
                  curveColor.blue());
    dataCursorColorLabel->setStyleSheet(style);

    QString text = QString(selectedCurve->title().text() + ": (%0, %1)" ).arg(
                position.x()).arg(
                position.y());
    dataCursorTextLabel->setText(text);

    emit dataSampleMarkerMoved(selectedCurve->title().text(),
                               curveColor,
                               position);
}


/***************************
 *  PUBLIC SLOTS: CURVES
 ***************************/

void BasePlotWidgetQwt::handleCurveDataChanged(bool replot)
{
    avgTool->updateData();
    fitTool->updateData();

    onPlotTypeActionTriggerd();

    qwtPlot->updateLegend();

    if(replot)
        qwtPlot->replot();

    // emit signal to notify listeners, that the curve data has changed
    emit curveDataChanged();
}


/**
 * @brief BasePlotWidgetQwt::registerCurve adds curve to plot and to internal curve list
 * (this ensures that the curve will also be listed in the plots data table).
 * @param curve
 */
void BasePlotWidgetQwt::registerCurve(BasePlotCurve *curve)
{
    curve->attach(qwtPlot);
    if(!curves.contains(curve))
        curves.append(curve);

    if(actionShowDataPoints->isChecked())
    {
        QwtSymbol* s = new QwtSymbol(QwtSymbol::Cross);
        s->setColor(curve->pen().color());
        s->setSize(12);
        curve->setSymbol(s);
    }

    handleCurveDataChanged();
    // show marker if it is enabled and curve was the first curve
   // if(curves.size() == 1 && markerEnabled )
   // {
   //     pointMarker->setVisible(true);
   // }
}


/**
 * @brief BasePlotWidgetQwt::unregisterCurve removes curve from plot and from
 * internal curve list.
 * @param curve
 */
void BasePlotWidgetQwt::unregisterCurve(BasePlotCurve *curve)
{
    curve->detach();

    curves.removeOne(curve);
    //onAvgPickerMoved(QPointF());

    if(curve == selectedCurve)
    {
        selectedCurve = 0;
        dataCursorSampleIndex = -1;
    }

    // hide marker if the marker is enabled and curve was the last curve in plot
    if(curves.isEmpty() && dataCursorEnabled)
    {
        pointMarker->setVisible(false);
    }
    handleCurveDataChanged();
}


/**
 * @brief BasePlotWidgetQwt::detachAllCurves Removes all curves from plot and
 * deletes curve data.
 */
void BasePlotWidgetQwt::detachAllCurves()
{
    selectedCurve = 0;
    dataCursorSampleIndex = -1;
    pointMarker->setVisible(false);
    while(!curves.isEmpty())
    {
        curves.first()->detach();
        // delete curve object
        delete curves.first();
        curves.removeFirst();
    }
    qwtPlot->replot();
}



void BasePlotWidgetQwt::setXLogScale(bool e)
{
    // update context menu for consistency
    actionChangeScaleTypeLogX->setChecked(e);
    onChangeScaleTypeLogX(e);
}


void BasePlotWidgetQwt::setYLogScale(bool e)
{
    // update context menu for consistency
    actionChangeScaleTypeLogY->setChecked(e);
    onChangeScaleTypeLogY(e);
}



/************************************
 *  PUBLIC SLOTS: PLOT CUSTOMIZATION
 ************************************/

/**
 * @brief BasePlotWidgetQwt::setPlotTitle set plot title
 * @param plotTitle plot title
 */
void BasePlotWidgetQwt::setPlotTitle(const QString &plotTitle)
{
    QwtText title(plotTitle);
    title.setFont(titleFont);
    qwtPlot->setTitle(title);
}


/**
 * @brief BasePlotWidgetQwt::setPlotAxisTitle set axis titles
 * @param xTitle title for x-axis
 * @param yTitle title for y-axis
 */
void BasePlotWidgetQwt::setPlotAxisTitles(const QString &xTitle, const QString &yTitle)
{
    QwtText title1(xTitle);
    title1.setFont(axisFont);    
    qwtPlot->setAxisTitle(QwtPlot::xBottom, title1);


    QwtText title2(yTitle);
    title2.setFont(axisFont);
    qwtPlot->setAxisTitle(QwtPlot::yLeft, title2);
}


/***********************
 * PUBLIC SLOTS: PLOT
 ***********************/

void BasePlotWidgetQwt::setXView(double xmin, double xmax)
{
    bool linkState = linkXscale;

    linkXscale = false;
    qwtPlot->setAxisScale(QwtPlot::xBottom,xmin,xmax);
    qwtPlot->replot();
    linkXscale = linkState;
}


void BasePlotWidgetQwt::setYView(double ymin, double ymax)
{
    bool linkState = linkYscale;

    linkYscale = false;
    qwtPlot->setAxisScale(QwtPlot::yLeft,ymin,ymax);
    qwtPlot->replot();
    linkYscale = linkState;
}


void BasePlotWidgetQwt::setView(double xmin, double xmax, double ymin, double ymax)
{
    qwtPlot->setAxisScale(QwtPlot::xBottom,xmin,xmax);
    qwtPlot->setAxisScale(QwtPlot::yLeft,ymin,ymax);
}


/**
 * @brief BasePlotWidgetQwt::getView get the current view rect of plot
 * @return view rect
 */
QRectF BasePlotWidgetQwt::getView()
{
    QRectF view;
    QwtInterval ix =  qwtPlot->axisInterval(QwtPlot::xBottom);
    QwtInterval iy = qwtPlot->axisInterval(QwtPlot::yLeft);

    view.setCoords(ix.minValue(),iy.minValue(),ix.maxValue(),iy.maxValue());
    return view;
}


/**
 * @brief BasePlotWidgetQwt::setEnabledXViewLink enable/disable
 * linked axis scaling of x-axis. If this option is enabled the widget
 * will emit the xViewChanged signal if the x-axis scaling has been edited
 * @param e enable x-link
 */
void BasePlotWidgetQwt::setEnabledXViewLink(bool e)
{
    linkXscale = e;
}


/**
 * @brief BasePlotWidgetQwt::setEnabledYViewLink enable/disable
 * linked axis scaling of y-axis. If this option is enabled the widget
 * will emit the yViewChanged signal if the y-axis scaling has been edited
 * @param e enable y-link
 */
void BasePlotWidgetQwt::setEnabledYViewLink(bool e)
{
    linkYscale = e;
}


/**
 * @brief BasePlotWidgetQwt::setMaxLegendColumns Sets number of columns of legend.
 * @param cols
 */
void BasePlotWidgetQwt::setMaxLegendColumns(int cols)
{
    legendItem->setMaxColumns(cols);
}


/**
 * @brief BasePlotWidgetQwt::setCurrentColor set current color for signal plots.
 * This color will be used only if the autoColor argument ist set to false when adding a signal.
 * @param color
 */
void BasePlotWidgetQwt::setCurrentColor(QColor color)
{
    this->currentColor = color;
}


/***********************
 * TOOLS PUBLIC SLOTS
 ***********************/

/**
 * @brief SignalPlotWidgetQwt::setZoomMode Changes Zoom-mode oder does a zoom reset.
 * @param zMode
 */
void BasePlotWidgetQwt::setZoomMode(PlotZoomMode zMode)
{

    if( zMode == ZOOM_RESET )
    {
       qwtPlot->setAxisAutoScale(QwtPlot::yLeft);
       qwtPlot->setAxisAutoScale(QwtPlot::xBottom);

       // inform linked plots (signalprocessingeditor.cpp)
       emit currentToolChanged(zMode);
       qwtPlot->replot();
       return;

    }
    else if(zMode == DATA_CURSOR)
    {
        toggleDataCursor();
        // inform linked plots (signalprocessingeditor.cpp)
        emit currentToolChanged(zMode);
        qwtPlot->replot();
        return;
    }

    z_panner->setEnabled(false);
    z_rectZoomer->setEnabled(false);

    z_mag->setEnabled(true);
    z_mag_x->setEnabled(true);
    z_mag_y->setEnabled(true);

    avgTool->setEnabled(false);
    fitTool->setEnabled(false);

    // always deactivate plotAnalysisTool
    plotAnalysisTool->setEnabled(false);
    toolAction_PlotAnalysisTool->setChecked(false);

    if(zMode == PLOT_PAN)
    {
        z_panner->setEnabled(true);
        zmode = zMode;
    }
    else if( zMode == ZOOM_RECT )
    {
        z_rectZoomer->setEnabled(true);
        zmode = zMode;
    }
    else if( zMode == AVG_RECT)
    {
        avgTool->setEnabled(true);
        zmode = zMode;
    }
    else if( zMode == FIT_RECT)
    {
        fitTool->setEnabled(true);
        zmode = zMode;
    }

    qwtPlot->replot();

    // inform linked plots (signalprocessingeditor.cpp)
    emit currentToolChanged(zmode);
}


/**
 * @brief BasePlotWidgetQwt::resizeEvent
 * @param event
 */
void BasePlotWidgetQwt::resizeEvent(QResizeEvent *event)
{
    avgTool->onPlotResized(event);
    fitTool->onPlotResized(event);
    QWidget::resizeEvent(event);
}


void BasePlotWidgetQwt::keyPressEvent(QKeyEvent *event)
{
    if(!selectedCurve || !dataCursorEnabled || dataCursorSampleIndex <= -1)
        return;

    int newPos;
    if(event->key() == Qt::Key_Right)
    {
        newPos = dataCursorSampleIndex +1;
    }
    else if(event->key() == Qt::Key_Left)
    {
        newPos = dataCursorSampleIndex -1;
    }

    if(newPos > -1 && newPos <  selectedCurve->dataSize())
    {
        dataCursorSampleIndex = newPos;
        setDataCursorPosition(selectedCurve->sample(newPos));
    }
}


void BasePlotWidgetQwt::toggleDataCursor()
{
    // invert signal
    dataCursorEnabled = !dataCursorEnabled;

    pointMarker->setVisible(dataCursorEnabled);
    z_dataCursor->setEnabled(dataCursorEnabled);

    // bottom box
    dataCursorColorLabel->setVisible(dataCursorEnabled);
    dataCursorTextLabel->setVisible(dataCursorEnabled);
}


/************************
 *  PLOT PROTECTED SLOTS  *
 ************************/
/**
 * @brief BasePlotWidgetQwt::onXscaleChanged private slot, handles
 * x-scale-engine signal
 */
void BasePlotWidgetQwt::onXscaleChanged()
{
    if(linkXscale)
    {
        QwtInterval i =  qwtPlot->axisInterval(QwtPlot::xBottom);
        emit xViewChanged(i.minValue(), i.maxValue());
    }
}


/**
 * @brief BasePlotWidgetQwt::onYscaleChanged private slot, handles
 * y-scale-engine signal
 */
void BasePlotWidgetQwt::onYscaleChanged()
{
    if(linkYscale)
    {
        QwtInterval i = qwtPlot->axisInterval(QwtPlot::yLeft);
        emit yViewChanged(i.minValue(), i.maxValue());
    }
}


/**
 * @brief BasePlotWidgetQwt::onToolActionTriggered This
 * slot is executed when any of the tool actions has been triggered.
 * Updates cursor appearance and behavior
 */
void BasePlotWidgetQwt::onToolActionTriggered()
{
    for(int i = 0; i < listPlotAnalysisTools.size(); i++)
        listPlotAnalysisTools.at(i)->setEnabled(false);

    PlotZoomMode zMode;

    if(QObject::sender() == toolAction_PlotPan)
    {
        toolAction_PlotPan->setChecked(true);
        zMode = PLOT_PAN;
    }
    else if(QObject::sender() == toolAction_DataCursor)
    {
        // toggle button independent of other buttons
        toolAction_DataCursor->setChecked(!dataCursorEnabled);
        zMode = DATA_CURSOR;

    }
    else if(QObject::sender() == toolAction_RectZoom)
    {
        toolAction_RectZoom->setChecked(true);
        zMode = ZOOM_RECT;
    }
    else if(QObject::sender() == toolAction_AvgTool)
    {
        toolAction_AvgTool->setChecked(true);
        zMode = AVG_RECT;
    }
    else if(QObject::sender() == toolAction_FitTool)
    {
        toolAction_FitTool->setChecked(true);
        zMode = FIT_RECT;
    }
    else if(QObject::sender() == toolAction_ZoomReset)
    {
        zMode = ZOOM_RESET;
    }

    setZoomMode(zMode);
}


void BasePlotWidgetQwt::onActionPlotAnalysisTool()
{
    // get new state (is automatically changed when clicking the button)
    bool is_checked = toolAction_PlotAnalysisTool->isChecked();

    if(is_checked)
    {
        plotAnalysisTool->setEnabled(true);

        z_panner->setEnabled(false);
        z_rectZoomer->setEnabled(false);

        z_mag->setEnabled(true);
        z_mag_x->setEnabled(true);
        z_mag_y->setEnabled(true);

        avgTool->setEnabled(false);
        fitTool->setEnabled(false);

    }
    else
    {
        plotAnalysisTool->setEnabled(false);
        setZoomMode(PLOT_PAN);
    }
}



/*********************************
 *  CONTEXT MENU: PRIVATE SLOTS  *
 *********************************/

void BasePlotWidgetQwt::onContextMenuEvent(QPoint pos)
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

    menu.exec( QCursor::pos());
}


/**
 * @brief BasePlotWidgetQwt::onShowDataTable create a new table plot if it has not been created yet
 */
void BasePlotWidgetQwt::onShowDataTable()
{
    if(dataTableWindow == NULL)
    {
        dataTableWindow = new DataTableWidget(&curves);
        dataTableWindow->setXAxisNonTimeType(xAxisNonTimeType);
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


/**
 * @brief BasePlotWidgetQwt::onActionShowLegend This slot is executed when the
 * actionShowLegend action has been toggled. It changes the visibility of the legend item.
 * @param show toggled value
 */
void BasePlotWidgetQwt::onActionShowLegend(bool show)
{
    legendItem->setVisible(show);
    qwtPlot->replot();
}



/**************************************
 *  TOOL ACTIONS: DATA CURSOR TOOL    *
 **************************************/

/**
 * @brief BasePlotWidgetQwt::onDataCursorSelection sets the marker position to closest data sample
 * @param point clicked position (of QwtPlotPicker member)
 */
void BasePlotWidgetQwt::onDataCursorSelection(const QPoint &point)
{
    if(!dataCursorEnabled)
        return;

    if(curves.size()>0)
    {
        bool res = selectClosestCurve(point);
        if(!res || !selectedCurve)
            return;

        int x = selectedCurve->closestPoint(point);
        dataCursorSampleIndex = x;
        QPointF np = selectedCurve->sample(x);
        pointMarker->setVisible(true);
        setDataCursorPosition(np);
    }
}


void BasePlotWidgetQwt::onChangeScaleTypeLogX(bool log)
{
    if(log)
        qwtPlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine);
    else
        qwtPlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);

    qwtPlot->setAxisAutoScale(QwtPlot::xBottom);
    z_mag->setLogRescaling(log,QwtPlot::xBottom);
    z_mag_x->setLogRescaling(log,QwtPlot::xBottom);
    z_mag_y->setLogRescaling(log,QwtPlot::xBottom);

    qwtPlot->replot();
}


void BasePlotWidgetQwt::onChangeScaleTypeLogY(bool log)
{
    if(log)
        qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new CustomLogScaleEngine);
     //   qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine);
    else
        qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);

    qwtPlot->setAxisAutoScale(QwtPlot::yLeft);

    z_mag->setLogRescaling(log,QwtPlot::yLeft);
    z_mag_x->setLogRescaling(log,QwtPlot::yLeft);
    z_mag_y->setLogRescaling(log,QwtPlot::yLeft);

    qwtPlot->replot();
}


/**
 * @brief BasePlotWidgetQwt::onPlotTypeActionTriggerd this slot
 * is executed when the user has triggered an action from the "plot type"
 * context menu.
 * @param action current action
 */
void BasePlotWidgetQwt::onPlotTypeActionTriggerd(QAction *action)
{
    /*if(!action)
    {
        QList<QAction*> actions = plotTypeActions->actions();
        for(int i = 0; i < actions.size(); i++)
        {
            if(actions[i]->isChecked())
            {
                action = actions[i];
                break;
            }
        }
    }

    // LINE + CROSSES
    if(action == actionShowDataPoints)
    {
        setPlotType(PlotType::LINE_CROSSES);
    }
    // LINE
    else if(action == actionShowDataLine)
    {
        setPlotType(PlotType::LINE);
    }
    // DOTS SMALL
    else if(action == actionShowDataDotsSize1)
    {
        setPlotType(PlotType::DOTS_SMALL);
    }
    // DOTS MEDIUM
    else if(action == actionShowDataDotsSize2)
    {
        setPlotType(PlotType::DOTS_MEDIUM);
    }
    // DOTS LARGE
    else if(action == actionShowDataDotsSize3)
    {
        setPlotType(PlotType::DOTS_LARGE);
    }*/

    // LINE + CROSSES
    if(action == actionShowDataPoints)
        plotType = PlotType::LINE_CROSSES;
    // LINE
    else if(action == actionShowDataLine)
        plotType = PlotType::LINE;
    // DOTS SMALL
    else if(action == actionShowDataDotsSize1)
        plotType = PlotType::DOTS_SMALL;
    // DOTS MEDIUM
    else if(action == actionShowDataDotsSize2)
        plotType = PlotType::DOTS_MEDIUM;
    // DOTS LARGE
    else if(action == actionShowDataDotsSize3)
        plotType = PlotType::DOTS_LARGE;

    emit plotTypeChanged(plotType);

    updateToPlotType();
}


void BasePlotWidgetQwt::setPlotType(PlotType type)
{
    plotType = type;

    switch(type)
    {
        case PlotType::LINE_CROSSES: actionShowDataPoints->setChecked(true); break;
        case PlotType::LINE: actionShowDataLine->setChecked(true); break;
        case PlotType::DOTS_SMALL: actionShowDataDotsSize1->setChecked(true); break;
        case PlotType::DOTS_MEDIUM: actionShowDataDotsSize2->setChecked(true); break;
        case PlotType::DOTS_LARGE: actionShowDataDotsSize3->setChecked(true); break;
    }

    emit plotTypeChanged(type);

    updateToPlotType();
}


void BasePlotWidgetQwt::updateToPlotType()
{
    switch(plotType)
    {
        case PlotType::LINE_CROSSES:
        {
            for(int i = 0; i < curves.size(); i++)
            {
                BasePlotCurve* curve = curves[i];

                // curve style: line
                curve->setStyle(BasePlotCurve::Lines);

                // skip curve if style is fixed
                if(curve->isFixedStyle())
                    continue;

                // data point symbol: cross
                QwtSymbol* s = new QwtSymbol(QwtSymbol::Cross);
                s->setSize(12);
                s->setColor(curve->pen().color());
                curve->setSymbol(s);

                // pen style: normal width
                QPen p = curve->pen();
                p.setWidthF(curveLineWidth);
                curve->setPen(p);
            }
        }
    break;
        case PlotType::LINE:
        {
            for(int i = 0; i < curves.size(); i++)
            {
                BasePlotCurve* curve = curves[i];

                // skip curve if style is fixed
                if(curve->isFixedStyle())
                    continue;

                // curve style: line
                curve->setStyle(BasePlotCurve::Lines);

                // data point symbol: no symbol
                curve->setSymbol(0);

                // pen style: normal width
                QPen p = curve->pen();
                p.setWidthF(curveLineWidth);
                curve->setPen(p);
            }
        }
    break;
        case PlotType::DOTS_SMALL:
        {
            for(int i = 0; i < curves.size(); i++)
            {
                BasePlotCurve* curve = curves[i];

                // skip curve if style is fixed
                if(curve->isFixedStyle())
                    continue;

                // curve style: dots
                curve->setStyle(BasePlotCurve::Dots);

                // data point symbol: no symbol
                curve->setSymbol(0);

                // pen style: normal width
                QPen p = curve->pen();
                p.setWidthF(curveLineWidth);
                curve->setPen(p);
            }
        }
    break;
        case PlotType::DOTS_MEDIUM:
        {
            for(int i = 0; i < curves.size(); i++)
            {
                BasePlotCurve* curve = curves[i];

                // skip curve if style is fixed
                if(curve->isFixedStyle())
                    continue;

                // curve style: dots
                curve->setStyle(BasePlotCurve::Dots);

                // data point symbol: no symbol
                curve->setSymbol(0);

                // pen style: medium width
                QPen p = curve->pen();
                p.setWidthF(3.0);
                curve->setPen(p);
            }
        }
    break;
        case PlotType::DOTS_LARGE:
        {
            for(int i = 0; i < curves.size(); i++)
            {
                BasePlotCurve* curve = curves[i];

                // skip curve if style is fixed
                if(curve->isFixedStyle())
                    continue;

                // curve style: dots
                curve->setStyle(BasePlotCurve::Dots);

                // data point symbol: no symbol
                curve->setSymbol(0);

                // pen style: large width
                QPen p = curve->pen();
                p.setWidthF(5.5);
                curve->setPen(p);
            }
        }
    break;
    }
    qwtPlot->replot();
}


void BasePlotWidgetQwt::setPlotLabelText(QString text)
{
    plotlabelText->setText(text);
    plotLabel->setText(*plotlabelText);
}


void BasePlotWidgetQwt::setPlotLabelAlignment(int alignment)
{
    plotlabelText->setRenderFlags(alignment);
}


void BasePlotWidgetQwt::plotTextLabelVisible(bool visible)
{
    if(visible)
        plotLabel->attach(qwtPlot);
    else
        plotLabel->detach();
}


void BasePlotWidgetQwt::setDataTableRunName(QString name)
{
    dataTableRunName = name;
}


void BasePlotWidgetQwt::setDataTableToolName(QString name)
{
    dataTableToolName = name;
}


void BasePlotWidgetQwt::setXAxisNonTimeType(bool nonTime)
{
    xAxisNonTimeType = nonTime;
}
