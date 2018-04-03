#include "parameteranalysis.h"

#include <qwt_symbol.h>
#include <QToolButton>
#include <QColorDialog>
#include <QMessageBox>
#include <QMutexLocker>
#include <QClipboard>
#include <QShortcut>
#include <QApplication>
#include <QHeaderView>

#include "core.h"
#include "../../signal/processing/processingchain.h"
#include "../../signal/processing/plugins/multisignalaverage.h"

const QString ParameterAnalysis::identifier_splitterLeft = "parameter_analysis_splitter_left_h";
const QString ParameterAnalysis::identifier_splitterRight = "parameter_analysis_splitter_right_h";
const QString ParameterAnalysis::identifier_splitterMiddle = "parameter_analysis_splitter_middle_v";

#define TABLE_ROW_HEIGHT 22

// --- ParameterAnalysisUpdater class implementation ---

ParameterAnalysisUpdater::ParameterAnalysisUpdater(ParameterAnalysis *parent) : QThread(static_cast<QObject*>(parent))
{
    updateLastCurveOnly = false;
    selectionChanged = false;
}


void ParameterAnalysisUpdater::run()
{
    ParameterAnalysis *pa = static_cast<ParameterAnalysis*>(parent());

    QMutexLocker(pa->updaterLock);

    if(pa->curves.isEmpty() || pa->selectedRuns().isEmpty())
    {
        for(ParameterAnalysisCurve *curve : pa->curves)
        {
            curve->xData.clear();
            curve->yData.clear();
            curve->curve->setSamples(curve->xData, curve->yData);
        }

        emit updaterFinished();
        return;
    }

    QList<MRun*> mruns = pa->selectedRuns();

    int progress = 0;
    int progressAdd = 100 / pa->curves.size();

    for(int curveIndex = 0; curveIndex < pa->curves.size(); curveIndex++)
    {
        if(selectionChanged)
        {
            curveIndex = 0;
            progress = 0;
            mruns = pa->selectedRuns();
            selectionChanged = false;
        }

        if(updateLastCurveOnly)
            curveIndex = pa->curves.size() - 1;

        pa->curves.at(curveIndex)->xData.clear();
        pa->curves.at(curveIndex)->yData.clear();

        for(int runIndex = 0; runIndex < mruns.size(); runIndex++)
        {
            QVector<double> xDataTemp, yDataTemp;
            xDataTemp.clear();
            yDataTemp.clear();

            bool multiSignalX = false;
            bool multiSignalY = false;

            //--------------
            // prepare xData

            // user defined parameters
            if(pa->curves.at(curveIndex)->parameterX == pa->identifier_udp)
            {
                if(mruns.at(runIndex)->userDefinedParameters.contains(pa->curves.at(curveIndex)->sourceX.value<QString>()))
                    xDataTemp.push_back(mruns.at(runIndex)->userDefinedParameters.value(pa->curves.at(curveIndex)->sourceX.value<QString>()).toDouble());
            }
            // average data (for all signal types)
            else if(pa->curves.at(curveIndex)->parameterX == pa->identifier_avg_data_1 || pa->curves.at(curveIndex)->parameterX == pa->identifier_avg_data_2)
            {
                multiSignalX = true;

                try
                {
                    for(int mpointIndex = 0; mpointIndex < mruns.at(runIndex)->sizeValidMpoints(); mpointIndex++)
                    {
                        Signal signal = mruns.at(runIndex)->getValidPostPre(mpointIndex)->getSignal(pa->curves.at(curveIndex)->sourceChannelX, pa->curves.at(curveIndex)->sourceX.value<Signal::SType>());

                        if(pa->curves.at(curveIndex)->parameterX == pa->identifier_avg_data_1)
                            xDataTemp.push_back(signal.calcRangeAverage(pa->xStart, pa->xEnd));
                        else if(pa->curves.at(curveIndex)->parameterX == pa->identifier_avg_data_2)
                            xDataTemp.push_back(signal.calcRangeAverage(pa->xStart1, pa->xEnd1));
                    }
                }
                catch(LIISimException e)
                {
                    qDebug() << "ParameterAnalysis xData: no valid MPoint" << e.what();
                    //TODO: message in ui
                }

                //check if there is either a multi signal average plugin in the source processing chain or if there is
                //a msa plugin in raw/absolute when the source is a temperature signal
                bool msa = false;
                if(pa->curves.at(curveIndex)->sourceX.value<Signal::SType>() == Signal::TEMPERATURE
                        && (mruns.at(runIndex)->getProcessingChain(Signal::RAW)->containsActivePlugin(MultiSignalAverage::pluginName)
                        || mruns.at(runIndex)->getProcessingChain(Signal::ABS)->containsActivePlugin(MultiSignalAverage::pluginName))
                        && !xDataTemp.isEmpty())
                    msa = true;
                else if(mruns.at(runIndex)->getProcessingChain(pa->curves.at(curveIndex)->sourceX.value<Signal::SType>())->containsActivePlugin(MultiSignalAverage::pluginName)
                        && !xDataTemp.isEmpty())
                    msa = true;

                if(msa)
                {
                    bool allValuesEqual = true;
                    double testValue = xDataTemp.first();

                    for(int i = 0; i < xDataTemp.size(); i++)
                    {
                        if(testValue != xDataTemp.at(i))
                            allValuesEqual = false;
                    }

                    // use only one signal if all signals are equal
                    if(allValuesEqual)
                    {
                        double temp = xDataTemp.first();
                        xDataTemp.clear();
                        xDataTemp.push_back(temp);

                        multiSignalX = false;
                    }
                }
            }
            // integral
            else if(pa->curves.at(curveIndex)->parameterX == pa->identifier_integral_1 || pa->curves.at(curveIndex)->parameterX == pa->identifier_integral_2)
            {
                multiSignalX = true;

                for(int mpointIndex = 0; mpointIndex < mruns.at(runIndex)->sizeValidMpoints(); mpointIndex++)
                {
                    Signal signal = mruns.at(runIndex)->getValidPostPre(mpointIndex)->getSignal(pa->curves.at(curveIndex)->sourceChannelX, pa->curves.at(curveIndex)->sourceX.value<Signal::SType>());
                    Signal section;

                    if(pa->curves.at(curveIndex)->parameterX == pa->identifier_integral_1)
                        section = signal.getSection(pa->xStart, pa->xEnd);
                    else if(pa->curves.at(curveIndex)->parameterX == pa->identifier_integral_2)
                        section = signal.getSection(pa->xStart1, pa->xEnd1);

                    double value = 0;

                    for(int i = 0; i < section.data.size(); i++)
                        value += section.data.at(i);

                    xDataTemp.push_back(value);
                }
            }
            // laser fluence
            else if(pa->curves.at(curveIndex)->parameterX == pa->identifier_laser_fluence)
            {
                xDataTemp.push_back(mruns.at(runIndex)->laserFluence());
            }
            // filter
            else if(pa->curves.at(curveIndex)->parameterX == pa->identifier_filter)
            {
                if(mruns.at(runIndex)->filter().identifier == "no Filter")
                    xDataTemp.push_back(100.0);
                else
                    xDataTemp.push_back(mruns.at(runIndex)->filter().identifier.toDouble());
            }
            // fit scaling factor
            else if(pa->curves.at(curveIndex)->parameterX == pa->identifier_fit_factor)
            {
                try
                {
                    for(int mpointIndex = 0; mpointIndex < mruns.at(runIndex)->sizeValidMpoints(); mpointIndex++)
                    {
                        Signal signal = mruns.at(runIndex)->getValidPost(mpointIndex)->getSignal(0, Signal::TEMPERATURE);

                        if(!signal.fitData.isEmpty())
                            xDataTemp.push_back(signal.fitData.last().last().at(4)); // scaling factor: res[4]
                    }
                }
                catch(LIISimException e)
                {
                    qDebug() << "ParameterAnalysis Error - Fit scaling factor: " << e.what();
                }
            }
            // pmt gain voltage
            else if(pa->curves.at(curveIndex)->parameterX == pa->identifier_pmt_gain_voltage)
            {
                xDataTemp.push_back(mruns.at(runIndex)->pmtGainVoltage(pa->curves.at(curveIndex)->sourceChannelX));
            }
            // pmt measured voltage
            else if(pa->curves.at(curveIndex)->parameterX == pa->identifier_pmt_reference_voltage)
            {
                xDataTemp.push_back(mruns.at(runIndex)->pmtReferenceGainVoltage(pa->curves.at(curveIndex)->sourceChannelX));
            }
            else if(pa->curves.at(curveIndex)->parameterX == pa->identifier_mrun_list)
            {
                xDataTemp.push_back(runIndex+1);
            }

            //--------------
            // prepare yData

            if(pa->curves.at(curveIndex)->parameterY == pa->identifier_udp)
            {
                if(mruns.at(runIndex)->userDefinedParameters.contains(pa->curves.at(curveIndex)->sourceY.value<QString>()))
                    yDataTemp.push_back(mruns.at(runIndex)->userDefinedParameters.value(pa->curves.at(curveIndex)->sourceY.value<QString>()).toDouble());
            }
            else if(pa->curves.at(curveIndex)->parameterY == pa->identifier_avg_data_1 || pa->curves.at(curveIndex)->parameterY == pa->identifier_avg_data_2)
            {
                multiSignalY = true;

                // all signal types (raw, abs, temperature)
                try
                {
                    for(int mpointIndex = 0; mpointIndex < mruns.at(runIndex)->sizeValidMpoints(); mpointIndex++)
                    //for(int mpointIndex = 0; mpointIndex < mruns.at(runIndex)->sizeValidMpoints(); mpointIndex++)
                    {
                        Signal signal = mruns.at(runIndex)->getValidPostPre(mpointIndex)->getSignal(pa->curves.at(curveIndex)->sourceChannelY, pa->curves.at(curveIndex)->sourceY.value<Signal::SType>());

                        // avg data 1
                        if(pa->curves.at(curveIndex)->parameterY == pa->identifier_avg_data_1)
                            yDataTemp.push_back(signal.calcRangeAverage(pa->xStart, pa->xEnd));
                        // avg data 2
                        else if(pa->curves.at(curveIndex)->parameterY == pa->identifier_avg_data_2)
                            yDataTemp.push_back(signal.calcRangeAverage(pa->xStart1, pa->xEnd1));
                    }
                }
                catch(LIISimException e)
                {
                    qDebug() << "ParameterAnalysis yData: no valid MPoint" << e.what();
                    //TODO: message in ui
                }

                //check if there is either a multi signal average plugin in the source processing chain or if there is
                //a msa plugin in raw/absolute when the source is a temperature signal
                bool msa = false;
                if(pa->curves.at(curveIndex)->sourceY.value<Signal::SType>() == Signal::TEMPERATURE
                        && (mruns.at(runIndex)->getProcessingChain(Signal::RAW)->containsActivePlugin(MultiSignalAverage::pluginName)
                        || mruns.at(runIndex)->getProcessingChain(Signal::ABS)->containsActivePlugin(MultiSignalAverage::pluginName)) && !yDataTemp.isEmpty())
                    msa = true;
                else if(mruns.at(runIndex)->getProcessingChain(pa->curves.at(curveIndex)->sourceY.value<Signal::SType>())->containsActivePlugin(MultiSignalAverage::pluginName)
                        && !yDataTemp.isEmpty())
                    msa = true;

                if(msa)
                {
                    bool allValuesEqual = true;
                    double testValue = yDataTemp.first();

                    for(int i = 0; i < yDataTemp.size(); i++)
                    {
                        if(testValue != yDataTemp.at(i))
                            allValuesEqual = false;
                    }

                    // use only one signal if all signals are equal
                    if(allValuesEqual)
                    {
                        double temp = yDataTemp.first();
                        yDataTemp.clear();
                        yDataTemp.push_back(temp);

                        multiSignalY = false;
                    }
                }
            }
            // integral
            else if(pa->curves.at(curveIndex)->parameterY == pa->identifier_integral_1 || pa->curves.at(curveIndex)->parameterY == pa->identifier_integral_2)
            {
                multiSignalY = true;

                for(int mpointIndex = 0; mpointIndex < mruns.at(runIndex)->sizeValidMpoints(); mpointIndex++)
                {
                    Signal signal = mruns.at(runIndex)->getValidPostPre(mpointIndex)->getSignal(pa->curves.at(curveIndex)->sourceChannelY, pa->curves.at(curveIndex)->sourceY.value<Signal::SType>());
                    Signal section;

                    if(pa->curves.at(curveIndex)->parameterY == pa->identifier_integral_1)
                        section = signal.getSection(pa->xStart, pa->xEnd);
                    else if(pa->curves.at(curveIndex)->parameterY == pa->identifier_integral_2)
                        section = signal.getSection(pa->xStart1, pa->xEnd1);

                    double value = 0;

                    for(int i = 0; i < section.data.size(); i++)
                        value += section.data.at(i);

                    yDataTemp.push_back(value);
                }
            }
            // laser fluence
            else if(pa->curves.at(curveIndex)->parameterY == pa->identifier_laser_fluence)
            {
                yDataTemp.push_back(mruns.at(runIndex)->laserFluence());
            }
            // filter
            else if(pa->curves.at(curveIndex)->parameterY == pa->identifier_filter)
            {
                if(mruns.at(runIndex)->filter().identifier == "no Filter")
                    yDataTemp.push_back(100.0);
                else
                    yDataTemp.push_back(mruns.at(runIndex)->filter().identifier.toDouble());
            }
            // fit scaling factor
            else if(pa->curves.at(curveIndex)->parameterY == pa->identifier_fit_factor)
            {
                try
                {
                    for(int mpointIndex = 0; mpointIndex < mruns.at(runIndex)->sizeValidMpoints(); mpointIndex++)
                    {

                        // this is only for first t-channel
                        Signal signal = mruns.at(runIndex)->getValidPost(mpointIndex)->getSignal(1, Signal::TEMPERATURE);

                        if(!signal.fitData.isEmpty())
                        {
                            //yDataTemp.push_back(signal.fitData.last().last().at(1));

                            double sum = 0.0;
                            int count = 0;

                            for(int i = signal.indexAt(pa->xStart); i < signal.indexAt(pa->xEnd); i++)
                            {
                                // sum up all fit scaling factors in range
                                sum += signal.fitData.at(i).last().at(4); // scaling factor: res[4]
                                count++;
                            }
                            yDataTemp.push_back(sum/double(count));
                        }
                    }
                }
                catch(LIISimException e)
                {
                    qDebug() << "ParameterAnalysis Error - Fit scaling factor: " << e.what();
                }
            }
            // pmt gain voltage
            else if(pa->curves.at(curveIndex)->parameterY == pa->identifier_pmt_gain_voltage)
            {
                yDataTemp.push_back(mruns.at(runIndex)->pmtGainVoltage(pa->curves.at(curveIndex)->sourceChannelY));
            }
            // pmt reference voltage
            else if(pa->curves.at(curveIndex)->parameterY == pa->identifier_pmt_reference_voltage)
            {
                yDataTemp.push_back(mruns.at(runIndex)->pmtReferenceGainVoltage(pa->curves.at(curveIndex)->sourceChannelY));
            }
            else if(pa->curves.at(curveIndex)->parameterY == pa->identifier_mrun_list)
            {
                yDataTemp.push_back(runIndex+1);
            }


            // multi signal average (just take first signal)
            if(multiSignalX && !multiSignalY && yDataTemp.size() > 0 && xDataTemp.size() > 0)
            {
                while(xDataTemp.size() > yDataTemp.size())
                    yDataTemp.push_back(yDataTemp.at(0));
            }
            if(!multiSignalX && multiSignalY && xDataTemp.size() > 0 && yDataTemp.size() > 0)
            {
                while(yDataTemp.size() > xDataTemp.size())
                    xDataTemp.push_back(xDataTemp.at(0));
            }
            if(!xDataTemp.isEmpty() && !yDataTemp.isEmpty())
            {
                //TODO: error message
                pa->curves.at(curveIndex)->xData.append(xDataTemp);
                pa->curves.at(curveIndex)->yData.append(yDataTemp);
            }
        }

        pa->curves.at(curveIndex)->curve->setSamples(pa->curves.at(curveIndex)->xData, pa->curves.at(curveIndex)->yData);

        progress += progressAdd;

        emit updaterProgress(progress);
    }

    updateLastCurveOnly = false;

    emit updaterFinished();
}


void ParameterAnalysisUpdater::lastCurveOnly()
{
    updateLastCurveOnly = true;
}

void ParameterAnalysisUpdater::setSelectionChanged()
{
    selectionChanged = true;
}


//---


ParameterAnalysis::ParameterAnalysis(QWidget *parent) : SignalPlotTool(parent)
{
    setObjectName("AT_PA");
    plotter->setObjectName("AT_PA_PLOT");
    m_title = "Parameter Analysis";
    m_iconLocation = Core::rootDir + "resources/icons/chart_line.png";

    xStart = 0.0;
    xEnd = 0.0;

    xStart1 = 0.0;
    xEnd1 = 0.0;

    curveCounter = 1;

    updaterLock = new QMutex();
    updater = new ParameterAnalysisUpdater(this);

    identifier_udp                      = "user parameter";
    identifier_settings                 = "settings";
    identifier_raw                      = "raw";
    identifier_absolute                 = "absolute";
    identifier_temperature              = "temperature";

    identifier_laser_fluence            = "laser fluence";
    identifier_filter                   = "ND-filter transmission";
    identifier_avg_data_1               = "average (range 1)";
    identifier_avg_data_2               = "average (range 2)";
    identifier_integral_1               = "integral (range 1)";
    identifier_integral_2               = "integral (range 2)";
    identifier_fit_factor               = "fit scaling factor";
    identifier_pmt_gain_voltage         = "PMT gain (set) voltage";
    identifier_pmt_reference_voltage    = "PMT gain (measured) voltage";
    identifier_mrun_list                = "MRun List";

    layout()->removeWidget(plotterWidget);
    patAvg1 = plotter->addPlotAnalysisTool("range 1", "Select range for calculation", false);
    patAvg2 = plotter->addPlotAnalysisTool("range 2", "Select range for calculation", false);

    splitterLeft = new QSplitter(Qt::Vertical);
    splitterLeft->addWidget(plotterWidget);

    SignalPlotTool::horizontalSplitter->addWidget(splitterLeft);

    paramPlot = new BasePlotWidgetQwt;
    paramPlot->setObjectName("AT_PA_PARAMETER_PLOT");
    paramPlot->setZoomMode(BasePlotWidgetQwt::PLOT_PAN);
    paramPlot->setDataTableToolName(m_title);
    paramPlot->setPlotTitle("Parameter Visualization");
    paramPlot->setPlotLabelText("<b>Parameter visualization:</b><br>\n\
(1) Select at least one measurement run<br>\n\
(2) Select source and parameter type for both axes<br>\n\
(3) Select a range with plot tool \"range 1\" or \"range 2\" if needed for parameter(s)<br>\n\
(4) Press \"Add (All Channels)\" or select a individual channel combination and press \"Add\"");
    paramPlot->plotTextLabelVisible(true);

    comboboxSignalType1 = new LabeledComboBox("X-Axis:");
    comboboxSignalType1->addStringItem(identifier_udp);
    comboboxSignalType1->addStringItem(identifier_settings);
    comboboxSignalType1->addStringItem(identifier_raw);
    comboboxSignalType1->addStringItem(identifier_absolute);
    comboboxSignalType1->addStringItem(identifier_temperature);
    comboboxSignalType1->setCurrentIndex(2);
    comboboxSignalType2 = new LabeledComboBox("Y-Axis:");
    comboboxSignalType2->addStringItem(identifier_udp);
    comboboxSignalType2->addStringItem(identifier_settings);
    comboboxSignalType2->addStringItem(identifier_raw);
    comboboxSignalType2->addStringItem(identifier_absolute);
    comboboxSignalType2->addStringItem(identifier_temperature);
    comboboxSignalType2->setCurrentIndex(3);

    comboboxParameter1 = new LabeledComboBox("Parameter:");
    comboboxParameter1->choices->setMinimumContentsLength(21);
    comboboxParameter2 = new LabeledComboBox("Parameter:");
    comboboxParameter2->choices->setMinimumContentsLength(21);

    updateParameterChoicesX();
    updateParameterChoicesY();

    comboboxChannelX = new QComboBox();
    comboboxChannelX->setMinimumContentsLength(10);
    comboboxChannelY = new QComboBox();
    comboboxChannelY->setEnabled(false); // link channel (true) -> disabled by default

    buttonAddToPlot = new QPushButton("Add");
    buttonAddToPlot->setEnabled(false);

    checkboxLinkChannel = new QCheckBox("Link Channel");
    checkboxLinkChannel->setToolTip("Links the channel selection if possible");
    checkboxLinkChannel->setChecked(true);

    checkboxUpdateOnSelection = new QCheckBox("Update on Selection Change");
    checkboxUpdateOnSelection->setToolTip("When selected, automatically updates the displayed data when measurement run / range selection is changed");

    checkboxUpdateOnSignalChange = new QCheckBox("Update on Signal Data Change");
    checkboxUpdateOnSignalChange->setToolTip("When selected, automatically updates the displayed data when any signal data is changed");

    buttonUpdateCurves = new QPushButton("Update Curves");
    buttonUpdateCurves->setToolTip("Updates 'Parameter Visualization'-plot");

    buttonAddAllChannel = new QPushButton("Add (All Channels)");
    buttonAddAllChannel->setToolTip("Only works in 'Link Channel'-mode");

    buttonUpdateWarning = new QToolButton;
    buttonUpdateWarning->setStyleSheet("QToolButton { border-style: none}");
    //buttonUpdateWarning->setToolTip("Selection / signal data changed. Displayed data might be out-of-date.");

    progressBarUpdater = new QProgressBar();
    progressBarUpdater->setMinimum(0);
    progressBarUpdater->setMaximum(100);
    progressBarUpdater->setValue(100);
    progressBarUpdater->setTextVisible(false);

    QWidget *curveSettingsWidget = new QWidget;
    QGridLayout *curveSettingsLayout = new QGridLayout;
    curveSettingsLayout->setMargin(0);
    curveSettingsWidget->setLayout(curveSettingsLayout);
    curveSettingsLayout->addWidget(comboboxSignalType1, 0, 0);
    curveSettingsLayout->addWidget(comboboxParameter1, 0, 1);
    curveSettingsLayout->addWidget(comboboxChannelX, 0, 2);
    curveSettingsLayout->addWidget(comboboxSignalType2, 1, 0);
    curveSettingsLayout->addWidget(comboboxParameter2, 1, 1);
    curveSettingsLayout->addWidget(comboboxChannelY, 1, 2);
    curveSettingsLayout->addWidget(checkboxLinkChannel, 0, 3, 2, 1);    

    // two-row layout (950 px)
    //curveSettingsLayout->addWidget(buttonAddToPlot, 0, 4, 1, 1);
    //curveSettingsLayout->addWidget(buttonAddAllChannel, 1, 4, 1, 1);

    // three-row layout (850 px) (for smaller screens)
    curveSettingsLayout->addWidget(buttonAddToPlot, 3, 2, 1, 1);
    curveSettingsLayout->addWidget(buttonAddAllChannel, 3, 3, 1, 1);

    QWidget *curveUpdateWidget = new QWidget;
    QGridLayout *curveUpdateLayout = new QGridLayout;
    curveUpdateLayout->setMargin(0);
    curveUpdateWidget->setLayout(curveUpdateLayout);
    curveUpdateLayout->addWidget(buttonUpdateWarning, 1, 0, 1, 1);
    curveUpdateLayout->addWidget(progressBarUpdater, 0, 1, 1, 1);
    curveUpdateLayout->addWidget(buttonUpdateCurves, 1, 1, 1, 1);
    curveUpdateLayout->addWidget(checkboxUpdateOnSelection, 0, 2, 1, 1);
    curveUpdateLayout->addWidget(checkboxUpdateOnSignalChange, 1, 2, 1, 1);
    curveUpdateLayout->addWidget(new QWidget(), 3, 1, 1, 2); // spacer, colspan=2

    QWidget *curveSettingsUpdateWidget = new QWidget;
    curveSettingsUpdateWidget->setObjectName("AT_PA_CURVE_SETTINGS");
    QHBoxLayout *curveSettingsUpdateLayout = new QHBoxLayout;
    curveSettingsUpdateLayout->setMargin(0);
    curveSettingsUpdateWidget->setLayout(curveSettingsUpdateLayout);
    curveSettingsUpdateLayout->addWidget(curveSettingsWidget);
    curveSettingsUpdateLayout->addWidget(curveUpdateWidget);
    curveSettingsUpdateLayout->setAlignment(curveSettingsWidget, Qt::AlignLeft);
    curveSettingsUpdateLayout->setAlignment(curveUpdateWidget, Qt::AlignRight);

    tableCurves = new QTableWidget();
    tableCurves->setObjectName("AT_PA_TABLE_CURVE");
    tableCurves->setColumnCount(9);
    QStringList columnNames;
    columnNames << "" << "Curve Name" << "Source X" << "Parameter X" << "Channel X" << "Source Y" << "Parameter Y" << "Channel Y" << "";
    tableCurves->setHorizontalHeaderLabels(columnNames);
    tableCurves->setColumnWidth(0, 30);
    tableCurves->setColumnWidth(2, 60);
    tableCurves->setColumnWidth(3, 120); // parameter x
    tableCurves->setColumnWidth(4, 70);
    tableCurves->setColumnWidth(5, 60);
    tableCurves->setColumnWidth(6, 120); // parameter y
    tableCurves->setColumnWidth(7, 70);
    tableCurves->setColumnWidth(8, 150);

    tableData = new ExtendedTableWidget();
    tableData->setObjectName("AT_PA_TABLE_DATA");
    tableData->setAllRowHeight(22);

    QWidget *parameterPlotterToolbar = new QWidget();
    QHBoxLayout *parameterPlotterToolbarLayout = new QHBoxLayout();
    parameterPlotterToolbarLayout->setMargin(0);
    parameterPlotterToolbar->setLayout(parameterPlotterToolbarLayout);
    autoPlotScaling = new QCheckBox("auto scale plot");
    parameterPlotterToolbarLayout->addWidget(autoPlotScaling);
    QToolBar *parameterPlotterTB = new QToolBar;
    parameterPlotterTB->addActions(paramPlot->toolActions());
    parameterPlotterToolbarLayout->addWidget(parameterPlotterTB);
    parameterPlotterTB->actions().first()->triggered(true);

    QWidget *parameterPlotterWidget = new QWidget();
    QVBoxLayout *parameterPlotterLayout = new QVBoxLayout();
    parameterPlotterWidget->setLayout(parameterPlotterLayout);
    parameterPlotterLayout->addWidget(parameterPlotterToolbar);
    parameterPlotterLayout->addWidget(paramPlot);
    parameterPlotterLayout->setAlignment(parameterPlotterToolbar, Qt::AlignRight);

    splitterRight = new QSplitter(Qt::Vertical);
    SignalPlotTool::horizontalSplitter->addWidget(splitterRight);
    splitterRight->addWidget(parameterPlotterWidget);
    splitterRight->addWidget(tableData);

    // resize left and right side
    QList<int> wlist;
    wlist << 300;
    wlist << horizontalSplitter->width()-wlist.at(0);

    horizontalSplitter->setSizes(wlist);


    QWidget *settingsAndCurveTableWidget = new QWidget();
    QVBoxLayout *settingsAndCurveTableLayout = new QVBoxLayout();
    settingsAndCurveTableLayout->setMargin(0);
    settingsAndCurveTableWidget->setLayout(settingsAndCurveTableLayout);

    settingsAndCurveTableLayout->addWidget(curveSettingsUpdateWidget);
    //settingsAndCurveTableLayout->addWidget(settingsWidget);
    settingsAndCurveTableLayout->addWidget(tableCurves);

    //settingsAndCurveTableLayout->setAlignment(settingsWidget, Qt::AlignLeft);

    splitterLeft->addWidget(settingsAndCurveTableWidget);

    plotterToolbar->addActions(toolbarActions());

    //connect(plotter, SIGNAL(rangeSelected(double,double)), SLOT(onRangeSelected(double,double)));
    connect(patAvg1, SIGNAL(dataSelected(double,double)), SLOT(onRangeSelected(double,double)));
    connect(patAvg2, SIGNAL(dataSelected(double,double)), SLOT(onRangeSelected(double,double)));
    connect(Core::instance()->guiSettings, SIGNAL(settingsChanged()), SLOT(onGuiSettingsChanged()));

    connect(comboboxParameter1, SIGNAL(currentIndexChanged(int)), SLOT(onComboboxCurrentIndexChanged()));
    connect(comboboxParameter2, SIGNAL(currentIndexChanged(int)), SLOT(onComboboxCurrentIndexChanged()));
    connect(comboboxSignalType1, SIGNAL(currentIndexChanged(int)), SLOT(onComboboxCurrentIndexChanged()));
    connect(comboboxSignalType2, SIGNAL(currentIndexChanged(int)), SLOT(onComboboxCurrentIndexChanged()));
    connect(comboboxChannelX, SIGNAL(currentIndexChanged(int)), SLOT(onComboboxCurrentIndexChanged()));
    connect(comboboxChannelY, SIGNAL(currentIndexChanged(int)), SLOT(onComboboxCurrentIndexChanged()));

    connect(buttonAddToPlot, SIGNAL(released()), SLOT(onButtonAddToPlotReleased()));

    connect(tableCurves, SIGNAL(cellChanged(int,int)), SLOT(onTableCurveDataCellChanged(int,int)));

    connect(buttonUpdateCurves, SIGNAL(clicked(bool)), SLOT(updateCurves()));

    connect(checkboxLinkChannel, SIGNAL(stateChanged(int)), SLOT(onCheckboxLinkChannelStateChanged(int)));

    connect(updater, SIGNAL(updaterProgress(int)), SLOT(onUpdaterProgress(int)));
    connect(updater, SIGNAL(updaterFinished()), SLOT(onUpdaterFinished()));

    connect(buttonAddAllChannel, SIGNAL(clicked(bool)), SLOT(onButtonAddAllChannelClicked()));

    connect(autoPlotScaling, SIGNAL(stateChanged(int)), SLOT(onCheckboxAutoPlotScalingStateChanged(int)));

    connect(splitterLeft, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    connect(splitterRight, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    connect(horizontalSplitter, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
}


void ParameterAnalysis::handleSignalDataChanged()
{
    SignalPlotTool::handleSignalDataChanged();

    if(checkboxUpdateOnSignalChange->isChecked())
        updateCurves();
    else
        if(curves.size() > 0)
            buttonUpdateWarning->setIcon(QIcon(Core::rootDir + "resources/icons/warning.png"));
}


void ParameterAnalysis::handleSelectedRunsChanged(QList<MRun *> &runs)
{
    SignalPlotTool::handleSelectedRunsChanged(runs);

    updateChannelCount();

    if(checkboxUpdateOnSelection->isChecked())
        updateCurves();
    else
        if(curves.size() > 0)
            buttonUpdateWarning->setIcon(QIcon(Core::rootDir + "resources/icons/warning.png"));
}


void ParameterAnalysis::handleCurrentRunChanged(MRun *run)
{
    SignalPlotTool::handleCurrentRunChanged(run);

    updateChannelCount();

    if(checkboxUpdateOnSelection->isChecked())
        updateCurves();
    else
        if(curves.size() > 0)
            buttonUpdateWarning->setIcon(QIcon(Core::rootDir + "resources/icons/warning.png"));
}


void ParameterAnalysis::handleSelectedChannelsChanged(QList<int>& ch_ids)
{
    SignalPlotTool::handleSelectedChannelsChanged(ch_ids);
}


void ParameterAnalysis::onToolActivation()
{
    SignalPlotTool::onToolActivation();

    updateChannelCount();

    if(checkboxUpdateOnSelection->isChecked())
        updateCurves();
    else
        if(curves.size() > 0)
            buttonUpdateWarning->setIcon(QIcon(Core::rootDir + "resources/icons/warning.png"));
}


void ParameterAnalysis::onRangeSelected(double start, double end)
{
    if(QObject::sender() == patAvg1)
    {
        xStart = start * 1E-9;
        xEnd = end * 1E-9;
    }
    if(QObject::sender() == patAvg2)
    {
        xStart1 = start * 1E-9;
        xEnd1 = end * 1E-9;
    }

    if(checkboxUpdateOnSelection->isChecked())
        updateCurves();
    else
        if(curves.size() > 0)
            buttonUpdateWarning->setIcon(QIcon(Core::rootDir + "resources/icons/warning.png"));
}


void ParameterAnalysis::onComboboxCurrentIndexChanged()
{
    if(QObject::sender() == comboboxSignalType1)
        updateParameterChoicesX();

    if(QObject::sender() == comboboxSignalType2)
        updateParameterChoicesY();

    if(QObject::sender() != comboboxChannelX && QObject::sender() != comboboxChannelY)
        updateChannelCount();

    if(QObject::sender() == comboboxChannelX && checkboxLinkChannel->isChecked())
    {
        int index = comboboxChannelY->findText(comboboxChannelX->currentText());
        if(index != -1)
            comboboxChannelY->setCurrentIndex(index);
    }

    if(QObject::sender() == comboboxChannelY && checkboxLinkChannel->isChecked())
    {
        int index = comboboxChannelX->findText(comboboxChannelY->currentText());
        if(index != -1)
            comboboxChannelX->setCurrentIndex(index);
    }
}


void ParameterAnalysis::updateCurveTable()
{
    tableCurves->blockSignals(true);
    tableCurves->setRowCount(curves.size());

    for(int i = 0; i < curves.size(); i++)
    {
        QString sourceTypeX;
        QString sourceTypeY;

        if(curves.at(i)->sourceX.type() == QVariant::UserType)
        {
            if(curves.at(i)->sourceX.value<Signal::SType>() == Signal::RAW)
                sourceTypeX = "raw";
            else if(curves.at(i)->sourceX.value<Signal::SType>() == Signal::ABS)
                sourceTypeX = "absolute";
            else if(curves.at(i)->sourceX.value<Signal::SType>() == Signal::TEMPERATURE)
                sourceTypeX = "temperature";
        }
        else if(curves.at(i)->parameterX == identifier_udp)
        {
            sourceTypeX = curves.at(i)->sourceX.value<QString>();
        }
        else
            sourceTypeX = "-";

        if(curves.at(i)->sourceY.type() == QVariant::UserType)
        {
            if(curves.at(i)->sourceY.value<Signal::SType>() == Signal::RAW)
                sourceTypeY = "raw";
            else if(curves.at(i)->sourceY.value<Signal::SType>() == Signal::ABS)
                sourceTypeY = "absolute";
            else if(curves.at(i)->sourceY.value<Signal::SType>() == Signal::TEMPERATURE)
                sourceTypeY = "temperature";
        }
        else if(curves.at(i)->parameterY == identifier_udp)
        {
            sourceTypeY = curves.at(i)->sourceY.value<QString>();
        }
        else
            sourceTypeY = "-";

        QWidget *visibilityWidget = new QWidget();
        QHBoxLayout *visibilityLayout = new QHBoxLayout();
        visibilityLayout->setMargin(2);
        visibilityLayout->addWidget(curves.at(i)->buttonVisibility);
        visibilityWidget->setLayout(visibilityLayout);

        tableCurves->setRowHeight(i, TABLE_ROW_HEIGHT);

        tableCurves->setItem(i, 0, new QTableWidgetItem(""));
        tableCurves->item(i, 0)->setFlags(Qt::ItemIsEnabled);
        tableCurves->setCellWidget(i, 0, visibilityWidget);

        tableCurves->setItem(i, 1, new QTableWidgetItem(QIcon(curves.at(i)->curve->legendIcon(0, QSizeF(16, 16)).toPixmap()), curves.at(i)->curve->title().text()));
        tableCurves->item(i, 1)->setToolTip(curves.at(i)->runNames);
        tableCurves->setItem(i, 2, new QTableWidgetItem(sourceTypeX));
        tableCurves->item(i, 2)->setFlags(Qt::ItemIsEnabled);        
        tableCurves->setItem(i, 3, new QTableWidgetItem(curves.at(i)->parameterX));
        tableCurves->item(i, 3)->setFlags(Qt::ItemIsEnabled);

        if(curves.at(i)->sourceChannelX != 0 && sourceTypeX != "temperature")
            tableCurves->setItem(i, 4, new QTableWidgetItem(QString::number(curves.at(i)->sourceChannelX)));
        else if (sourceTypeX == "temperature")
            tableCurves->setItem(i, 4, new QTableWidgetItem(
                                     QString("T%0").arg(curves.at(i)->sourceChannelX)));
        else
            tableCurves->setItem(i, 4, new QTableWidgetItem("-"));

        tableCurves->item(i, 4)->setFlags(Qt::ItemIsEnabled);

        tableCurves->setItem(i, 5, new QTableWidgetItem(sourceTypeY));
        tableCurves->item(i, 5)->setFlags(Qt::ItemIsEnabled);        
        tableCurves->setItem(i, 6, new QTableWidgetItem(curves.at(i)->parameterY));
        tableCurves->item(i, 6)->setFlags(Qt::ItemIsEnabled);

        if(curves.at(i)->sourceChannelY != 0 && sourceTypeY != "temperature")
            tableCurves->setItem(i, 7, new QTableWidgetItem(QString::number(curves.at(i)->sourceChannelY)));
        else if (sourceTypeY == "temperature")
            tableCurves->setItem(i, 7, new QTableWidgetItem(
                                     QString("T%0").arg(curves.at(i)->sourceChannelY)));
        else
            tableCurves->setItem(i, 7, new QTableWidgetItem("-"));
        tableCurves->item(i, 7)->setFlags(Qt::ItemIsEnabled);

        QWidget *buttonWidget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout();
        layout->setMargin(2);
        layout->addWidget(curves.at(i)->buttonRemove);
        layout->addWidget(curves.at(i)->buttonColor);
        buttonWidget->setLayout(layout);

        tableCurves->setCellWidget(i, 8, buttonWidget);
    }

    tableCurves->blockSignals(false);
}


void ParameterAnalysis::updateDataTable()
{
    QStringList headerLabels;
    QList<QList<QString>> data;

    for(int curveIndex = 0; curveIndex < curves.size(); curveIndex++)
    {
        if(curves.at(curveIndex)->visible)
        {
            headerLabels << QString("X%0").arg(curveIndex+1);
            headerLabels << QString("Y%0").arg(curveIndex+1);

            QList<QString> xData;
            QList<QString> yData;

            for(int sampleIndex = 0; sampleIndex < curves.at(curveIndex)->xData.size(); sampleIndex++)
                xData.push_back(QString::number(curves.at(curveIndex)->xData.at(sampleIndex)));

            for(int sampleIndex = 0; sampleIndex < curves.at(curveIndex)->yData.size(); sampleIndex++)
                yData.push_back(QString::number(curves.at(curveIndex)->yData.at(sampleIndex)));

            data.push_back(xData);
            data.push_back(yData);
        }
    }

    tableData->setData(headerLabels, data);
    tableData->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}


void ParameterAnalysis::updatePlot()
{
    QList<QString> plotXTitles;
    QList<QString> plotYTitles;

    if(curves.isEmpty() || selectedRuns().isEmpty())
        paramPlot->plotTextLabelVisible(true);
    else
        paramPlot->plotTextLabelVisible(false);

    for(int i = 0; i < curves.size(); i++)
    {
        if(curves.at(i)->visible)
        {
            paramPlot->registerCurve(curves.at(i)->curve);

            if(curves.at(i)->parameterX == identifier_avg_data_1)
            {
                switch(curves.at(i)->sourceX.value<Signal::SType>())
                {
                case Signal::RAW:
                    if(!plotXTitles.contains("Signal Average (raw) / V"))
                        plotXTitles.push_back("Signal Average (raw) / V");
                    break;
                case Signal::ABS:
                    if(!plotXTitles.contains("Signal Average (absolute) / arb. unit"))
                        plotXTitles.push_back("Signal Average (absolute) / arb. unit");
                    break;
                case Signal::TEMPERATURE:
                    if(!plotXTitles.contains("Average (temperature) / K"))
                        plotXTitles.push_back("Average (temperature) / K"); break;
                }
            }
            else if(curves.at(i)->parameterX == identifier_laser_fluence)
            {
                if(!plotXTitles.contains("Laser Fluence / mJ/mm²"))
                    plotXTitles.push_back("Laser Fluence / mJ/mm²");
            }
            else if(curves.at(i)->parameterX == identifier_filter)
            {
                if(!plotXTitles.contains("Filter Transmission / %"))
                    plotXTitles.push_back("Filter Transmission / %");
            }
            else if(curves.at(i)->parameterX == identifier_fit_factor)
            {
                if(!plotXTitles.contains("Fit scaling factor C / -"))
                    plotXTitles.push_back("Fit scaling factor C / -");
            }

            if(curves.at(i)->parameterY == identifier_avg_data_1)
            {
                switch(curves.at(i)->sourceY.value<Signal::SType>())
                {
                case Signal::RAW:
                    if(!plotYTitles.contains("Signal Average (raw) / V"))
                        plotYTitles.push_back("Signal Average (raw) / V");
                    break;
                case Signal::ABS:
                    if(!plotYTitles.contains("Signal Average (absolute) / arb. unit"))
                        plotYTitles.push_back("Signal Average (absolute) / arb. unit");
                    break;
                case Signal::TEMPERATURE:
                    if(!plotYTitles.contains("Average (temperature) / K"))
                        plotYTitles.push_back("Average (temperature) / K");
                    break;
                }
            }
            else if(curves.at(i)->parameterY == identifier_laser_fluence)
            {
                if(!plotYTitles.contains("Laser Fluence / mJ/mm²"))
                    plotYTitles.push_back("Laser Fluence / mJ/mm²");
            }
            else if(curves.at(i)->parameterY == identifier_filter)
            {
                if(!plotYTitles.contains("Filter Transmission / %"))
                    plotYTitles.push_back("Filter Transmission / %");
            }
            else if(curves.at(i)->parameterY == identifier_fit_factor)
            {
                if(!plotYTitles.contains("Fit scaling factor C / -"))
                    plotYTitles.push_back("Fit scaling factor C / -");
            }
        }
        else
            paramPlot->unregisterCurve(curves.at(i)->curve);
    }

    QString plotXTitle;
    QString plotYTitle;

    for(int i = 0; i < plotXTitles.size(); i++)
    {
        if(!plotXTitle.isEmpty())
            plotXTitle.append(" | ");
        plotXTitle.append(plotXTitles.at(i));
    }
    for(int i = 0; i < plotYTitles.size(); i++)
    {
        if(!plotYTitle.isEmpty())
            plotYTitle.append(" | ");
        plotYTitle.append(plotYTitles.at(i));
    }

    paramPlot->setPlotAxisTitles(plotXTitle, plotYTitle);

    if(autoPlotScaling->isChecked())
        paramPlot->setZoomMode(BasePlotWidgetQwt::ZOOM_RESET);
}


void ParameterAnalysis::updateColors()
{
    double h=0;
    double golden_ratio = 0.62473847638746;

    for(int i = 0; i < curves.size(); i++)
    {
        h = golden_ratio * 360 / (curves.size()) * i;
        h = (int)h;

        if(h < 140)
            h = (int)(h * 0.9);
        else
            h = (int)(h * 0.9 + 60);

        QColor curveColor = QColor::fromHsv(int(h), 245, 245, 255);

        if(!curves.at(i)->ownColor)
        {
            curves.at(i)->curve->setPen(QPen(curveColor, 1, Qt::DashLine));
            QwtSymbol *s = new QwtSymbol(QwtSymbol::Ellipse);
            s->setSize(5);
            s->setColor(curveColor);
            curves.at(i)->curve->setSymbol(s);
        }
    }
}


void ParameterAnalysis::updateCurves()
{
    buttonUpdateCurves->setEnabled(false);
    buttonUpdateWarning->setIcon(QIcon());

    if(updater->isRunning())
        updater->setSelectionChanged();
    else
        updater->start();
}


void ParameterAnalysis::updateParameterChoicesX()
{
    if(comboboxSignalType1->getCurrentText() == identifier_settings)
    {
        comboboxParameter1->clearAll();
        comboboxParameter1->addStringItem(identifier_laser_fluence);
        comboboxParameter1->addStringItem(identifier_filter);
        comboboxParameter1->addStringItem(identifier_pmt_gain_voltage);
        comboboxParameter1->addStringItem(identifier_pmt_reference_voltage);
        comboboxParameter1->addStringItem(identifier_mrun_list);
    }
    if(comboboxSignalType1->getCurrentText() == identifier_raw || comboboxSignalType1->getCurrentText() == identifier_absolute)
    {
        comboboxParameter1->clearAll();
        comboboxParameter1->addStringItem(identifier_avg_data_1);
        comboboxParameter1->addStringItem(identifier_avg_data_2);
        comboboxParameter1->addStringItem(identifier_integral_1);
        comboboxParameter1->addStringItem(identifier_integral_2);
    }
    if(comboboxSignalType1->getCurrentText() == identifier_temperature)
    {
        comboboxParameter1->clearAll();
        comboboxParameter1->addStringItem(identifier_avg_data_1);
        comboboxParameter1->addStringItem(identifier_avg_data_2);
        comboboxParameter1->addStringItem(identifier_integral_1);
        comboboxParameter1->addStringItem(identifier_integral_2);
        comboboxParameter1->addStringItem(identifier_fit_factor);
    }
    if(comboboxSignalType1->getCurrentText() == identifier_udp)
    {
        QStringList parameterList;

        for(int i = 0; i < selectedRuns().size(); i++)
        {
            for(auto e : selectedRuns().at(i)->userDefinedParameters.keys())
            {
                if(!parameterList.contains(e))
                    parameterList << e;
            }
        }

        comboboxParameter1->clearAll();
        for(int i = 0; i < parameterList.size(); i++)
            comboboxParameter1->addStringItem(parameterList.at(i));
    }
}


void ParameterAnalysis::updateParameterChoicesY()
{
    if(comboboxSignalType2->getCurrentText() == identifier_settings)
    {
        comboboxParameter2->clearAll();
        comboboxParameter2->addStringItem(identifier_laser_fluence);
        comboboxParameter2->addStringItem(identifier_filter);
        comboboxParameter2->addStringItem(identifier_pmt_gain_voltage);
        comboboxParameter2->addStringItem(identifier_pmt_reference_voltage);
        comboboxParameter2->addStringItem(identifier_mrun_list);
    }
    if(comboboxSignalType2->getCurrentText() == identifier_raw || comboboxSignalType2->getCurrentText() == identifier_absolute)
    {
        comboboxParameter2->clearAll();
        comboboxParameter2->addStringItem(identifier_avg_data_1);
        comboboxParameter2->addStringItem(identifier_avg_data_2);
        comboboxParameter2->addStringItem(identifier_integral_1);
        comboboxParameter2->addStringItem(identifier_integral_2);
    }
    if(comboboxSignalType2->getCurrentText() == identifier_temperature)
    {
        comboboxParameter2->clearAll();
        comboboxParameter2->addStringItem(identifier_avg_data_1);
        comboboxParameter2->addStringItem(identifier_avg_data_2);
        comboboxParameter2->addStringItem(identifier_integral_1);
        comboboxParameter2->addStringItem(identifier_integral_2);
        comboboxParameter2->addStringItem(identifier_fit_factor);
    }
    if(comboboxSignalType2->getCurrentText() == identifier_udp)
    {
        QStringList parameterList;

        for(int i = 0; i < selectedRuns().size(); i++)
        {
            for(auto e : selectedRuns().at(i)->userDefinedParameters.keys())
            {
                if(!parameterList.contains(e))
                    parameterList << e;
            }
        }

        comboboxParameter2->clearAll();
        for(int i = 0; i < parameterList.size(); i++)
            comboboxParameter2->addStringItem(parameterList.at(i));
    }
}


ParameterAnalysisCurve* ParameterAnalysis::buildCurveObject(QString parameterX, QString parameterY, QVariant signalTypeX, QVariant signalTypeY, int channelX, int channelY)
{
    BasePlotCurve *curve = new BasePlotCurve(QString("Curve %0").arg(curveCounter++));
    curve->setFixedStyle(true);
    curve->setPen(QPen(QColor(Qt::black), 1, Qt::DashDotDotLine));
    QwtSymbol *s = new QwtSymbol(QwtSymbol::Diamond);
    s->setSize(6);
    s->setColor(QColor(Qt::black));
    curve->setSymbol(s);

    ParameterAnalysisCurve *pac = new ParameterAnalysisCurve(this);
    pac->curve = curve;
    pac->parameterX = parameterX;
    pac->parameterY = parameterY;
    pac->sourceX = signalTypeX;
    pac->sourceY = signalTypeY;
    pac->sourceChannelX = channelX;
    pac->sourceChannelY = channelY;

    connect(pac, SIGNAL(visibilityChanged()), SLOT(onPACVisibilityChanged()));
    connect(pac, SIGNAL(colorChanged()), SLOT(onPACColorChanged()));
    connect(pac, SIGNAL(removeRequest(ParameterAnalysisCurve*)), SLOT(removeCurve(ParameterAnalysisCurve*)));

    return pac;

}


void ParameterAnalysis::updateChannelCount()
{
    comboboxChannelX->clear();
    comboboxChannelY->clear();

    buttonAddToPlot->setEnabled(false);

    comboboxChannelX->blockSignals(true);
    comboboxChannelY->blockSignals(true);

    bool channelsXValid = false;
    bool channelsYValid = false;

    if(!selectedRuns().isEmpty())
    {
        if((comboboxSignalType1->getCurrentText() == identifier_settings || comboboxSignalType1->getCurrentText() == identifier_udp)
                && !(comboboxParameter1->getCurrentText() == identifier_pmt_gain_voltage || comboboxParameter1->getCurrentText() == identifier_pmt_reference_voltage))
        {
            comboboxChannelX->addItem("-", 0);
            channelsXValid = true;
        }
        else
        {
            int channelXCount = 0;

            Signal::SType xSType = Signal::RAW;

            if(comboboxSignalType1->getCurrentText() == "raw")
                xSType = Signal::RAW;
            else if(comboboxSignalType1->getCurrentText() == "absolute")
                xSType = Signal::ABS;
            else if(comboboxSignalType1->getCurrentText() == "temperature")
                xSType = Signal::TEMPERATURE;

            QSet<int> channelIDSet;

            for(int i = 0; i < selectedRuns().size(); i++)
            {
                channelIDSet = channelIDSet + selectedRuns().at(i)->channelIDs(xSType).toSet();
                if(selectedRuns().at(i)->getNoChannels(xSType) > channelXCount)
                    channelXCount = selectedRuns().at(i)->getNoChannels(xSType);
            }

            QList<int> channelIDList = channelIDSet.toList();
            std::sort(channelIDList.begin(), channelIDList.end());

            for(int i = 0; i < channelIDList.size(); i++)
            {
                if(xSType == Signal::RAW || xSType == Signal::ABS)
                    comboboxChannelX->addItem(QString("Channel %0").arg(channelIDList.at(i)), channelIDList.at(i));
                else if(xSType == Signal::TEMPERATURE)
                    comboboxChannelX->addItem(QString("T%0").arg(channelIDList.at(i)), channelIDList.at(i));
            }

            if(channelXCount > 0)
                channelsXValid = true;
        }

        if((comboboxSignalType2->getCurrentText() == identifier_settings || comboboxSignalType2->getCurrentText() == identifier_udp)
                && !(comboboxParameter2->getCurrentText() == identifier_pmt_gain_voltage || comboboxParameter2->getCurrentText() == identifier_pmt_reference_voltage))
        {
            comboboxChannelY->addItem("-", 0);
            channelsYValid = true;
        }
        else
        {
            int channelYCount = 0;

            Signal::SType ySType = Signal::RAW;

            if(comboboxSignalType2->getCurrentText() == "raw")
                ySType = Signal::RAW;
            else if(comboboxSignalType2->getCurrentText() == "absolute")
                ySType = Signal::ABS;
            else if(comboboxSignalType2->getCurrentText() == "temperature")
                ySType = Signal::TEMPERATURE;

            QSet<int> channelIDSet;

            for(int i = 0; i < selectedRuns().size(); i++)
            {
                channelIDSet = channelIDSet + selectedRuns().at(i)->channelIDs(ySType).toSet();
                if(selectedRuns().at(i)->getNoChannels(ySType) > channelYCount)
                    channelYCount = selectedRuns().at(i)->getNoChannels(ySType);
            }

            QList<int> channelIDList = channelIDSet.toList();
            std::sort(channelIDList.begin(), channelIDList.end());

            for(int i = 0; i < channelIDList.size(); i++)
            {
                if(ySType == Signal::RAW || ySType == Signal::ABS)
                    comboboxChannelY->addItem(QString("Channel %0").arg(channelIDList.at(i)), channelIDList.at(i));
                else if(ySType == Signal::TEMPERATURE)
                    comboboxChannelY->addItem(QString("T%0").arg(channelIDList.at(i)), channelIDList.at(i));
            }

            /*for(int i = 0; i < selectedRuns().size(); i++)
            {
                if(selectedRuns().at(i)->getNoChannels(ySType) > channelYCount)
                    channelYCount = selectedRuns().at(i)->getNoChannels(ySType);
            }

            for(int i = 1; i <= channelYCount; i++)
                comboboxChannelY->addItem(QString("Channel %0").arg(i), i);*/

            if(channelYCount > 0)
                channelsYValid = true;
        }

        if(channelsXValid && channelsYValid)
            buttonAddToPlot->setEnabled(true);
    }

    comboboxChannelX->blockSignals(false);
    comboboxChannelY->blockSignals(false);
}


void ParameterAnalysis::onButtonAddToPlotReleased()
{
    if(((comboboxParameter1->getCurrentText() == identifier_avg_data_1 || comboboxParameter2->getCurrentText() == identifier_avg_data_1)
        && (xStart == 0.0 && xEnd == 0.0)) || ((comboboxParameter1->getCurrentText() == identifier_avg_data_2 ||
        comboboxParameter2->getCurrentText() == identifier_avg_data_2) && (xStart1 == 0.0 && xEnd1 == 0.0)))
    {
        QMessageBox msgBox;
        msgBox.setText("Please select a range for average calculation before adding any averaged parameters.");
        msgBox.exec();
        return;
    }

    buttonAddToPlot->setEnabled(false);

    int channelX = comboboxChannelX->currentData().toInt();
    int channelY = comboboxChannelY->currentData().toInt();

    QString parameterX;
    QString parameterY;

    QVariant sourceX;
    QVariant sourceY;

    if(comboboxSignalType1->getCurrentText() == identifier_udp)
    {
        parameterX = identifier_udp;
        sourceX = comboboxParameter1->getCurrentText();
    }
    else if(comboboxSignalType1->getCurrentText() == identifier_settings)
    {
        parameterX = comboboxParameter1->getCurrentText();
    }
    else if(comboboxSignalType1->getCurrentText() == identifier_raw)
    {
        parameterX = comboboxParameter1->getCurrentText();
        sourceX = qVariantFromValue(Signal::RAW);
    }
    else if(comboboxSignalType1->getCurrentText() == identifier_absolute)
    {
        parameterX = comboboxParameter1->getCurrentText();
        sourceX = qVariantFromValue(Signal::ABS);
    }
    else if(comboboxSignalType1->getCurrentText() == identifier_temperature)
    {
        parameterX = comboboxParameter1->getCurrentText();
        sourceX = qVariantFromValue(Signal::TEMPERATURE);
    }

    if(comboboxSignalType2->getCurrentText() == identifier_udp)
    {
        parameterY = identifier_udp;
        sourceY = comboboxParameter2->getCurrentText();
    }
    else if(comboboxSignalType2->getCurrentText() == identifier_settings)
    {
        parameterY = comboboxParameter2->getCurrentText();
    }
    else if(comboboxSignalType2->getCurrentText() == identifier_raw)
    {
        parameterY = comboboxParameter2->getCurrentText();
        sourceY = qVariantFromValue(Signal::RAW);
    }
    else if(comboboxSignalType2->getCurrentText() == identifier_absolute)
    {
        parameterY = comboboxParameter2->getCurrentText();
        sourceY = qVariantFromValue(Signal::ABS);
    }
    else if(comboboxSignalType2->getCurrentText() == identifier_temperature)
    {
        parameterY = comboboxParameter2->getCurrentText();
        sourceY = qVariantFromValue(Signal::TEMPERATURE);
    }

    ParameterAnalysisCurve *pac = buildCurveObject(parameterX, parameterY, sourceX, sourceY, channelX, channelY);

    curves.push_back(pac);
    updater->lastCurveOnly();
    updater->start();

    buttonAddToPlot->setEnabled(true);
}


void ParameterAnalysis::onButtonAddAllChannelClicked()
{
    if(((comboboxParameter1->getCurrentText() == identifier_avg_data_1 || comboboxParameter2->getCurrentText() == identifier_avg_data_1)
        && (xStart == 0.0 && xEnd == 0.0)) || ((comboboxParameter1->getCurrentText() == identifier_avg_data_2 ||
        comboboxParameter2->getCurrentText() == identifier_avg_data_2) && (xStart1 == 0.0 && xEnd1 == 0.0)))
    {
        QMessageBox msgBox;
        msgBox.setText("Please select a range for average calculation before adding any averaged parameters.");
        msgBox.exec();
        return;
    }

    buttonAddAllChannel->setEnabled(false);

    QString parameterX;
    QString parameterY;

    QVariant sourceX;
    QVariant sourceY;

    if(comboboxSignalType1->getCurrentText() == identifier_udp)
    {
        parameterX = identifier_udp;
        sourceX = comboboxParameter1->getCurrentText();
    }
    else if(comboboxSignalType1->getCurrentText() == identifier_settings)
    {
        parameterX = comboboxParameter1->getCurrentText();
    }
    else if(comboboxSignalType1->getCurrentText() == identifier_raw)
    {
        parameterX = comboboxParameter1->getCurrentText();
        sourceX = qVariantFromValue(Signal::RAW);
    }
    else if(comboboxSignalType1->getCurrentText() == identifier_absolute)
    {
        parameterX = comboboxParameter1->getCurrentText();
        sourceX = qVariantFromValue(Signal::ABS);
    }
    else if(comboboxSignalType1->getCurrentText() == identifier_temperature)
    {
        parameterX = comboboxParameter1->getCurrentText();
        sourceX = qVariantFromValue(Signal::TEMPERATURE);
    }

    if(comboboxSignalType2->getCurrentText() == identifier_udp)
    {
        parameterY = identifier_udp;
        sourceY = comboboxParameter2->getCurrentText();
    }
    else if(comboboxSignalType2->getCurrentText() == identifier_settings)
    {
        parameterY = comboboxParameter2->getCurrentText();
    }
    else if(comboboxSignalType2->getCurrentText() == identifier_raw)
    {
        parameterY = comboboxParameter2->getCurrentText();
        sourceY = qVariantFromValue(Signal::RAW);
    }
    else if(comboboxSignalType2->getCurrentText() == identifier_absolute)
    {
        parameterY = comboboxParameter2->getCurrentText();
        sourceY = qVariantFromValue(Signal::ABS);
    }
    else if(comboboxSignalType2->getCurrentText() == identifier_temperature)
    {
        parameterY = comboboxParameter2->getCurrentText();
        sourceY = qVariantFromValue(Signal::TEMPERATURE);
    }

    if(comboboxChannelX->currentData().toInt() != 0 && comboboxChannelY->currentData().toInt() != 0)
    {
        for(int i = 0; i < comboboxChannelX->count(); i++)
        {
            if(comboboxChannelY->findText(comboboxChannelX->itemText(i)) != -1)
            {
                int channelX = comboboxChannelX->itemData(i).toInt();
                int channelY = channelX;
                ParameterAnalysisCurve *pac = buildCurveObject(parameterX, parameterY, sourceX, sourceY, channelX, channelY);
                curves.push_back(pac);
            }
        }
    }
    else if(comboboxChannelX->currentData().toInt() == 0 && comboboxChannelY->currentData().toInt() != 0)
    {
        for(int i = 0; i < comboboxChannelY->count(); i++)
        {
            int channelY = comboboxChannelY->itemData(i).toInt();
            ParameterAnalysisCurve *pac = buildCurveObject(parameterX, parameterY, sourceX, sourceY, 0, channelY);
            curves.push_back(pac);
        }
    }
    else if(comboboxChannelX->currentData().toInt() != 0 && comboboxChannelY->currentData().toInt() == 0)
    {
        for(int i = 0; i < comboboxChannelX->count(); i++)
        {
            int channelX = comboboxChannelX->itemData(i).toInt();
            ParameterAnalysisCurve *pac = buildCurveObject(parameterX, parameterY, sourceX, sourceY, channelX, 0);
            curves.push_back(pac);
        }
    }
    else if(comboboxChannelX->currentData().toInt() == 0 && comboboxChannelY->currentData().toInt() == 0)
    {
        ParameterAnalysisCurve *pac = buildCurveObject(parameterX, parameterY, sourceX, sourceY, 0, 0);
        curves.push_back(pac);
    }

    if(updater->isRunning())
        updater->setSelectionChanged();
    else
        updater->start();

    buttonAddAllChannel->setEnabled(true);
}


void ParameterAnalysis::onCheckboxLinkChannelStateChanged(int state)
{
    // state:
    // true:    channel y (disabled) and button all channels(enabled)
    // false:   channel y (enabled) and button all channels(disabled)
    if(state == Qt::Checked)
    {
        comboboxChannelY->blockSignals(true);

        comboboxChannelY->setEnabled(false);
        int channel = comboboxChannelX->currentData().toInt();

        for(int i = 0; i < comboboxChannelY->count(); i++)
            if(channel == comboboxChannelY->itemData(i))
                comboboxChannelY->setCurrentIndex(i);

        comboboxChannelY->blockSignals(false);
        buttonAddAllChannel->setEnabled(true);
    }
    else
    {
        comboboxChannelY->setEnabled(true);
        buttonAddAllChannel->setEnabled(false);
    }
}


void ParameterAnalysis::updateView()
{
    updateColors();
    updateCurveTable();
    updatePlot();
    updateDataTable();
}


void ParameterAnalysis::onTableCurveDataCellChanged(int row, int column)
{
    if(column == 1 && row < curves.size())
    {
        curves.at(row)->curve->setTitle(tableCurves->item(row, column)->text());
        paramPlot->plot()->replot();
    }
}


void ParameterAnalysis::onPACVisibilityChanged()
{
    updatePlot();
    updateDataTable();
}


void ParameterAnalysis::onPACColorChanged()
{
    updateView();
}


void ParameterAnalysis::onCheckboxAutoPlotScalingStateChanged(int state)
{
    if(state == Qt::Checked)
        paramPlot->setZoomMode(BasePlotWidgetQwt::ZOOM_RESET);
}


void ParameterAnalysis::removeCurve(ParameterAnalysisCurve *curve)
{
    int index = curves.indexOf(curve);
    if(index != -1)
    {
        curves.removeAt(index);
        paramPlot->unregisterCurve(curve->curve);
        updateView();
        delete curve;
    }
}


void ParameterAnalysis::onUpdaterProgress(int value)
{
    progressBarUpdater->setValue(value);
}


void ParameterAnalysis::onUpdaterFinished()
{
    updateView();
    progressBarUpdater->setValue(100);
    buttonUpdateCurves->setEnabled(true);
}


void ParameterAnalysis::onGuiSettingsChanged()
{
    QVariant position;
    if(Core::instance()->guiSettings->getSplitterPosition(identifier_splitterLeft, position))
        splitterLeft->restoreState(position.toByteArray());
    if(Core::instance()->guiSettings->getSplitterPosition(identifier_splitterRight, position))
        splitterRight->restoreState(position.toByteArray());
    if(Core::instance()->guiSettings->getSplitterPosition(identifier_splitterMiddle, position))
        horizontalSplitter->restoreState(position.toByteArray());
}


void ParameterAnalysis::onSplitterMoved()
{
    if(QObject::sender() == splitterLeft)
        Core::instance()->guiSettings->setSplitterPosition(identifier_splitterLeft, splitterLeft->saveState());
    else if(QObject::sender() == splitterRight)
        Core::instance()->guiSettings->setSplitterPosition(identifier_splitterRight, splitterRight->saveState());
    else if(QObject::sender() == horizontalSplitter)
        Core::instance()->guiSettings->setSplitterPosition(identifier_splitterMiddle, horizontalSplitter->saveState());
}


//--- ParameterAnalysisCurve class implementation ---


ParameterAnalysisCurve::ParameterAnalysisCurve(QObject *parent) : QObject(parent)
{
    buttonVisibility = new QToolButton();
    buttonVisibility->setIcon(QIcon(Core::rootDir + "resources/icons/visible1_32.png"));
    buttonVisibility->setStyleSheet("QToolButton { border-style: none}");

    buttonRemove = new QPushButton("Remove");
    buttonColor = new QPushButton("Color");

    visible = true;
    ownColor = false;
    color = QColor(Qt::black);

    //parameterXudp = false;
    //parameterYudp = false;

    connect(buttonVisibility, SIGNAL(clicked(bool)), SLOT(onButtonVisibilityClicked()));
    connect(buttonRemove, SIGNAL(clicked(bool)), SLOT(onButtonRemoveReleased()));
    connect(buttonColor, SIGNAL(clicked(bool)), SLOT(onButtonColorReleased()));
}


void ParameterAnalysisCurve::onButtonVisibilityClicked()
{
    visible = !visible;
    if(visible)
        buttonVisibility->setIcon(QIcon(Core::rootDir + "resources/icons/visible1_32.png"));
    else
        buttonVisibility->setIcon(QIcon(Core::rootDir + "resources/icons/visible1gray_32.png"));

    emit visibilityChanged();
}


void ParameterAnalysisCurve::onButtonRemoveReleased()
{
    emit removeRequest(this);
}


void ParameterAnalysisCurve::onButtonColorReleased()
{
    QColorDialog *colorDialog = new QColorDialog;
    colorDialog->setCurrentColor(color);
    connect(colorDialog, SIGNAL(colorSelected(QColor)), SLOT(onColorSelected(QColor)));
    colorDialog->open();
}


void ParameterAnalysisCurve::onColorSelected(const QColor &color)
{
    ownColor = true;
    this->color = color;

    curve->setPen(QPen(color, 1, Qt::DashDotDotLine));
    QwtSymbol *s = new QwtSymbol(QwtSymbol::Diamond);
    s->setSize(6);
    s->setColor(color);
    curve->setSymbol(s);

    emit colorChanged();
}

