#include "atooltemperaturefit.h"

#include "../../core.h"

#include "../../utils/customQwtPlot/baseplotcurve.h"
#include <qwt_symbol.h>

#include <QHeaderView>
#include <QShortcut>
#include <QItemSelectionModel>

#include "../../utils/numberlineedit.h"

#include "../../calculations/temperature.h"
#include "../../signal/spectrum.h"

#include "../../signal/processing/processingchain.h"
#include "../../signal/processing/temperatureprocessingchain.h"
#include "../../signal/processing/plugins/temperaturecalculator.h"


const QString AToolTemperatureFit::identifier_splitterLeftV = "temperature_fit_splitter_left_v";
const QString AToolTemperatureFit::identifier_splitterRightV = "temperature_fit_splitter_right_v";
const QString AToolTemperatureFit::identifier_bottomSplitterH = "temperature_fit_splitter_h";
const QString AToolTemperatureFit::identifier_splitterMiddleH = "temperature_fit_splitter_middle_h";


AToolTemperatureFit::AToolTemperatureFit(QWidget *parent) : SignalPlotTool(parent)
{
    setObjectName("AT_TF");
    plotter->setObjectName("AT_TF_PLOT");
    m_title = "Temperature Fit";
    m_iconLocation = Core::rootDir + "resources/icons/temperature_warm.png";

    // no iteration selection
    it_selection = -1;
    tempChannelID = -1;

    statAutoX = true;
    statAutoZ = true;
    statResX  = 500;
    statResY  = 500;

    currentMPoint = NULL;

    // TODO

    // switch to temperature plot / implement visibility of stype combobox
    // allow only single Mrun selection?

    // connect data cursor tool (arrow keys) to selection function
    connect(plotter,SIGNAL(dataSampleMarkerMoved(const QString, const QColor, const QPointF)),
            this,SLOT(onPointSelected(const QString, const QColor, const QPointF)));



    /******************
     *  TOP LEFT BOX
     */

    // add left vertically splitted box (two plots)
    verticalSplitterLeft = new QSplitter(Qt::Vertical);
    //verticalSplitterLeft->setContentsMargins(5,5,2,0);
    //verticalSplitterLeft->setHandleWidth(10);

    // remove plotter from horizontal splitter
    // and add it to vertical splitter (left box)

    plotterWidget->setParent(0);
    verticalSplitterLeft->addWidget(plotterWidget);
    //plotter->setEnabledXViewLink(true);
    //connect(plotter,SIGNAL(xViewChanged(double,double)),SLOT(onXViewChanged(double,double)));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *widgetLayout = new QVBoxLayout;
    widgetLayout->setMargin(0);
    widget->setLayout(widgetLayout);
    QHBoxLayout *widgetMRunTChannel = new QHBoxLayout;
    widgetMRunTChannel->setMargin(0);
    widgetLayout->addLayout(widgetMRunTChannel);
    widgetLayout->setAlignment(widgetMRunTChannel, Qt::AlignLeft);

    QLabel *labelMRunList = new QLabel("MRun: ");
    comboboxMRun = new QComboBox(this);
    comboboxMRun->setMinimumWidth(200);
    widgetMRunTChannel->addWidget(labelMRunList);
    widgetMRunTChannel->addWidget(comboboxMRun);

    verticalSplitterLeft->addWidget(widget);

    // tab widget
    fitResultTabWidget = new QTabWidget();

    connect(comboboxMRun, SIGNAL(currentIndexChanged(int)), SLOT(onComboboxMRunIndexChanged()));

    connect(fitResultTabWidget, SIGNAL(currentChanged(int)), SLOT(onFitResultTabChanged(int)));

    // 1) fit result plot
    resultPlot = new BasePlotWidgetQwt;
    resultPlot->setZoomMode(BasePlotWidgetQwt::PLOT_PAN);
    resultPlot->setPlotTitle("Fit Results");
    resultPlot->setDataTableToolName(m_title);

    connect(resultPlot, SIGNAL(plotTypeChanged(BasePlotWidgetQwt::PlotType)), SLOT(onPlotTypeChanged(BasePlotWidgetQwt::PlotType)));

    QWidget *widgetFitResult = new QWidget(this);
    QVBoxLayout *widgetFRMainLayout = new QVBoxLayout;
    widgetFRMainLayout->setMargin(0);

    widgetFitResult->setLayout(widgetFRMainLayout);

    QHBoxLayout *widgetFRSettingsLayout = new QHBoxLayout;
    widgetFRSettingsLayout->setMargin(10);
    widgetFRMainLayout->addLayout(widgetFRSettingsLayout);
    widgetFRMainLayout->setAlignment(widgetFRSettingsLayout, Qt::AlignTop);

    QHBoxLayout *widgetFRSettingsLeftLayout = new QHBoxLayout;
    widgetFRSettingsLeftLayout->setMargin(0);
    widgetFRSettingsLayout->addLayout(widgetFRSettingsLeftLayout);
    widgetFRSettingsLayout->setAlignment(widgetFRSettingsLeftLayout, Qt::AlignLeft);
    widgetFRSettingsLeftLayout->addSpacing(10);

    widgetFRMainLayout->addWidget(resultPlot, 2);

    fitResultTabWidget->addTab(widgetFitResult, tr("Fit Results"));
    fitResultTabWidget->setObjectName("AT_TF_RESULT_TAB");


    // 2) statistics plot
    statisticsPlot = new BasePlotSpectrogramWidgetQwt;

    statisticsPlot->setPlotAxisTitles("Scaling factor C/ -",
                                      "Temperature / K",
                                      "Chisquare / -");

    int midx  = statisticsPlot->createMatrixPlot(statResY, statResX);


    connect(statisticsPlot, SIGNAL(dataCursorSelected(double,double)), SLOT(onStatisticsPlotClicked(double,double)));

    //statisticsPlot->setAlpha(0, 150);

    curve_stats = new BasePlotCurve("Fit Stats");
    curve_stats->attach(statisticsPlot->qwtPlot);
    curve_stats_init = new BasePlotCurve("Fit Stats (Initialization)");
    curve_stats_init->attach(statisticsPlot->qwtPlot);

    QWidget *widgetStatistics = new QWidget(this);
    QHBoxLayout *mainLayoutWidgetStat = new QHBoxLayout;
    mainLayoutWidgetStat->setMargin(10);
    mainLayoutWidgetStat->addWidget(statisticsPlot, 2);
    widgetStatistics->setLayout(mainLayoutWidgetStat);


    QVBoxLayout *layoutStatSettings = new QVBoxLayout;
    layoutStatSettings->setMargin(0);
    mainLayoutWidgetStat->addLayout(layoutStatSettings);
    mainLayoutWidgetStat->setAlignment(layoutStatSettings, Qt::AlignTop | Qt::AlignRight);

    fitResultTabWidget->addTab(widgetStatistics, tr("Chi-squared function (2D)"));

    // 3) covariance plot

    covarPlot = new BasePlotWidgetQwt;
    covarPlot->setZoomMode(BasePlotWidgetQwt::PLOT_PAN);
    covarPlot->setPlotTitle("Covariance Matrix Elements");
    covarPlot->setDataTableToolName(m_title);


    // change plot type and scale
    covarPlot->setPlotType(BasePlotWidgetQwt::PlotType::DOTS_SMALL);

    covarPlot->setXLogScale(true);
    covarPlot->setYLogScale(true);

    //covarPlot->setXView(-1.0, 1.0);

    // link to signal plot (top left)
    //covarPlot->setEnabledXViewLink(true);
    connect(covarPlot,SIGNAL(xViewChanged(double,double)),SLOT(onXViewChanged(double,double)));

    // show this only in full version
#ifdef LIISIM_FULL
    fitResultTabWidget->addTab(covarPlot, tr("Covariance"));
#endif

    // switch to statistics by default
    fitResultTabWidget->setCurrentIndex(1);

    widget->layout()->addWidget(fitResultTabWidget);


    //verticalSplitterLeft->addWidget(resultPlot);
    //verticalSplitterLeft->addWidget(fitResultTabWidget);


    /******************
     *  RIGHT BOX
     */

    // add right vertically splitted box
    verticalSplitterRight = new QSplitter(Qt::Vertical);
    //verticalSplitterRight->setContentsMargins(5,5,2,0);

    /******************
     *  TOP RIGHT BOX
     */

    QWidget *widgetFitPlot = new QWidget(this);
    QVBoxLayout *widgetFitPlotLayout = new QVBoxLayout;
    widgetFitPlotLayout->setMargin(0);
    widgetFitPlot->setLayout(widgetFitPlotLayout);

    // buttons (top)

    QAction* clearPlotAction = new QAction("clear plot", this);
    QToolBar* toolbarFit = new QToolBar();
    toolbarFit->addAction(clearPlotAction);

    connect(clearPlotAction,
            SIGNAL(triggered(bool)),
            SLOT(onClearPlotButtonReleased()));

    widgetFitPlotLayout->addWidget(toolbarFit);
    widgetFitPlotLayout->setAlignment(toolbarFit, Qt::AlignRight | Qt::AlignBottom);


    // add visualization plot
    fitPlot = new BasePlotWidgetQwt;
    fitPlot->setObjectName("AT_TF_FIT_PLOT");
    fitPlot->setPlotTitle("Fit Visualization (Spectrum)");
    fitPlot->setZoomMode(BasePlotWidgetQwt::PLOT_PAN);    
    fitPlot->setPlotAxisTitles("Wavelength / nm", "Intensity / arb. u.");
    fitPlot->setMaxLegendColumns(1);
    fitPlot->setDataTableToolName(m_title);
    fitPlot->setPlotLabelText("<b>Temperature fit visualization:</b><br>\
    1) Calculate temperature-trace in SignalProcessing<br>\
       (TemperatureCalculator: make sure \"Spectrum\"-Method is used)<br>\
    2) Select MRun (only one), which will be visible in left plot<br>\
    3) Use data cursor on data point of interest<br>\
    For more information please see the manual.");
    fitPlot->plotTextLabelVisible(true);



    widgetFitPlotLayout->addWidget(fitPlot);

    verticalSplitterRight->addWidget(widgetFitPlot);


    /******************
     *  BOTTOM RIGHT BOX
     */

    // split bottom right box into two
    bottomRightSplitter = new QSplitter(Qt::Horizontal);


    /******************
     *  SETTINGS BOX
     */

    QGridLayout *settingsLayout = new QGridLayout;
    settingsLayout->setMargin(0);

    QWidget *settingsLayoutDummy = new QWidget();
    settingsLayoutDummy->setLayout(settingsLayout);


    QPushButton * resetPlotButton = new QPushButton("clear plot");
    resetPlotButton->setMaximumWidth(60);

    connect(resetPlotButton,
            SIGNAL(released()),
            SLOT(onResetPlotButtonReleased()));


    /******************
     *  SETTINGS TABLE
     */

    // Combobox "Results type"
    cbResultsType = new QComboBox();
    cbResultsType->addItem("Iterations");
    cbResultsType->addItem("Final result");

    widgetFRSettingsLeftLayout->addWidget(new QLabel("Results Type:"));
    widgetFRSettingsLeftLayout->addWidget(cbResultsType);

    connect(cbResultsType,
            SIGNAL(currentIndexChanged(int)),
            SLOT(onCbResultsTypeChanged(int)));

    // Combobox "Show results"
    cbShowResults = new QComboBox();
    cbShowResults->addItem("Temperature");
    cbShowResults->addItem("Scaling factor C");
    cbShowResults->addItem("Chisquare");
    cbShowResults->addItem("Lambda");

    widgetFRSettingsLeftLayout->addWidget(new QLabel("Show:"));
    widgetFRSettingsLeftLayout->addWidget(cbShowResults);

    connect(cbShowResults,
            SIGNAL(currentIndexChanged(int)),
            SLOT(onCbShowResultsChanged(int)));

    // Combobox "Temperature channel"
    cbTChannel = new QComboBox();

    // get available channels
    cbTChannel->addItem("N/A");
    cbTChannel->setEnabled(false);

    QLabel *labelAvailableTChannel = new QLabel("Currently shown Temperature-Trace: ");
    widgetMRunTChannel->addSpacing(10);
    widgetMRunTChannel->addWidget(labelAvailableTChannel);
    widgetMRunTChannel->addWidget(cbTChannel);

    connect(cbTChannel,
            SIGNAL(currentIndexChanged(QString)),
            SLOT(onCbTChannelChanged(QString)));

    // checkbox show filter approximation
    checkBoxBPAnalysis = new QCheckBox("Show bandpass analysis");
    checkBoxBPAnalysis->setChecked(false);

    // show bandpass analyis options only for full version
#ifdef LIISIM_FULL
    widgetFRSettingsLeftLayout->addWidget(checkBoxBPAnalysis);
#endif

    widgetFRSettingsLayout->addSpacing(-1);

    widgetFRSettingsLayout->addWidget(resetPlotButton);
    widgetFRSettingsLayout->setAlignment(resetPlotButton, Qt::AlignRight);

    connect(checkBoxBPAnalysis,
            SIGNAL(stateChanged(int)),
            SLOT(onCheckBoxBPAnalysisSelected(int)));


    // input axis scale
    layoutStatSettings->addWidget(new QLabel("Colormap Range (Z-Axis)"), 0, Qt::AlignTop);

    input_zmin = new NumberLineEdit(NumberLineEdit::DOUBLE);
    input_zmax = new NumberLineEdit(NumberLineEdit::DOUBLE);

    QHBoxLayout *layoutColormapInput = new QHBoxLayout;
    layoutColormapInput->setMargin(0);
    layoutStatSettings->addLayout(layoutColormapInput);

    layoutColormapInput->addWidget(input_zmin);
    layoutColormapInput->addWidget(input_zmax);

    connect(input_zmin, SIGNAL(valueChanged()),
                        SLOT(onInputZChanged()));
    connect(input_zmax, SIGNAL(valueChanged()),
                        SLOT(onInputZChanged()));


    // update input fields in case of signal from BasePlotSpectrogramWidgetQwt
    connect(statisticsPlot, SIGNAL(rangeChangedZ(double, double)),
                        SLOT(onRangeChangedZ(double, double)));

    // checkbox autoZScale
    checkBoxStatAutoZ = new QCheckBox("Auto Z-Axis");
    checkBoxStatAutoZ->setChecked(statAutoZ);

    layoutStatSettings->addWidget(checkBoxStatAutoZ);

    connect(checkBoxStatAutoZ, SIGNAL(stateChanged(int)),
                                SLOT(onCheckBoxStatAutoZ(int)));

    QWidget* dummy = new QWidget();
    layoutStatSettings->addWidget(dummy);

    // input x-axis    
    layoutStatSettings->addWidget(new QLabel("X-Axis"));

    //fitSettingsTableWidget->setRowHeight(rowID,rh);

    input_xmin = new NumberLineEdit(NumberLineEdit::DOUBLE);
    input_xmax = new NumberLineEdit(NumberLineEdit::DOUBLE);

    input_xmin->setValue(1E-1);
    input_xmax->setValue(1E2);

    QHBoxLayout *layoutXAxis = new QHBoxLayout;
    layoutXAxis->setMargin(0);
    layoutStatSettings->addLayout(layoutXAxis);

    layoutXAxis->addWidget(input_xmin);
    layoutXAxis->addWidget(input_xmax);

    // checkbox autoXScale
    checkBoxStatAutoX = new QCheckBox("Auto X-Axis");
    checkBoxStatAutoX->setChecked(true);

    layoutStatSettings->addWidget(checkBoxStatAutoX);

    connect(checkBoxStatAutoX, SIGNAL(stateChanged(int)),
                                SLOT(onCheckBoxStatAutoX(int)));

    QWidget* dummy2 = new QWidget();
    layoutStatSettings->addWidget(dummy2);

    // input y-axis
    input_ymin = new NumberLineEdit(NumberLineEdit::DOUBLE);
    input_ymax = new NumberLineEdit(NumberLineEdit::DOUBLE);

    input_ymin->setValue(1200.0);
    input_ymax->setValue(4500.0);

    layoutStatSettings->addWidget(new QLabel("Y-Axis"));
    QHBoxLayout *layoutYAxis = new QHBoxLayout;
    layoutYAxis->setMargin(0);
    layoutStatSettings->addLayout(layoutYAxis);

    layoutYAxis->addWidget(input_ymin);
    layoutYAxis->addWidget(input_ymax);

    // checkbox autoYScale
    checkBoxStatAutoY = new QCheckBox("Auto Y-Axis");
    checkBoxStatAutoY->setChecked(true);

    layoutStatSettings->addWidget(checkBoxStatAutoY);

    connect(checkBoxStatAutoY, SIGNAL(stateChanged(int)),
                                SLOT(onCheckBoxStatAutoY(int)));

    QWidget* dummy3 = new QWidget();
    layoutStatSettings->addWidget(dummy3);


    // input resolution
    input_resolutionX = new NumberLineEdit(NumberLineEdit::INTEGER);
    input_resolutionY = new NumberLineEdit(NumberLineEdit::INTEGER);

    input_resolutionX->setValue(double(statResX));
    input_resolutionY->setValue(double(statResY));

    layoutStatSettings->addWidget(new QLabel("Resolution X/Y"));
    QHBoxLayout *layoutStatResolution = new QHBoxLayout;
    layoutStatResolution->setMargin(0);
    layoutStatSettings->addLayout(layoutStatResolution);

    layoutStatResolution->addWidget(input_resolutionX);
    layoutStatResolution->addWidget(input_resolutionY);

    buttonUpdateStatSettings = new QPushButton("Update Settings");
    layoutStatSettings->addWidget(buttonUpdateStatSettings, 0, Qt::AlignRight);

    connect(buttonUpdateStatSettings, SIGNAL(clicked(bool)), SLOT(onButtonUpdateSettingsClicked()));

    /******************
     *  RESULT TABLE
     */

    // shortcut
    QShortcut *shortcut_selUp = new QShortcut(QKeySequence(Qt::Key_Up), this);
    connect(shortcut_selUp, SIGNAL(activated()), this, SLOT(scrollResultsUp()));

    QShortcut *shortcut_selDown = new QShortcut(QKeySequence(Qt::Key_Down), this);
    connect(shortcut_selDown, SIGNAL(activated()), this, SLOT(scrollResultsDown()));

    fitResultTableWidget = new QTableWidget();
    fitResultTableWidget->setObjectName("AT_TF_FIT_RESULT_TABLE");
    fitResultTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    fitResultTableWidget->setColumnCount(8);
    fitResultTableWidget->setColumnWidth(0,80);
    fitResultTableWidget->setColumnWidth(1,90);
    fitResultTableWidget->setColumnWidth(2,90);
    fitResultTableWidget->setColumnWidth(3,90);
    fitResultTableWidget->setColumnWidth(4,90);
    fitResultTableWidget->setColumnWidth(5,90);
    fitResultTableWidget->setColumnWidth(6,90);
    fitResultTableWidget->setColumnWidth(7,90);

    QStringList hHeaderList;

    hHeaderList << "Temperature" << "C" << "Delta T" << "Delta C" << "Chisquare" << "Lambda" << "" << "";

    fitResultTableWidget->setHorizontalHeaderLabels(hHeaderList);

     // make table vertical header clickable
    QHeaderView *vHeader = fitResultTableWidget->verticalHeader();
    connect(vHeader, SIGNAL(sectionClicked(int)), this, SLOT(onCellClicked(int)));
    connect(fitResultTableWidget,SIGNAL(cellClicked(int,int)),SLOT(onCellClicked(int,int)));


    // add widget to general layout

    //bottomRightSplitter->addWidget();

    int w1 = 190;
    int w2 = bottomRightSplitter->width()-w1;

    QList<int> wlist;
    wlist << w1 << w2;
    bottomRightSplitter->setSizes(wlist);


    verticalSplitterRight->addWidget(fitResultTableWidget);

    // change main horizontal splitter size

    horizontalSplitter->addWidget(verticalSplitterLeft);
    horizontalSplitter->addWidget(verticalSplitterRight);

    w1 = 300;
    w2 = horizontalSplitter->width()-w1;

    wlist.clear();
    wlist << w1 << w2;
    horizontalSplitter->setSizes(wlist);

    connect(Core::instance()->guiSettings, SIGNAL(settingsChanged()), SLOT(onGuiSettingsChanged()));

    connect(verticalSplitterLeft, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    connect(verticalSplitterRight, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    connect(bottomRightSplitter, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    connect(horizontalSplitter, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
}


void AToolTemperatureFit::processDataInterval()
{
    if(plot_selection.isNull())
        return;

    // process data
    int chID;
    QVector<double> xData;
    QVector<double> yData;

    double x = plot_selection.x()*1E-9;

    // temporary channel properties
    int wavelength;
    int bandwidth;
    int b2; // rounded half bandwidth

    // show data points in fitPlot
    MRun *run = currentMRun;
    MPoint *mp = currentMPoint;

    TemperatureProcessingChain *tpc = dynamic_cast<TemperatureProcessingChain*>(run->getProcessingChain(Signal::TEMPERATURE));
    TemperatureCalculator *tc = dynamic_cast<TemperatureCalculator*>(tpc->getPlug(tempChannelID-1));
    if(tc)
    {
        Signal::SType tempSType = tc->getInputSignalType();

        BasePlotCurve *curve = new BasePlotCurve(QString("Data at x=%0 ns").arg(x*1E9));

        for (chID = 1; chID <= run->getNoChannels(tempSType); chID++)
        {
            // skip channel if not selected
            //if(!selectedChannelIds().contains(chID))
            //    continue;

            Signal signal = mp->getSignal(chID, tempSType);

            wavelength = run->liiSettings().channels.at(chID-1).wavelength;

            xData.append(wavelength);
            yData.append(signal.at(x));
        }

        // never change appearance
        curve->setFixedStyle(true);

        // curve style: no line
        curve->setStyle(BasePlotCurve::NoCurve);

        // data point symbol: cross
        QwtSymbol* s = new QwtSymbol(QwtSymbol::Cross);
        s->setSize(10);
        s->setColor(curve->pen().color());
        curve->setSymbol(s);

        // show symbol in legend
        curve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, true);
        curve->setLegendIconSize(QSize(8,8));

        curve->setSamples(xData,yData);

        // -> register curve latest to be in the front

        // remove all curves
        fitPlot->plotTextLabelVisible(false);
        fitPlot->detachAllCurves();

        // get fitdata if available and visualize in table

        Signal t_signal;

        t_signal = getCurrentTChannel(mp);

        // check if signal is empty
        int idx = t_signal.indexAt(x);

        if(idx == -1)
        {

            // TODO: optimze dataCursor

            qDebug() << "AToolTemperatureFit: Curve not found";
            fitResultTableWidget->clearContents();
            resultPlot->detachAllCurves();
        }
        else
        {

            material_spec = Core::instance()->modelingSettings->materialSpec();

            QList<DatabaseContent*> materials = *Core::instance()->getDatabaseManager()->getMaterials();
            for(int k = 0; k < materials.size(); k++)
            {
                if(t_signal.fitMaterial == materials.at(k)->name)
                    material_spec = Material(*Core::instance()->getDatabaseManager()->getMaterial(k));
            }

            qDebug() << "AToolTemp: " << material_spec.name;

            fir = t_signal.fitData.at(t_signal.indexAt(x));


            // select last iteration in result table
            it_selection = fir.size()-1;

            QTableWidgetItem * item;

            fitResultTableWidget->setRowCount(fir.size());

            QString val;
            int rh = 18; // rowheight
            double T, C;


            // refresh resolution
            statResX = int(input_resolutionX->getValue());
            statResY = int(input_resolutionY->getValue());

            // fill matrix with -1.0 values (idx = 0)
            statisticsPlot->initMatrix(0, statResY, statResX);


            if(statAutoX)
            {
                // comparison start with xmax for cmin
                double cmin = input_xmax->getValue();
                double cmax = input_xmin->getValue();

                // search for min and max C values for this data point
                for(int i = 0; i < fir.size(); i++)
                {
                    C = fir.at(i).at(4);
                    if(C < cmin)
                        cmin = C * 0.90; // add 10% border to left side
                    if(C > cmax)
                        cmax = C * 1.1; // add 10% border to right side;
                }
                input_xmin->setValue(cmin);
                input_xmax->setValue(cmax);
            }

            if(statAutoY)
            {
                // comparison start with ymax for Tmin
                double Tmin = input_ymax->getValue();
                double Tmax = input_ymin->getValue();

                // search for min and max T values for this data point
                for(int i = 0; i < fir.size(); i++)
                {
                    T = fir.at(i).at(2);
                    if(T < Tmin)
                        Tmin = T * 0.90; // add 10% border to left side
                    if(T > Tmax)
                        Tmax = T * 1.1; // add 10% border to right side;
                }
                input_ymin->setValue(Tmin);
                input_ymax->setValue(Tmax);
            }


            // auto center plot again
            if(statAutoX || statAutoY)
            {
                statisticsPlot->qwtPlot->setAxisAutoScale(QwtPlot::yLeft);
                statisticsPlot->qwtPlot->setAxisAutoScale(QwtPlot::xBottom);
            }

            // refresh x/y ranges
            double xmin = input_xmin->getValue();
            double xmax = input_xmax->getValue();
            double ymin = input_ymin->getValue();
            double ymax = input_ymax->getValue();

            statisticsPlot->setRange(xmin, xmax, ymin, ymax);


            QVector<double> statXData;
            QVector<double> statYData;

            for(int i = 0; i < fir.size(); i++)
            {
                fitResultTableWidget->setRowHeight(i,rh);

                T = fir.at(i).at(2); // first parameter: res[2]

                val.sprintf("%g",T);
                fitResultTableWidget->setItem(i,0,new QTableWidgetItem(val));

                C = fir.at(i).at(4); // second parameter: res[4]

                val.sprintf("%g",C);
                fitResultTableWidget->setItem(i,1,new QTableWidgetItem(val));

                val.sprintf("%g",fir.at(i).at(3)); // delta T: res[3]
                fitResultTableWidget->setItem(i,2,new QTableWidgetItem(val));

                val.sprintf("%g",fir.at(i).at(5)); // delta C: res[5]
                fitResultTableWidget->setItem(i,3,new QTableWidgetItem(val));

                val.sprintf("%g",fir.at(i).at(0)); // chsiquare: res[0]
                fitResultTableWidget->setItem(i,4,new QTableWidgetItem(val));

                val.sprintf("%g",fir.at(i).at(1)); // lambda: res[1]
                fitResultTableWidget->setItem(i,5,new QTableWidgetItem(val));

                if(C < xmin)
                    statXData.append(xmin);
                else if (C > xmax)
                    statXData.append(xmax);
                else
                    statXData.append(C);

                if(T < ymin)
                    statYData.append(ymin);
                else if (T > ymax)
                    statYData.append(ymax);
                else
                    statYData.append(T);
            }

            double chisquare;
            double ymod, dy;
            double Ti, Ci;

            // active channels during fitting of selected T-Channel
            QList<bool> act = t_signal.fitActiveChannels;

            // E(m) source
            QString sourceEm = currentMRun->tempMetadata.value(tempChannelID).sourceEm;


            // calculate chisquare map
            for (int k = 0; k < statResY; k++)
                for (int u = 0; u < statResX; u++ )
                {
                    chisquare = 0.0;

                    for(int i = 0; i < xData.size(); i++)
                    {
                        if(act.at(i))
                        {
                            Ti = ymin + double(k)/double(statResY)*(ymax - ymin);
                            Ci = xmin + double(u)/double(statResX)*(xmax - xmin);

                            ymod = Temperature::calcPlanckIntensity(xData.at(i)*1E-9,
                                                                    Ti,
                                                                    Ci,
                                                                    material_spec,
                                                                    sourceEm);
                            dy = yData.at(i) - ymod;

                            chisquare += dy*dy;
                        }
                    }
                    statisticsPlot->setValue(0, k, u, chisquare);
                }

            if(statAutoZ)
                statisticsPlot->setZRangeAuto(0);


            // add iteration points to statistics plot
            curve_stats->detach();
            curve_stats->setPen(QColor(Qt::black));
            curve_stats->setStyle(QwtPlotCurve::Lines);

            QwtSymbol* s = new QwtSymbol(QwtSymbol::Cross);
            s->setColor(Qt::black);
            s->setSize(12);
            curve_stats->setSymbol(s);

            curve_stats->setSamples(statXData,statYData);

            curve_stats->attach(statisticsPlot->qwtPlot);


            // add first iteration point to statistics plot
            curve_stats_init->detach();
            curve_stats_init->setPen(QColor(Qt::red));
            QwtSymbol* si = new QwtSymbol(QwtSymbol::Triangle);
            si->setColor(Qt::red);
            si->setSize(12);
            curve_stats_init->setSymbol(si);

            // statXData.mid(0, 1) sub vector starting at 0 with length=1
            curve_stats_init->setSamples(statXData.mid(0, 1), statYData.mid(0, 1));
            curve_stats_init->attach(statisticsPlot->qwtPlot);

            statisticsPlot->updateData(0);
            statisticsPlot->qwtPlot->replot();


            //----------------------
            // Covariance plot

            plotCovariance();


            //----------------------
            // Fit Result Table


            fitResultTableWidget->selectRow(it_selection);

            QString name = QString("T%0 - %1").arg(tempChannelID).arg(material_spec.name);

            plotTemperatureFit(T, C, material_spec, sourceEm, name, QPen(Qt::black, 1, Qt::SolidLine));


            // define colormap for other temperature channels
            ColorMap *cmap = new ColorMap();
            cmap->setToJetStyle();

            Signal tt_signal;

            // plot temperature channels
            for(int j = 0; j < availableTChannels.size(); j++)
            {
                // skip for current channel
                if(availableTChannels.at(j) == tempChannelID)
                    continue;

                Material tmaterial;
                QString sourceEm_channels = currentMRun->tempMetadata.value(availableTChannels.at(j)).sourceEm;

                tt_signal = mp->getSignal(availableTChannels.at(j), Signal::TEMPERATURE);

                // get material
                QList<DatabaseContent*> materials = *Core::instance()->getDatabaseManager()->getMaterials();
                for(int k = 0; k < materials.size(); k++)
                {
                    if(tt_signal.fitMaterial == materials.at(k)->name)
                        tmaterial = Material(*Core::instance()->getDatabaseManager()->getMaterial(k));
                }

                QList<FitIterationResult> tfir = tt_signal.fitData.at(tt_signal.indexAt(x));
                T = tfir.last().at(2); // res[2]
                C = tfir.last().at(4); // res[4]

                QColor color = cmap->color(j, 0, availableTChannels.size());

                plotTemperatureFit(T,
                                   C,
                                   tmaterial,
                                   sourceEm_channels,
                                   QString("T%0 - %1").arg(availableTChannels.at(j)).arg(tmaterial.name),
                                   QPen(color, 1, Qt::SolidLine));

            }

            //onCbShowResultsChanged(cbShowResults->currentIndex());
        }

        // front level display
        fitPlot->registerCurve(curve);
    }
    else
    {
        MSG_ERR("Temperature signal not calculated!");
    }
}


void AToolTemperatureFit::plotTemperatureFit(double T, double C, Material mat,
                                             QString sourceEm,
                                             QString name, QPen pen)
{
    double lambda, intensity;
    QVector<double> xData, yData;
    QVector<double> xDataC, yDataC;
    QVector<double> xDataI, yDataI;

    //fitPlot->detachAllCurves();
    //fitPlot->unregisterCurve(curve_fit);

    // check if Material exists
    if(mat.ident == -1)
            return;

    curve_fit = new BasePlotCurve(QString("%0 T=%1; C=%2").arg(name).arg(T).arg(C));
    curve_fit->setPen(pen);

    curve_fit->setSamples(xData,yData);

    // show planck curve as overlay for fitted temperature
    for (int k = 300; k <= 1000; k++)
    {
        lambda = double(k);
        intensity = Temperature::calcPlanckIntensity(lambda*1E-9, T, C, mat, sourceEm);
        xData.append(lambda);
        yData.append(intensity);
    }

    curve_fit->setSamples(xData,yData);


    // temporary channel properties
    int wavelength;
    int bandwidth;
    int b2; // rounded half bandwidth

    double cintensity; // centerwavelength intensity
    double approx; // center wavelength approximation

    for (int chID = 1; chID <= currentMRun->getNoChannels(Signal::ABS); chID++)
    {

        wavelength = currentMRun->liiSettings().channels.at(chID-1).wavelength;
        bandwidth = currentMRun->liiSettings().channels.at(chID-1).bandwidth;
        b2 = currentMRun->liiSettings().channels[chID-1].getHalfBandwidth();

        xData.clear();
        yData.clear();

        Spectrum spec;

        for(int k = - b2 ; k <= b2; k++)
        {
            intensity = Temperature::calcPlanckIntensity((wavelength+k)*1E-9, T, C, mat, sourceEm);

            xData.append(wavelength + k);
            yData.append(intensity);

            spec.xData.append((wavelength + k)* 1E-9);
            spec.yData.append(intensity);

            // approximation is based on rounded half bandwidth (b2)
            if(k == 0)
            {
                approx = intensity * (b2*2) * 1E-9;
                cintensity = intensity;

                xDataC.append(wavelength);
                yDataC.append(intensity);
            }
        }

        double integral = spec.integrate(spec.xData.first(),spec.xData.last());

        xDataI.append(wavelength);
        yDataI.append(integral / (b2 * 2 * 1E-9));

        double aError = ((approx / integral) - 1.0) * 100.0;

        QString txtError,txtRes1, txtRes2;

        txtRes1.sprintf("%3.3f", approx);
        txtRes2.sprintf("%3.3f", integral);
        txtError.sprintf("%+.2f %%", aError);

        BasePlotCurve *bcurve = new BasePlotCurve(QString("%0 nm(%1 nm)  %2c: %3  %4: %5  %6: %7")
                                                  .arg(wavelength)
                                                  .arg(bandwidth)
                                                  .arg(QChar(0x03BB))
                                                  .arg(txtRes1)
                                                  .arg(QChar(0x222B))
                                                  .arg(txtRes2)
                                                  .arg(QChar(0x03B5))
                                                  .arg(txtError)
                                                  );
        bcurve->setSamples(xData,yData);
        bcurve->setBaseline(0.0);
        bcurve->setBrush(QBrush(Qt::blue,Qt::Dense6Pattern));

        // show legend only if checkbox for bandpass filter analysis is checked
        if(checkBoxBPAnalysis->isChecked() == false)
            bcurve->setItemAttribute(QwtPlotItem::Legend,false);

        fitPlot->registerCurve(bcurve);
    }


     // show symbols only if checkbox for bandpass filter anylsis is checked
    if(checkBoxBPAnalysis->isChecked())
    {
        // plot symbols for intensity of center wavelength
        BasePlotCurve *bcurve_center = new BasePlotCurve(QString("%0c - Center wavelength approximation")
                                                  .arg(QChar(0x03BB))
                                                  );

        // never change appearance
        bcurve_center->setFixedStyle(true);

        // curve style: no line
        bcurve_center->setStyle(BasePlotCurve::NoCurve);

        // data point symbol: Triangle
        QwtSymbol* s = new QwtSymbol(QwtSymbol::Triangle);
        s->setSize(5);
        s->setColor(Qt::black);
        bcurve_center->setSymbol(s);


        // legend attributes
        bcurve_center->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, true);
        bcurve_center->setLegendIconSize(QSize(8,8));

        bcurve_center->setSamples(xDataC,yDataC);

        fitPlot->registerCurve(bcurve_center);


        // plot symbols for intensity of integral
        BasePlotCurve *bcurve_int = new BasePlotCurve(QString("%0 - Integral approximation")
                                                  .arg(QChar(0x222B))
                                                  );

        // never change appearance
        bcurve_int->setFixedStyle(true);

        // curve style: no line
        bcurve_int->setPen(QPen(Qt::red, 1, Qt::DashLine));
        bcurve_int->setStyle(BasePlotCurve::NoCurve);

        // data point symbol: diamond
        QwtSymbol* s2 = new QwtSymbol(QwtSymbol::Diamond);
        s2->setSize(5);
        s2->setColor(Qt::red);
        bcurve_int->setSymbol(s2);

        // legend attributes
        bcurve_int->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, true);
        bcurve_int->setLegendIconSize(QSize(8,8));

        bcurve_int->setSamples(xDataI,yDataI);

        fitPlot->registerCurve(bcurve_int);
    }

    // plot fit
    fitPlot->registerCurve(curve_fit);
}


void AToolTemperatureFit::plotResultsIterations(int idx)
{
    //resultPlot->detachAllCurves();

    QVector<double> xData;
    QVector<double> yData;
    BasePlotCurve *curve;

    switch(idx)
    {
         default:
            curve = new BasePlotCurve("Temperature");
            resultPlot->setYLogScale(false);
            resultPlot->setPlotAxisTitles("Iteration", "Temperature / K");

            for(int i = 0; i < fir.size(); i++)
            {
                xData.append(i+1);
                yData.append(fir.at(i).at(2));
            }

        break;

        case 1:
            curve = new BasePlotCurve("C");
            resultPlot->setYLogScale(false);
            resultPlot->setPlotAxisTitles("Iteration", "Fit scaling factor C / -");

            for(int i = 0; i < fir.size(); i++)
            {
                xData.append(i+1);
                yData.append(fir.at(i).at(4));
            }

        break;

        case 2:
            curve = new BasePlotCurve("Chisquare");
            resultPlot->setYLogScale(true);
            resultPlot->setPlotAxisTitles("Iteration", "Chisquare / -");

            for(int i = 0; i < fir.size(); i++)
            {
                xData.append(i+1);
                yData.append(fir.at(i).at(0));
            }

        break;

        case 3:
            curve = new BasePlotCurve("Lambda");
            resultPlot->setYLogScale(true);
            resultPlot->setPlotAxisTitles("Iteration", "Damping factor Lambda / -");

            for(int i = 0; i < fir.size(); i++)
            {
                xData.append(i+1);
                yData.append(fir.at(i).at(1));
            }

        break;
    }

    curve->setSamples(xData, yData);

    // color

    QColor color = currentMRun->data(1).value<QColor>();
    QPen pen = QPen(color, 1, Qt::SolidLine);
    curve->setPen(pen);

    resultPlot->registerCurve(curve);
}


void AToolTemperatureFit::plotResultsFinal(int idx)
{
    //resultPlot->detachAllCurves();

    //QString title =currentMRun->getName();

    QVector<double> xData;
    QVector<double> yData;
    BasePlotCurve *curve;

    // check if t channel is available
    if(tempChannelID < 1)
        return;

    Signal signal = currentMPoint->getSignal(tempChannelID, Signal::TEMPERATURE);

    QList<FitIterationResult> fitResults;

    switch(idx)
    {
         default:
            curve = new BasePlotCurve("Temperature");
            resultPlot->setYLogScale(false);
            resultPlot->setPlotAxisTitles("Time / ns", "Temperature / K");

            for(int i = 0; i < signal.data.size(); i++)
            {
                fitResults = signal.fitData.at(i);

                xData.append(signal.time(i)*1E9);
                yData.append(fitResults.last().at(2)); // res[2];
            }

        break;

        case 1:
            curve = new BasePlotCurve("C");
            resultPlot->setYLogScale(false);
            resultPlot->setPlotAxisTitles("Time / ns", "Fit scaling factor C / -");

            for(int i = 0; i < signal.data.size(); i++)
            {
                fitResults = signal.fitData.at(i);

                xData.append(signal.time(i)*1E9);
                yData.append(fitResults.last().at(4)); // res[4];
            }

        break;

        case 2:
            curve = new BasePlotCurve("Chisquare");            
            resultPlot->setYLogScale(true);
            resultPlot->setPlotAxisTitles("Time / ns", "Chisquare / -");

            for(int i = 0; i < signal.data.size(); i++)
            {
                fitResults = signal.fitData.at(i);

                xData.append(signal.time(i)*1E9);
                yData.append(fitResults.last().at(0)); // res[0];
            }

        break;

        case 3:
            curve = new BasePlotCurve("Lambda");
            resultPlot->setYLogScale(true);
            resultPlot->setPlotAxisTitles("Time / ns", "Damping factor Lambda / -");

            for(int i = 0; i < signal.data.size(); i++)
            {
                fitResults = signal.fitData.at(i);

                xData.append(signal.time(i)*1E9);
                yData.append(fitResults.last().at(1)); // res[1];
            }

        break;
    }

    curve->setSamples(xData, yData);


    // color

    QColor color = currentMRun->data(1).value<QColor>();

    QPen pen = QPen(color, 1, Qt::SolidLine);
    curve->setPen(pen);


    resultPlot->registerCurve(curve);
}



/**
 * @brief AToolTemperatureFit::plotCovariance
 */
void AToolTemperatureFit::plotCovariance()
{
    if(currentMPoint == NULL)
        return;

    covarPlot->detachAllCurves();

    Signal::SType stype = selectedSignalType();;

    if(stype == Signal::TEMPERATURE)
        return;

    ColorMap *cmapcov = new ColorMap();
    cmapcov->setToJetStyle();

    // take delta time from first channel
    Signal signal = currentMPoint->getSignal(1, stype);


    QList<CovMatrix> covar_list;

    if(stype == Signal::RAW)
    {
        covar_list = currentMPoint->covar_list_raw;
        covarPlot->setPlotAxisTitles("Raw Intensity /-", "Covariance / -");
    }
    else
    {
        covar_list = currentMPoint->covar_list_abs;
        covarPlot->setPlotAxisTitles("Absolute Intensity /-", "Covariance / -");
    }

    if(covar_list.size() == 0)
        return;

    QVector<double> xDataCov;
    QVector<double> yDataCov;

    for(int i = 0; i < currentMPoint->channelCount(stype); i++)
        for(int j = 0; j <= i; j++)
        {
            // skip channel if not selected
            if(!selectedChannelIds().contains(i+1))
                continue;

            if(!selectedChannelIds().contains(j+1))
                continue;

            xDataCov.clear();
            yDataCov.clear();

            BasePlotCurve *curveCov = new BasePlotCurve(
                        QString("%0%1,%2").arg(QChar(0x03C3)).arg(i+1).arg(j+1));

            QColor color = cmapcov->color(i+0.2*(j+i), 0, currentMPoint->channelCount(Signal::ABS)+1);
            curveCov->setPen(QPen(color, 0.5, Qt::SolidLine));

            for(int k = 0; k < signal.data.size(); k++)
            {
                // make sure index is in list (may be ill defined if getSection was used
                if(k < covar_list.size())
                {
                    CovMatrix covar = covar_list.at(k);

                    // correlation coefficient
                    //double corr_coeff = covar(i,j) / sqrt(covar(i,i) * covar(j,j));
                    //xDataCov.append(corr_coeff);


                    //xDataCov.append(signal.data.at(k));
                    //yDataCov.append(covar(i,j));

                    // covariance over time
                    xDataCov.append(signal.time(k)*1E9);
                    yDataCov.append(covar(i,j));
                }
            }

            curveCov->setSamples(xDataCov, yDataCov);

            covarPlot->registerCurve(curveCov);
        }
}



Signal AToolTemperatureFit::getCurrentTChannel(MPoint *mp)
{
    Signal t_signal;

    if(tempChannelID > 0)
        t_signal = mp->getSignal(tempChannelID, Signal::TEMPERATURE);

    //qDebug() << "AtToolTemperatureFit: Current T-Channel: " << tempChannelID;

    return t_signal;
}


void AToolTemperatureFit::handleSignalDataChanged()
{
    SignalPlotTool::handleSignalDataChanged();

    onComboboxMRunIndexChanged();
}


void AToolTemperatureFit::handleSelectedRunsChanged(QList<MRun *> &runs)
{

    comboboxMRun->blockSignals(true);
    int oldMRunID = comboboxMRun->currentData().toInt();
    comboboxMRun->clear();

    for(MRun *run : runs)
        comboboxMRun->addItem(run->name, run->id());

    int idx = comboboxMRun->findData(oldMRunID);
    if(idx != -1)
        comboboxMRun->setCurrentIndex(idx);
    comboboxMRun->blockSignals(false);

    onComboboxMRunIndexChanged();

    if(runs.size() <= 0)
    {
        updateMRunSelection();

        // emit signal for child classes
        emit mRunSelectionUpdated();
        return;
    }

    //MRun *run = selectedRuns().last();
    //MPoint *mp = run->getPost(0); // first mpoint idx=0
    //int chID;

    // save for later use
    //currentMRun = run;
    //currentMPoint = mp;

    //updateMRunSelection();

    // emit signal for child classes
    emit mRunSelectionUpdated();
}


/***************
 * HELPER
 */


void AToolTemperatureFit::onPointSelected(const QString, const QColor,
                                          const QPointF & markerPosition)
{
    addedRows.clear();
    plot_selection = markerPosition;
    //qDebug() << "AtToolTemperatureFit: Range changed: " << markerPosition.x() << " " << markerPosition.y();
    processDataInterval();
    onCbShowResultsChanged(cbShowResults->currentIndex());
}


void AToolTemperatureFit::onCellClicked(int row, int col)
{
    if(!addedRows.contains(row))
    {
        addedRows.push_back(row);

        it_selection = row;
        fitResultTableWidget->selectRow(it_selection);

        double T = fitResultTableWidget->item(row, 0)->text().toDouble();
        double C = fitResultTableWidget->item(row, 1)->text().toDouble();

        //qDebug() << "AToolTemperatureFit: Row selected: " << row << " - T: " << T << " - C: " << C;

        QString name = QString("IT: %0 ").arg(it_selection);

        QString sourceEm = currentMRun->tempMetadata.value(tempChannelID).sourceEm;

        plotTemperatureFit(T, C, material_spec, sourceEm, name);
    }
}


/**
 * @brief AToolTemperatureFit::onStatisticsPlotClicked executed if user clicks on statisticsPlot
 * @param x represents scaling factor
 * @param y represents temperature
 */
void AToolTemperatureFit::onStatisticsPlotClicked(double x,double y)
{
    //qDebug() << "AToolTemperatureFit: " << x << y << material_spec.name;

    double T = y;
    double C = x;

    QString name = QString("StatisticsPlot: ");

    QString sourceEm = currentMRun->tempMetadata.value(tempChannelID).sourceEm;

    plotTemperatureFit(T, C, material_spec, sourceEm, name);
}


void AToolTemperatureFit::onFitResultTabChanged(int idx)
{
    // recalculate plot
}


void AToolTemperatureFit::handleSelectedChannelsChanged(QList<int>& ch_ids)
{
    SignalPlotTool::handleSelectedChannelsChanged(ch_ids);

    if(ch_ids.size() > 0)
        plotCovariance();
}


void AToolTemperatureFit::handleSelectedStypeChanged(Signal::SType stype)
{
    SignalPlotTool::handleSelectedStypeChanged(stype);
    plotCovariance();
}


void AToolTemperatureFit::onToolActivation()
{
    handleSelectedRunsChanged(selectedRuns());

    SignalPlotTool::onToolActivation();
}


/******
 *  SHORTCUTS
 */

void AToolTemperatureFit::scrollResultsUp()
{
    if(it_selection > 0)
    {
        it_selection--;
        onCellClicked(it_selection,0);
    }
}


void AToolTemperatureFit::scrollResultsDown()
{
    if(it_selection < fitResultTableWidget->rowCount()-1)
    {
        it_selection++;
        onCellClicked(it_selection,0);
    }
}


/*******
 * BUTTONS
 */

void AToolTemperatureFit::onClearPlotButtonReleased()
{
    addedRows.clear();
    fitPlot->detachAllCurves();
    processDataInterval();
}


void AToolTemperatureFit::onResetPlotButtonReleased()
{
    resultPlot->detachAllCurves();
    onCbResultsTypeChanged(cbShowResults->currentIndex());
    resultPlot->setZoomMode(BasePlotWidgetQwt::PlotZoomMode::ZOOM_RESET);
}


/********
 * ComboBox
 */

void AToolTemperatureFit::onCbResultsTypeChanged(int idx)
{
    // change visualization of resultPlot

    onCbShowResultsChanged(cbShowResults->currentIndex());
}


void AToolTemperatureFit::onCbShowResultsChanged(int idx)
{
    if(cbResultsType->currentIndex() == 0)
        plotResultsIterations(idx);
    else
        plotResultsFinal(idx);
}


void AToolTemperatureFit::onCbTChannelChanged(QString idx)
{
    if(cbTChannel->currentText() == "N/A" || cbTChannel->currentText() == "")
        return;

    tempChannelID = cbTChannel->currentText().toInt();

    // get fit results for new temperature channel
    processDataInterval();
    onCbShowResultsChanged(cbShowResults->currentIndex());

    if(cbResultsType->currentIndex() == 0)
        plotResultsIterations(cbShowResults->currentIndex());
    else
        plotResultsFinal(cbShowResults->currentIndex());
}


void AToolTemperatureFit::onCheckBoxBPAnalysisSelected(int state)
{
    // reprocess data interval = hide/show bandpass analysis
    processDataInterval();
    onCbShowResultsChanged(cbShowResults->currentIndex());
}


void AToolTemperatureFit::onCheckBoxStatAutoX(int state)
{
    //qDebug() << "AToolTemperatureFit::onCheckBoxStateAutoX";
    statAutoX = checkBoxStatAutoX->isChecked();

    // if auto: reprocess data interval
    if(statAutoX)
    {
        processDataInterval();
        onCbShowResultsChanged(cbShowResults->currentIndex());
    }
}


void AToolTemperatureFit::onCheckBoxStatAutoY(int state)
{
    //qDebug() << "AToolTemperatureFit::onCheckBoxStateAutoY";
    statAutoY = checkBoxStatAutoY->isChecked();

    // if auto: reprocess data interval
    if(statAutoY)
    {
        processDataInterval();
        onCbShowResultsChanged(cbShowResults->currentIndex());
    }
}

void AToolTemperatureFit::onCheckBoxStatAutoZ(int state)
{
    //qDebug() << "AToolTemperatureFit::onCheckBoxStateAutoZ";
    statAutoZ = checkBoxStatAutoZ->isChecked();

    if(statAutoZ)
        statisticsPlot->setZRangeAuto(0);
    else
        onInputZChanged();

    // reprocess data interval
    processDataInterval();
    onCbShowResultsChanged(cbShowResults->currentIndex());
}


/********
 * NumberLineEdit
 */


/**
 * @brief AToolTemperatureFit::onInputZChanged Signal valueChanged() is called by SLOT of input_zmin/input_zmax
 */
void AToolTemperatureFit::onInputZChanged()
{
    //qDebug() << "AToolTemperatureFit::onInpuTZChanged";

    double zmin = input_zmin->getValue();
    double zmax = input_zmax->getValue();

    statisticsPlot->setZRange(zmin, zmax);
}

/**
 * @brief AToolTemperatureFit::onRangeChangedZ slot from BasePlotSpectrogramWidget
 * @param zi
 * @param za
 */
void AToolTemperatureFit::onRangeChangedZ(double zi, double za)
{
    //qDebug() << "AToolTemperatureFit::onRangeChangedZ";
    input_zmin->setValue(zi);
    input_zmax->setValue(za);
}


//void AToolTemperatureFit::onInputAxesChanged()
//{
//    double xmin = input_xmin->getValue();
//    double xmax = input_xmax->getValue();

//    double ymin = input_ymin->getValue();
//    double ymax = input_ymax->getValue();

//    statisticsPlot->setRange(xmin, xmax, ymin, ymax);
//}


//void AToolTemperatureFit::onInputResolutionChanged()
//{
//    statResX = input_resolutionX->getValue();
//    statResY = input_resolutionY->getValue();
//}


/**
 * @brief AToolTemperatureFit::onXViewChanged this slot links signal plot with covariance plot
 * @param xmin
 * @param xmax
 */
void AToolTemperatureFit::onXViewChanged(double xmin, double xmax)
{
    plotter->setXView(xmin,xmax);
    covarPlot->setXView(xmin,xmax);
}


/********
 * Keys
 */

void AToolTemperatureFit::keyReleaseEvent(QKeyEvent *event)
{

    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        // if user modifies range unckeck "Auto Z-Axis"
        if(statAutoZ)
        {
            qDebug() << "AToolTemperatureFit::keyRelease uncheck";
            checkBoxStatAutoZ->setChecked(false);
            statAutoZ = false;
        }
         qDebug() << "AToolTemperatureFit::keyRelease process";
        processDataInterval();
        onCbShowResultsChanged(cbShowResults->currentIndex());
    }

    QWidget::keyReleaseEvent(event);
}


void AToolTemperatureFit::onComboboxMRunIndexChanged()
{
    int runID = comboboxMRun->itemData(comboboxMRun->currentIndex()).toInt();

    currentMRun = Core::instance()->getSignalManager()->dataModel()->mrun(runID);

    if(currentMRun)
    {
        // TODO: get signal idx from treeview

        qDebug() << "AToolTemperatureFit: only first MPoint is used for calculation";
        currentMPoint = currentMRun->getPost(0);

        fitPlot->clearText();
        fitPlot->addText(currentMRun->name);

        // clear list of available temperature channels

        availableTChannels.clear();
        int prev_tempChannelID = tempChannelID;

        cbTChannel->clear(); // Caution: calls combobox slot currentIndexChanged
        cbTChannel->setEnabled(true);

        qDebug() << "Temperature channels: - " << currentMPoint->channelCount(Signal::TEMPERATURE);

        int chID;

        // check which temperature channels are available and show in combo box
        for(int i=0; i < currentMPoint->channelCount(Signal::TEMPERATURE); i++)
        {
            chID = currentMPoint->channelIDs(Signal::TEMPERATURE).at(i);

            if(currentMPoint->isValidFitTChannelID(chID))
            {
                    cbTChannel->addItem(QString::number(chID));

                    // add channel to list
                    availableTChannels.append(chID);

                    if(chID == prev_tempChannelID)
                    {
                        tempChannelID = prev_tempChannelID;
                        cbTChannel->setCurrentText(QString::number(chID));
                    }

                    //qDebug() << "Fitdata: " << chID;
            }
            //else
                //qDebug() << "No fitdata: " << chID;

        }

        // set initial temperature channel
        if(tempChannelID == -1)
        {
            if(cbTChannel->currentText() != "N/A" && cbTChannel->currentText() != "")
                tempChannelID = cbTChannel->currentText().toInt();
        }

        if(cbTChannel->count() == 0)
        {
            cbTChannel->setEnabled(false);
            cbTChannel->addItem("N/A");
        }

        updateMRunSelection();

        processDataInterval();

        if(cbResultsType->currentIndex() == 0)
            plotResultsIterations(cbShowResults->currentIndex());
        else
            plotResultsFinal(cbShowResults->currentIndex());

        qDebug() << "AToolTemperatureFit::onComboboxMRunIndexChanged" << currentMRun->name;
    }

    emit mRunSelectionUpdated();
}


void AToolTemperatureFit::onButtonUpdateSettingsClicked()
{
    //TODO: split processDataInterval into subfunctions
    if(checkBoxStatAutoZ->isChecked())
        statisticsPlot->setZRangeAuto(0);
    else
        onInputZChanged();

    // reprocess data interval
    processDataInterval();
    onCbShowResultsChanged(cbShowResults->currentIndex());
}


void AToolTemperatureFit::onSplitterMoved()
{
    if(QObject::sender() == verticalSplitterLeft)
        Core::instance()->guiSettings->setSplitterPosition(identifier_splitterLeftV, verticalSplitterLeft->saveState());
    else if(QObject::sender() == verticalSplitterRight)
        Core::instance()->guiSettings->setSplitterPosition(identifier_splitterRightV, verticalSplitterRight->saveState());
    else if(QObject::sender() == bottomRightSplitter)
        Core::instance()->guiSettings->setSplitterPosition(identifier_bottomSplitterH, bottomRightSplitter->saveState());
    else if(QObject::sender() == horizontalSplitter)
        Core::instance()->guiSettings->setSplitterPosition(identifier_splitterMiddleH, horizontalSplitter->saveState());
}


void AToolTemperatureFit::onGuiSettingsChanged()
{
    QVariant position;
    if(Core::instance()->guiSettings->getSplitterPosition(identifier_splitterLeftV, position))
        verticalSplitterLeft->restoreState(position.toByteArray());
    if(Core::instance()->guiSettings->getSplitterPosition(identifier_splitterRightV, position))
        verticalSplitterRight->restoreState(position.toByteArray());
    if(Core::instance()->guiSettings->getSplitterPosition(identifier_bottomSplitterH, position))
        bottomRightSplitter->restoreState(position.toByteArray());
    if(Core::instance()->guiSettings->getSplitterPosition(identifier_splitterMiddleH, position))
        horizontalSplitter->restoreState(position.toByteArray());
}

