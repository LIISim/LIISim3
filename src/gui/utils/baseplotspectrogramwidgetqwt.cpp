#include "baseplotspectrogramwidgetqwt.h"

#include <QSplitter>

#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>

#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_picker_machine.h>

#include "colormapspectrogram.h"

#include "customQwtPlot/customplotpicker.h"


BasePlotSpectrogramWidgetQwt::BasePlotSpectrogramWidgetQwt(QWidget *parent) : QWidget(parent)
{
    // main plot
    qwtPlot = new QwtPlot(this);

    qwtPlot->setAxisAutoScale(QwtPlot::xTop, true);
    qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);
    qwtPlot->setAxisAutoScale(QwtPlot::yLeft,  true);

    // setup axes font
    axisFont.setPointSize(8);

    // default axis scaling
    xmin = 0.0;
    xmax = 1.0;

    ymin = 0.0;
    ymax = 1.0;

    zmin = -1.0;
    zmax = -1.0;

    // add option to magnify
    QwtPlotMagnifier *magnifier = new QwtPlotMagnifier( qwtPlot->canvas() );
    magnifier->setAxisEnabled( QwtPlot::yRight, false); // disable for colormap

    // add option to panv
    QwtPlotPanner *panner = new QwtPlotPanner( qwtPlot->canvas() );
    panner->setAxisEnabled( QwtPlot::yRight, false); // disable for colormap


    // data cursor
    CustomPlotPicker *z_dataCursor = new CustomPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
                                                           QwtPlotPicker::CrossRubberBand,
                                                           QwtPicker::AlwaysOn,
                                                           qwtPlot->canvas());
    // activate z cursor
    z_dataCursor->setZMode();

    z_dataCursor->setStateMachine( new QwtPickerDragPointMachine() );
    z_dataCursor->setRubberBandPen( QColor( Qt::gray ) );
    z_dataCursor->setRubberBand( QwtPicker::CrossRubberBand);
    z_dataCursor->setTrackerPen( QColor( Qt::black ));

    connect(z_dataCursor,SIGNAL(appended(QPoint)),SLOT(onDataCursorSelection(QPoint)));

    // zoom in/out with the wheel
    QwtPlotMagnifier *z_mag = new QwtPlotMagnifier( qwtPlot->canvas() );
    QwtPlotMagnifier *z_mag_x = new QwtPlotMagnifier( qwtPlot->canvas() );
    QwtPlotMagnifier *z_mag_y = new QwtPlotMagnifier( qwtPlot->canvas() );


    z_mag->setAxisEnabled(QwtPlot::yRight, false);

    z_mag_x->setWheelModifiers(Qt::ControlModifier);
    z_mag_x->setAxisEnabled(QwtPlot::xBottom, true);
    z_mag_x->setAxisEnabled(QwtPlot::yLeft,false);
    z_mag_x->setAxisEnabled(QwtPlot::yRight, false);

    z_mag_y->setWheelModifiers(Qt::ShiftModifier);
    z_mag_y->setAxisEnabled(QwtPlot::xBottom,false);
    z_mag_y->setAxisEnabled(QwtPlot::yLeft,true);
    z_mag_y->setAxisEnabled(QwtPlot::yRight, false);

    // disable the "right click hold" zoom
    z_mag->setMouseButton(Qt::NoButton, Qt::NoModifier);
    z_mag_x->setMouseButton(Qt::NoButton, Qt::NoModifier);
    z_mag_y->setMouseButton(Qt::NoButton, Qt::NoModifier);

    // layout
    layMain = new QGridLayout;
    this->setLayout(layMain);
    layMain->setMargin(2);

    // main horizontal splitter (contains plot)
    QSplitter* mainHSplit = new QSplitter(Qt::Horizontal);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainHSplit->setSizePolicy(sizePolicy);

    mainHSplit->addWidget(qwtPlot);

    layMain->addWidget(mainHSplit,0,0,1,2);
}


BasePlotSpectrogramWidgetQwt::~BasePlotSpectrogramWidgetQwt()
{
}



/**
 * @brief BasePlotSpectrogramWidgetQwt::initMatrix
 * @param rows
 * @param columns
 * @return last index of qwtplotitem / raster data
 */
int BasePlotSpectrogramWidgetQwt::createMatrixPlot(int rows, int columns)
{
    // create new spectrogram
    QwtPlotSpectrogram *plotItem = new QwtPlotSpectrogram;
    plotItem->setAlpha(150);
    plotItem->attach(qwtPlot);

    // settings
    plotItem->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);

    // define colormap (cannot be the same object for plot and axis -> object would be deleted twice -> crash)
    plotItem->setColorMap(new ColorMapSpectrogram);


    qwtPlotItems.append(plotItem);


    // fill matrix
    initMatrix(qwtPlotItems.size()-1, rows, columns);

   // return size -1 == last idx
    return qwtPlotItems.size()-1;
}


void BasePlotSpectrogramWidgetQwt::initMatrix(int idx, int rows, int columns)
{
    // create plot item if idx does not exist
    if(qwtPlotItems.size() < idx)
        createMatrixPlot(rows, columns);

    if(idx < rasterData.size())
    {
        rasterData.removeAt(idx);
    }

    QwtMatrixRasterData *data = new QwtMatrixRasterData;

    QVector<double> values;

    for (uint i = 0; i < rows; i++)
        for ( uint k = 0; k < columns; k++ )
            values.append(-1.0);
            //data->setValue(i, k, -1.0);

   data->setValueMatrix(values, columns);

   rasterData.insert(idx, data);

   qwtPlotItems.at(idx)->setData(data);
}


/**
 * @brief BasePlotSpectrogramWidgetQwt::getMatrixRows
 * @param idx -1 if matrix does not exist, 0 if matrix is empty
 * @return
 */
int BasePlotSpectrogramWidgetQwt::getMatrixRows(int idx)
{
    if(rasterData.size() < idx)
        return -1;

    return rasterData.at(idx)->numRows();
}


int BasePlotSpectrogramWidgetQwt::getMatrixColumns(int idx)
{
    if(rasterData.size() < idx)
        return -1;

    return rasterData.at(idx)->numColumns();
}



void BasePlotSpectrogramWidgetQwt::setAlpha(int idx, int alpha)
{
    qwtPlotItems.at(idx)->setAlpha(alpha);
}


void BasePlotSpectrogramWidgetQwt::setValue(int idx, int row, int col, double value)
{
    if(rasterData.size() < idx)
        return;

    rasterData.at(idx)->setValue(row, col, value);
}


void BasePlotSpectrogramWidgetQwt::setRange(double xi, double xa, double yi, double ya)
{
    xmin = xi;
    xmax = xa;

    ymin = yi;
    ymax = ya;
}


void BasePlotSpectrogramWidgetQwt::setZRange(double zi, double za)
{
    if(zi == za)
    {
        zmin = zi;
        zmax = za + 1.0; // avoid equal borders
    }
    else if( zi > za)
    {
        zmin = za;
        zmax = zi;
    }
    else
    {
        zmin = zi;
        zmax = za;
    }
}


void BasePlotSpectrogramWidgetQwt::setZRangeAuto(int idx)
{

    QVector<double> values = rasterData.at(idx)->valueMatrix();

    zmin = *std::min_element(values.constBegin(), values.constEnd());
    zmax = *std::max_element(values.constBegin(), values.constEnd());

    //emit rangeChangedZ(zmin, zmax);
}


void BasePlotSpectrogramWidgetQwt::updateData(int idx)
{
    // check if idx is valid
    if(rasterData.size() < idx)
        return;

    if(zmin == -1.0 && zmax == -1.0)
    {
        setZRangeAuto(idx);
    }

    emit rangeChangedZ(zmin, zmax);

    QwtInterval zInterval = QwtInterval(zmin, zmax, QwtInterval::ExcludeMinimum);

    rasterData.at(idx)->setInterval( Qt::XAxis, QwtInterval(xmin, xmax, QwtInterval::IncludeBorders ) );
    rasterData.at(idx)->setInterval( Qt::YAxis, QwtInterval(ymin, ymax, QwtInterval::IncludeBorders ) );
    rasterData.at(idx)->setInterval( Qt::ZAxis, zInterval);

    // add contour lines
//    QList<double> contourLevels;
//    for ( double level = zmin; level < zmax; level *= 10.0 )
//    {
//        contourLevels.append(level);
//    }
//    qwtPlotItems.at(idx)->setContourLevels(contourLevels);

    //qwtPlotItems.at(idx)->setDefaultContourPen(Qt::black, 1.0, Qt::SolidLine);
    //qwtPlotItems.at(idx)->setConrecFlag(QwtRasterData::ConrecFlag::IgnoreOutOfRange, true);


    // add colormap on the right axis
    QwtLogScaleEngine *logScaleEngine = new QwtLogScaleEngine;
    //QwtLinearScaleEngine *linearScaleEngine = new QwtLinearScaleEngine;



    qwtPlot->setAxisScaleEngine(QwtPlot::yRight, logScaleEngine);


    QwtScaleWidget *rightAxis = qwtPlot->axisWidget(QwtPlot::yRight);
    rightAxis->setColorBarEnabled(true);
    rightAxis->setColorBarWidth(40);
    rightAxis->setColorMap(zInterval, new ColorMapSpectrogram);


    qwtPlot->setAxisScale(QwtPlot::yRight,
                          zInterval.minValue(),
                          zInterval.maxValue(),
                          0.0);
                          //(zmax-zmin)/10.0);

    qwtPlot->enableAxis(QwtPlot::yRight);

    //qwtPlot->setAxisAutoScale(QwtPlot::xBottom);
    //qwtPlot->setAxisAutoScale(QwtPlot::yLeft);

    qwtPlot->replot();
}



/**
 * @brief BasePlotSpectrogramWidgetQwt::setPlotAxisTitle set axis titles
 * @param xTitle title for x-axis
 * @param yTitle title for y-axis
 * @param zTitle title for colormap
 */
void BasePlotSpectrogramWidgetQwt::setPlotAxisTitles(const QString &xTitle, const QString &yTitle, const QString &zTitle)
{
    QwtText title1(xTitle);
    title1.setFont(axisFont);
    qwtPlot->setAxisTitle(QwtPlot::xBottom, title1);


    QwtText title2(yTitle);
    title2.setFont(axisFont);
    qwtPlot->setAxisTitle(QwtPlot::yLeft, title2);

    QwtText title3(zTitle);
    title3.setFont(axisFont);
    qwtPlot->setAxisTitle(QwtPlot::yRight, title3);

}


/**************************************
 *  TOOL ACTIONS: DATA CURSOR TOOL    *
 **************************************/

/**
 * @brief BasePlotSpectrogramWidgetQwt::onDataCursorSelection sets the marker position to closest data sample
 * @param point clicked position (of QwtPlotPicker member)
 */
void BasePlotSpectrogramWidgetQwt::onDataCursorSelection(const QPoint &point)
{
    //qDebug() << point.x() << point.y();
    //qDebug() << qwtPlot->invTransform(2,point.x()) << qwtPlot->invTransform(0,point.y());

    double x = qwtPlot->invTransform(2,point.x());
    double y = qwtPlot->invTransform(0,point.y());

    emit dataCursorSelected(x,y);
}
