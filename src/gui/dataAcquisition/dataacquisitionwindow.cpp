#include "dataacquisitionwindow.h"

#include <iostream>

#include <QLabel>
#include <QString>
#include <QFileDialog>
#include <QSpacerItem>
#include <QMessageBox>

#include "./core.h"
#include "./logging/statusmessagewidget.h"
#include "da_triggerdialog.h"
#include "../deviceManager/devicemanagerwidget.h"
#include "../signalEditor/memusagewidget.h"


DataAcquisitionWindow::DataAcquisitionWindow(QWidget *parent) : QWidget(parent), blockSequenceWorker(this)
{
    MSG_DETAIL_1("init DataAcquisitionWindow");

    //Base data init
    dataModel = Core::instance()->dataModel();
    picoscopeSettings = Core::instance()->psSettings;

    //Build toolbar
    rtbRibbonToolbar = new QToolBar;

    // RIBBON PICOSCOPE
    RibbonToolBox *rbtDevice = new RibbonToolBox("PICOSCOPE", this);
    actionOpenDevice = new QAction("Open Device", this);
    actionCloseDevice = new QAction("Close Device", this);
    rbtDevice->addAction(actionOpenDevice, 0, 0);
    rbtDevice->addAction(actionCloseDevice, 1, 0);

    // RIBBON STREAMING
    rbtStreamingMode = new RibbonToolBox("STREAMING", this);

    actionStartStreaming = new QAction("Start Streaming", this);
    actionStopStreaming = new QAction("Stop Streaming", this);
    actionClearStreamingAvg = new QAction("Clear Average", this);
    rbtStreamingMode->addAction(actionStartStreaming, 0, 0);
    rbtStreamingMode->addAction(actionStopStreaming, 1, 0);
    rbtStreamingMode->addAction(actionClearStreamingAvg, 2, 0);

    //groupStreamingMode = new QGroupBox(this);
    buttongroupStreamingMode = new QButtonGroup(this);
    rbStreamingSingleMeasurement = new QRadioButton("Show Single", this);
    rbStreamingAverage = new QRadioButton("Show Average", this);
    rbStreamingBoth = new QRadioButton("Show Both", this);
    buttongroupStreamingMode->addButton(rbStreamingSingleMeasurement);
    buttongroupStreamingMode->addButton(rbStreamingAverage);
    buttongroupStreamingMode->addButton(rbStreamingBoth);
    rbtStreamingMode->addWidget(rbStreamingSingleMeasurement, 0, 1);
    rbtStreamingMode->addWidget(rbStreamingAverage, 1, 1);
    rbtStreamingMode->addWidget(rbStreamingBoth, 2, 1);

    buttonOverflow = new QPushButton("Overflow", this);
    buttonOverflow->setIcon(QIcon(Core::rootDir + "resources/icons/warning.png"));
    buttonOverflow->setStyleSheet("QPushButton { color: red; background: transparent; }");
    rbtStreamingMode->addWidget(buttonOverflow, 0, 2, 1, 2);
    buttonOverflow->setHidden(true);

    connect(buttongroupStreamingMode, SIGNAL(buttonToggled(QAbstractButton*,bool)), SLOT(onStreamingModeToggled(QAbstractButton*,bool)));

    avgCounterLabel = new QLabel();
    avgCounterLabel->setText(QString("Cache 0 / "));
    spinboxAveragingBufferSize = new QSpinBox(this);
    spinboxAveragingBufferSize->setValue(picoscopeSettings->getAveragingBufferSize());
    spinboxAveragingBufferSize->setMinimum(1);
    spinboxAveragingBufferSize->setMaximum(2000);
    spinboxAveragingBufferSize->setMinimumSize(50,15);
    rbtStreamingMode->addWidget(avgCounterLabel,2, 2);
    rbtStreamingMode->addWidget(spinboxAveragingBufferSize, 2, 3);

    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // RIBBON CURRENT SELECTION
    RibbonToolBox *rbtRunSignalSelection = new RibbonToolBox("CURRENT SELECTION");
    QLabel *labelRun = new QLabel("Run:", rbtRunSignalSelection);
    labelRunName = new QLabel("No run selected", rbtRunSignalSelection);
    QLabel *labelSignal = new QLabel("Signal: ", rbtRunSignalSelection);
    spinboxSignal = new QSpinBox(rbtRunSignalSelection);
    spinboxSignal->setStyleSheet("QSpinBox { width : 50px; }");
    labelSignalCount = new QLabel("");
    rbtRunSignalSelection->addWidget(labelRun, 0, 0);
    rbtRunSignalSelection->addWidget(labelRunName, 0, 1);
    rbtRunSignalSelection->addWidget(labelSignal, 1, 0);
    rbtRunSignalSelection->addWidget(spinboxSignal, 1, 1);
    rbtRunSignalSelection->addWidget(labelSignalCount, 3, 1);

    connect(spinboxAveragingBufferSize, SIGNAL(valueChanged(int)), SLOT(onAveragingBufferSizeChanged(int)));

    // RIBBON REFERENCE SIGNAL WIDGET
    referenceWidget = new DA_ReferenceWidget(this);

    // RIBBON PICOSCOPE STATUS
    RibbonToolBox *rbtStatus = new RibbonToolBox("PICOSCOPE STATUS");
    labelConnection = new QLabel("Closed ");
    labelConnection->setStyleSheet("QLabel { color : red; }");
    buttonError = new QPushButton("Error", rbtStatus);
    buttonError->setStyleSheet("QPushButton { color : grey; background : transparent; }");
    buttonError->setFlat(true);
    buttonError->setEnabled(false);
    labelStreaming = new QLabel("Streaming ", rbtStatus);
    labelStreaming->setStyleSheet("QLabel { color : grey; }");
    labelTriggered = new QLabel("Triggered", rbtStatus);
    labelTriggered->setStyleSheet("QLabel { color : grey }");
    labelRunningBlock = new QLabel("Block Run", rbtStatus);
    labelRunningBlock->setStyleSheet("QLabel { color : grey }");
    labelProcessing = new QLabel("Processing", rbtStatus);
    labelProcessing->setStyleSheet("QLabel { color : grey; }");
    rbtStatus->addWidget(labelConnection, 0, 0);
    rbtStatus->addWidget(buttonError, 1, 0);
    rbtStatus->addWidget(labelStreaming, 0, 1);
    rbtStatus->addWidget(labelTriggered, 0, 2);
    rbtStatus->addWidget(labelRunningBlock, 1, 1);
    rbtStatus->addWidget(labelProcessing, 1, 2);

    //RIBBON DAQ DEVICE MANAGER
#ifdef LIISIM_NIDAQMX
    RibbonToolBox *rbtDAQDeviceManager = new RibbonToolBox("DAQmx DEVICE MANAGER");
    actionShowDeviceManager = new QAction(" S \n h \n o \n w", this);
    labelAnalogOutputState = new QLabel("Analog Out", this);
    labelAnalogOutputState->setAlignment(Qt::AlignCenter);
    labelAnalogOutputState->setStyleSheet("QLabel { color : firebrick; }");
    labelAnalogInputState = new QLabel("Analog In", this);
    labelAnalogInputState->setAlignment(Qt::AlignCenter);
    labelAnalogInputState->setStyleSheet("QLabel { color : firebrick; }");
    labelDigitalOutputState = new QLabel("Digital Out", this);
    labelDigitalOutputState->setAlignment(Qt::AlignCenter);
    labelDigitalOutputState->setStyleSheet("QLabel { color : firebrick; }");

    toolbuttonStartAnalogOutput = new QToolButton(this);
    toolbuttonStartAnalogOutput->setText("Start");
    toolbuttonStartAnalogOutput->setMinimumWidth(60);
    toolbuttonStopAnalogOutput = new QToolButton(this);
    toolbuttonStopAnalogOutput->setText("Stop");
    toolbuttonStopAnalogOutput->setMinimumWidth(60);
    toolbuttonStartAnalogInput = new QToolButton(this);
    toolbuttonStartAnalogInput->setText("Start");
    toolbuttonStartAnalogInput->setMinimumWidth(60);
    toolbuttonStopAnalogInput = new QToolButton(this);
    toolbuttonStopAnalogInput->setText("Stop");
    toolbuttonStopAnalogInput->setMinimumWidth(60);
    toolbuttonStartDigitalOutput = new QToolButton(this);
    toolbuttonStartDigitalOutput->setText("Start");
    toolbuttonStartDigitalOutput->setMinimumWidth(60);
    toolbuttonStopDigitalOutput = new QToolButton(this);
    toolbuttonStopDigitalOutput->setText("Stop");
    toolbuttonStopDigitalOutput->setMinimumWidth(60);

    toolbuttonDigitalOut1 = new QToolButton(this);
    toolbuttonDigitalOut2 = new QToolButton(this);
    toolbuttonDigitalOut3 = new QToolButton(this);
    toolbuttonDigitalOut4 = new QToolButton(this);
    toolbuttonDigitalOut5 = new QToolButton(this);
    toolbuttonDigitalOut6 = new QToolButton(this);
    toolbuttonDigitalOut7 = new QToolButton(this);
    toolbuttonDigitalOut8 = new QToolButton(this);
    toolbuttonDigitalOut9 = new QToolButton(this);

    toolbuttonDigitalOut1->setStyleSheet("QToolButton { color : firebrick; }");
    toolbuttonDigitalOut2->setStyleSheet("QToolButton { color : firebrick; }");
    toolbuttonDigitalOut3->setStyleSheet("QToolButton { color : firebrick; }");
    toolbuttonDigitalOut4->setStyleSheet("QToolButton { color : firebrick; }");
    toolbuttonDigitalOut5->setStyleSheet("QToolButton { color : firebrick; }");
    toolbuttonDigitalOut6->setStyleSheet("QToolButton { color : firebrick; }");
    toolbuttonDigitalOut7->setStyleSheet("QToolButton { color : firebrick; }");
    toolbuttonDigitalOut8->setStyleSheet("QToolButton { color : firebrick; }");
    toolbuttonDigitalOut9->setStyleSheet("QToolButton { color : firebrick; }");

    rbtDAQDeviceManager->addAction(actionShowDeviceManager, 0, 0, 3, 1);
    rbtDAQDeviceManager->addWidget(labelAnalogOutputState, 0, 1);
    rbtDAQDeviceManager->addWidget(toolbuttonStartAnalogOutput, 1, 1);
    rbtDAQDeviceManager->addWidget(toolbuttonStopAnalogOutput, 2, 1);
    rbtDAQDeviceManager->addWidget(labelAnalogInputState, 0, 2);
    rbtDAQDeviceManager->addWidget(toolbuttonStartAnalogInput, 1, 2);
    rbtDAQDeviceManager->addWidget(toolbuttonStopAnalogInput, 2, 2);
    rbtDAQDeviceManager->addWidget(labelDigitalOutputState, 0, 3);
    rbtDAQDeviceManager->addWidget(toolbuttonStartDigitalOutput, 1, 3);
    rbtDAQDeviceManager->addWidget(toolbuttonStopDigitalOutput, 2, 3);

    rbtDAQDeviceManager->addWidget(toolbuttonDigitalOut1, 0, 4);
    rbtDAQDeviceManager->addWidget(toolbuttonDigitalOut2, 1, 4);
    rbtDAQDeviceManager->addWidget(toolbuttonDigitalOut3, 2, 4);
    rbtDAQDeviceManager->addWidget(toolbuttonDigitalOut4, 0, 5);
    rbtDAQDeviceManager->addWidget(toolbuttonDigitalOut5, 1, 5);
    rbtDAQDeviceManager->addWidget(toolbuttonDigitalOut6, 2, 5);
    rbtDAQDeviceManager->addWidget(toolbuttonDigitalOut7, 0, 6);
    rbtDAQDeviceManager->addWidget(toolbuttonDigitalOut8, 1, 6);
    rbtDAQDeviceManager->addWidget(toolbuttonDigitalOut9, 2, 6);

    connect(actionShowDeviceManager, SIGNAL(triggered(bool)), SLOT(onShowDeviceManager()));
    connect(toolbuttonStartAnalogOutput, SIGNAL(released()), Core::instance()->devManager, SLOT(startAnalogOUT()));
    connect(toolbuttonStopAnalogOutput, SIGNAL(released()), Core::instance()->devManager, SLOT(stopAnalogOUT()));
    connect(toolbuttonStartAnalogInput, SIGNAL(released()), Core::instance()->devManager, SLOT(startAnalogIN()));
    connect(toolbuttonStopAnalogInput, SIGNAL(released()), Core::instance()->devManager, SLOT(stopAnalogIN()));
    connect(toolbuttonStartDigitalOutput, SIGNAL(released()), Core::instance()->devManager, SLOT(startDigitalOUT()));
    connect(toolbuttonStopDigitalOutput, SIGNAL(released()), Core::instance()->devManager, SLOT(stopDigitalOUT()));

    connect(toolbuttonDigitalOut1, SIGNAL(released()), SLOT(onDigitalOutTriggered()));
    connect(toolbuttonDigitalOut2, SIGNAL(released()), SLOT(onDigitalOutTriggered()));
    connect(toolbuttonDigitalOut3, SIGNAL(released()), SLOT(onDigitalOutTriggered()));
    connect(toolbuttonDigitalOut4, SIGNAL(released()), SLOT(onDigitalOutTriggered()));
    connect(toolbuttonDigitalOut5, SIGNAL(released()), SLOT(onDigitalOutTriggered()));
    connect(toolbuttonDigitalOut6, SIGNAL(released()), SLOT(onDigitalOutTriggered()));
    connect(toolbuttonDigitalOut7, SIGNAL(released()), SLOT(onDigitalOutTriggered()));
    connect(toolbuttonDigitalOut8, SIGNAL(released()), SLOT(onDigitalOutTriggered()));
    connect(toolbuttonDigitalOut9, SIGNAL(released()), SLOT(onDigitalOutTriggered()));

    connect(Core::instance()->devManager, SIGNAL(digitalOutputChanged()), SLOT(onDigitalOutputChanged()));
#endif

    // RIBBON TRIGGER
    RibbonToolBox *rbtInfo = new RibbonToolBox("TRIGGER", this);
    labelActiveTriggers = new QLabel("No Triggers");
    labelActiveTriggers->setStyleSheet("QLabel { font: bold; }");
    labelActiveTriggers->setAlignment(Qt::AlignCenter);
    labelActiveTriggers->setToolTip("No trigger selected");
    actionChangeTrigger = new QAction("Change", rbtInfo); 
    rbtInfo->addWidget(labelActiveTriggers, 0, 0);
    rbtInfo->addAction(actionChangeTrigger, 1, 0);

    // RIBBON PLOT
    RibbonToolBox *rbtPlot = new RibbonToolBox("PLOT", this);
    actionPlotPan = new QAction("Plot Pan", rbtPlot);
    actionPlotRectZoom = new QAction("Rect Zoom", rbtPlot);
    actionPlotZoomReset = new QAction("Reset Zoom", rbtPlot);
    actionPlotPan->setCheckable(true);
    actionPlotPan->setChecked(true);
    actionPlotRectZoom->setCheckable(true);
    rbtPlot->addAction(actionPlotPan, 0, 0);
    rbtPlot->addAction(actionPlotRectZoom, 0, 1);
    rbtPlot->addAction(actionPlotZoomReset, 1, 1);

    // BUILD TOOLBAR
    //rtbRibbonToolbar->addWidget(rbtSignalData);
    //rtbRibbonToolbar->addSeparator();

    // PICOSCOPE
    rtbRibbonToolbar->addWidget(rbtDevice);
    rtbRibbonToolbar->addSeparator();

    // PICOSCOPE STATUS
    rtbRibbonToolbar->addWidget(rbtStatus);
    rtbRibbonToolbar->addSeparator();

    // STREAMING
    rtbRibbonToolbar->addWidget(rbtStreamingMode);
    rtbRibbonToolbar->addSeparator();

    // CURRENT SELECTION
    rtbRibbonToolbar->addWidget(rbtRunSignalSelection);
    rtbRibbonToolbar->addSeparator();

    // DAQmx DEVICE MANAGER
#ifdef LIISIM_NIDAQMX
    rtbRibbonToolbar->addWidget(rbtDAQDeviceManager);
    rtbRibbonToolbar->addSeparator();
#endif

    // REFERENCE (curves)
    rtbRibbonToolbar->addWidget(referenceWidget);
    rtbRibbonToolbar->addSeparator();
    rtbRibbonToolbar->addWidget(spacer);
    rtbRibbonToolbar->addSeparator();

    // TRIGGER
    rtbRibbonToolbar->addWidget(rbtInfo);
    rtbRibbonToolbar->addSeparator();

    // PLOT
    rtbRibbonToolbar->addWidget(rbtPlot);
    rtbRibbonToolbar->addSeparator();

    // NOTIFICATIONS
    rtbRibbonToolbar->addWidget(Core::instance()->getNotificationManager()->getNotificationCenter(this));
    rtbRibbonToolbar->addSeparator();

    // MEMORY USAGE
    MemUsageWidget* memw = new MemUsageWidget;
    rtbRibbonToolbar->addWidget(memw);

    //Build window
    hbLayout = new QHBoxLayout();
    hbLayout->setMargin(0);
    hSplitter = new QSplitter(Qt::Horizontal, this);

    hbLayout->addWidget(hSplitter);

    // left box
    QSplitter *leftVertSplit = new QSplitter(Qt::Vertical);
    hSplitter->addWidget(leftVertSplit);

    dataTreeView = new DataItemTreeView(DataItemTreeView::DA_VC_GROUPS, dataModel->rootItem());
    dataTreeView->setHeaderLabel("Measurement Runs");
    dataTreeView->setMinimumWidth(255);
    dataTreeView->setMaximumWidth(270);
    leftVertSplit->addWidget(dataTreeView);

    mrunDetailsView = new MRunDetailsWidget(this);
    leftVertSplit->addWidget(mrunDetailsView);    

    //Acquisition settings widget (middle box)
    QWidget *settingsWidget = new QWidget(this);
    gLayout = new QGridLayout(settingsWidget);
    gLayout->setMargin(0);    
    gLayout->setColumnMinimumWidth(1,200);
    settingsWidget->setLayout(gLayout);

    QLabel *lliisettings = new QLabel("LII-Settings of next run");
    cbliisettings = new LIISettingsComboBox(settingsWidget);
    cbliisettings->setMinimumWidth(150);

    gLayout->addWidget(lliisettings,1,0);
    gLayout->addWidget(cbliisettings,1,1);
    gLayout->setAlignment(cbliisettings, Qt::AlignRight | Qt::AlignTop);

    pbRunBlockSequence = new QPushButton("Run Automatic Sequence", settingsWidget);
    pbRunBlockSequence->setMinimumWidth(150);
    gLayout->addWidget(pbRunBlockSequence, 2, 0, 1, 1);
    gLayout->setAlignment(pbRunBlockSequence, Qt::AlignRight | Qt::AlignTop);

    pbRunBlock = new QPushButton("Run Block", settingsWidget);
    pbRunBlock->setMinimumWidth(150);
    gLayout->addWidget(pbRunBlock, 2, 1, 1, 1);
    gLayout->setAlignment(pbRunBlock, Qt::AlignRight | Qt::AlignTop);


    pbRunCBuffer = new QPushButton("Save streaming buffer", settingsWidget);
    pbRunCBuffer->setMinimumWidth(150);
    gLayout->addWidget(pbRunCBuffer, 3, 1, 1, 1);
    gLayout->setAlignment(pbRunCBuffer, Qt::AlignRight | Qt::AlignTop);

    QSpacerItem *settingsSpacer = new QSpacerItem(QSizePolicy::Maximum, QSizePolicy::Maximum, QSizePolicy::Expanding, QSizePolicy::Expanding);
    gLayout->addItem(settingsSpacer, 3, 0, 1, 1);

    hSplitter->addWidget(settingsWidget);

    // signal plot (right box)
    //signalPlot = new SignalPlotWidgetQwt;
    signalPlot = new DataAcquisitionPlotWidget(this);
    //signalPlot->setZoomMode(SignalPlotWidgetQwt::PLOT_PAN);
    signalPlot->setZoomMode(DataAcquisitionPlotWidget::PLOT_PAN);
    signalPlot->setPlotAxisTitles("Time (ns)", "Voltage Signal (V)");
    signalPlot->setMaxLegendColumns(1);

    connect(signalPlot, SIGNAL(plotTypeChanged(BasePlotWidgetQwt::PlotType)), SLOT(onPlotTypeChanged(BasePlotWidgetQwt::PlotType)));

    hSplitter->addWidget(signalPlot);

    hSplitter->setStretchFactor(0, 1);
    hSplitter->setStretchFactor(1, 2);
    hSplitter->setStretchFactor(2, 6);

    setLayout(hbLayout);

    //Connect signals and slots
    connect(actionOpenDevice, SIGNAL(triggered(bool)), this, SLOT(openDevice(bool)));
    connect(actionCloseDevice, SIGNAL(triggered(bool)), this, SLOT(closeDevice(bool)));
    connect(actionStartStreaming, SIGNAL(triggered(bool)), this, SLOT(startStreaming(bool)));
    connect(actionStopStreaming, SIGNAL(triggered(bool)), this, SLOT(stopStreaming(bool)));

    connect(actionPlotPan, SIGNAL(triggered()), this, SLOT(plotZoomSelection()));
    connect(actionPlotRectZoom, SIGNAL(triggered()), this, SLOT(plotZoomSelection()));
    connect(actionPlotZoomReset, SIGNAL(triggered()), this, SLOT(plotZoomSelection()));
    connect(actionChangeTrigger, SIGNAL(triggered(bool)), this, SLOT(onChangeTriggerClicked(bool)));

    connect(cbliisettings,SIGNAL(currentIndexChanged(int)),SLOT(liisettingschanged(int)));

    connect(pbRunBlockSequence, SIGNAL(clicked(bool)), this, SLOT(runBlockSequence()));
    connect(pbRunBlock, SIGNAL(clicked(bool)), this, SLOT(runBlock()));
    connect(pbRunCBuffer, SIGNAL(clicked(bool)), this, SLOT(saveCircularBuffer()));

    connect(picoscopeSettings, SIGNAL(settingsChanged()), this, SLOT(onPsSettingsObjChanged()));


    // create new PicoScope instance
    onPsSettingsObjChanged();

    picoscope = new PicoScope(picoscopeSettings, this);

    // do everything that needed the PicoScope instance
    connect(actionClearStreamingAvg, SIGNAL(triggered(bool)), picoscope, SLOT(clearStreamingAvg()));


    QSplitter* tableSplitter = new QSplitter(Qt::Vertical);
    gLayout->addWidget(tableSplitter,0,0,1,3);

    runSettingsWidget = new DA_RunSettingsWidget();
    tableSplitter->addWidget(runSettingsWidget);

    psSettingsWidget = new PicoScopeSettingsWidget(picoscopeSettings,picoscope);
    psSettingsWidget->updateGUI();

#ifdef LIISIM_NIDAQMX
    //connect(Core::instance()->devManager, SIGNAL(analogInputValues(float,float,float,float)), psSettingsWidget, SLOT(updateGainReferenceVoltage(float,float,float,float)));
    connect(Core::instance()->devManager, SIGNAL(analogInputValue(int,float)), psSettingsWidget, SLOT(updateGainReferenceVoltage(int,float)));
    connect(Core::instance()->devManager, SIGNAL(analogInputStateChanged(bool)), SLOT(onAnalogInputStateChanged(bool)));
    connect(Core::instance()->devManager, SIGNAL(analogOutputStateChanged(bool)), SLOT(onAnalogOutputStateChanged(bool)));
    connect(Core::instance()->devManager, SIGNAL(digitalOutputStateChanged(bool)), SLOT(onDigitalOutputStateChanged(bool)));
#endif
    tableSplitter->addWidget(psSettingsWidget);

    exportSettingsWidget = new DA_ExportSettingsWidget;
    tableSplitter->addWidget(exportSettingsWidget);

    connect(picoscope, SIGNAL(runReady()), SLOT(processBlockReady()));
    connect(picoscope, SIGNAL(processingFinished(MRun*)), SLOT(processBlockProcessingFinished(MRun*)));

    connect(dataTreeView, SIGNAL(selectionChanged(QList<QTreeWidgetItem*>)), SLOT(treeViewChanged(QList<QTreeWidgetItem*>)));

    connect(Core::instance()->guiSettings, SIGNAL(settingsChanged()), SLOT(onGuiSettingsChanged()));

    connect(picoscope, SIGNAL(statusOpenChanged(bool)), SLOT(handlePicoScopeOpenStatusChanged(bool)));
    connect(picoscope, SIGNAL(statusStreamingChanged(bool)), SLOT(handlePicoScopeStreamingStatusChanged(bool)));
    connect(picoscope, SIGNAL(statusTriggeredChanged(bool)), SLOT(handlePicoScopeTriggeredStatusChanged(bool)));
    connect(picoscope, SIGNAL(statusBlockrunChanged(bool)), SLOT(handlePicoScopeBlockrunStatusChanged(bool)));
    connect(picoscope, SIGNAL(statusProcessingChanged(bool)), SLOT(handlePicoScopeProcessingStatusChanged(bool)));
    connect(picoscope, SIGNAL(statusErrorChanged(bool)), SLOT(handlePicoScopeErrorStatusChanged(bool)));

    connect(picoscope, SIGNAL(processingProgress(float)), SLOT(handleProcessingProgress(float)));

    connect(buttonError, SIGNAL(clicked(bool)), SLOT(errorClicked(bool)));
    connect(spinboxSignal, SIGNAL(valueChanged(int)), SLOT(signalSpinnerChanged(int)));

    connect(picoscope, SIGNAL(infoMsg(QString)), SLOT(handlePicoScopeInfoMsg(QString)));
    connect(picoscope, SIGNAL(errorMsg(QString)), SLOT(handlePicoScopeErrorMsg(QString)));

    connect(picoscope, SIGNAL(streamingData(StreamPoint)), SLOT(onStreamingData(StreamPoint)));

    connect(referenceWidget, SIGNAL(referenceRequest()), SLOT(onReferenceRequest()));

    connect(&blockSequenceWorker, SIGNAL(workerStopped()), SLOT(onBlockSequenceWorkerStopped()));
    connect(&blockSequenceWorker, SIGNAL(status(QString)), SLOT(onBlockSequenceWorkerStatusChanged(QString)));

    connect(dataTreeView, SIGNAL(visualEnabledItemsChanged(QList<QTreeWidgetItem*>)), SLOT(onTreeViewVisualEnabledItemsChanged(QList<QTreeWidgetItem*>)));

    connect(psSettingsWidget, SIGNAL(switchStreamSignalLayer(uint,bool)), signalPlot, SLOT(switchStreamSignalLayer(uint,bool)));

    connect(referenceWidget, SIGNAL(referencesChanged(QList<QPair<DA_ReferenceElementWidget*,Signal> >)), signalPlot, SLOT(updateReferences(QList<QPair<DA_ReferenceElementWidget*,Signal> >)));

    connect(psSettingsWidget, SIGNAL(stepSizeChanged(double)), runSettingsWidget->lecw, SLOT(setSpinBoxLaserFluenceSingleStep(double)));

    if(picoscopeSettings->streaming_mode == PSStreamingMode::SingleMeasurement)
        rbStreamingSingleMeasurement->setChecked(true);
    if(picoscopeSettings->streaming_mode == PSStreamingMode::AverageMeasurement)
        rbStreamingAverage->setChecked(true);
    if(picoscopeSettings->streaming_mode == PSStreamingMode::Both)
        rbStreamingBoth->setChecked(true);

    runIdsToPlotWhileStreamingChanged = false;
}


DataAcquisitionWindow::~DataAcquisitionWindow()
{
    //delete all the things
}


void DataAcquisitionWindow::openDevice(bool checked)
{
    picoscope->open();
}


void DataAcquisitionWindow::closeDevice(bool checked)
{
    picoscope->close();
}


void DataAcquisitionWindow::startStreaming(bool checked)
{
    picoscope->startStreaming();
    signalPlot->setViewMode(DataAcquisitionPlotWidget::STREAM);
    scalePlotStreaming();
}


void DataAcquisitionWindow::scalePlotStreaming()
{
    PSRange range = PSRange::R50mV;

    for(int i = 1; i < 5; i++)
    {
        PSChannel channel;
        switch(i)
        {
        case 1: channel = PSChannel::A; break;
        case 2: channel = PSChannel::B; break;
        case 3: channel = PSChannel::C; break;
        case 4: channel = PSChannel::D; break;
        }

        if(picoscopeSettings->channel(channel))
            if(picoscopeSettings->range(channel) > range)
                range = picoscopeSettings->range(channel);
    }

    switch(range)
    {
    case PSRange::R50mV:    signalPlot->setYView(-0.055,  0.055);   break;
    case PSRange::R100mV:   signalPlot->setYView(-0.11,   0.11);    break;
    case PSRange::R200mV:   signalPlot->setYView(-0.22,   0.22);    break;
    case PSRange::R500mV:   signalPlot->setYView(-0.55,   0.55);    break;
    case PSRange::R1V:      signalPlot->setYView(-1.1,    1.1);     break;
    case PSRange::R2V:      signalPlot->setYView(-2.1,    2.1);     break;
    case PSRange::R5V:      signalPlot->setYView(-5.1,    5.1);     break;
    case PSRange::R10V:     signalPlot->setYView(-10.1,   10.1);    break;
    case PSRange::R20V:     signalPlot->setYView(-20.1,   20.1);    break;
    }
}


void DataAcquisitionWindow::stopStreaming(bool checked)
{
    picoscope->stopStreaming();
    signalPlot->setViewMode(DataAcquisitionPlotWidget::STATIC);
}


void DataAcquisitionWindow::onAveragingBufferSizeChanged(int value)
{
    picoscopeSettings->setAveragingBufferSize(value);
}


/**
 * @brief DataAcquisitionWindow::liisettingschanged This slot is execueted when
 * the selection of the current liisettings has changed
 * @param index new combobox index
 */
void DataAcquisitionWindow::liisettingschanged(int index)
{
    // save selection to guisettings to
    Core::instance()->guiSettings->setValue("acquisition","liisettings",cbliisettings->currentText());
    // load available filters in run settings
    runSettingsWidget->cbFilter->setAvailableFilters(cbliisettings->currentLIISettings());
}


void DataAcquisitionWindow::treeViewChanged(QList<QTreeWidgetItem*> selection)
{
    if(selection.size() > 0)
    {
        try
        {
            int sel = selection.last()->data(0, Qt::UserRole).toInt();
            int runId = selection.last()->data(0, Qt::UserRole+1).toInt();
            if(sel == 2)
            {
                signalPlot->detachAllCurves();
                selectedRun = dataModel->mrun(runId);

                if(!selectedRun)
                    return;

                updateToSelectedRun();
            }
            else
            {
                selectedRun = 0;
                signalPlot->detachAllCurves();
                mrunDetailsView->setRun(selectedRun);
                labelRunName->setText("No run selected");
                labelSignalCount->setText("");
                spinboxSignal->setMaximum(0);
            }
        }
        catch(LIISimException e)
        {
            MSG_ERR(e.what());
        }
    }
    else
    {
        selectedRun = 0;
        signalPlot->detachAllCurves();
        mrunDetailsView->setRun(selectedRun);
        labelRunName->setText("No run selected");
        labelSignalCount->setText("");
        spinboxSignal->setMaximum(0);
    }
}


void DataAcquisitionWindow::onTreeViewVisualEnabledItemsChanged(QList<QTreeWidgetItem*> visualEnabledItems)
{
    runIdsToPlotWhileStreaming.clear();

    for(int i = 0; i < visualEnabledItems.size(); i++)
    {
        runIdsToPlotWhileStreaming.append(visualEnabledItems[i]->data(0, Qt::UserRole+1).toInt());
    }

    qDebug() << runIdsToPlotWhileStreaming;

    runIdsToPlotWhileStreamingChanged = true;
}


void DataAcquisitionWindow::updateToSelectedRun()
{
    if(!selectedRun)
        return;

    mrunDetailsView->setRun(selectedRun);

    labelRunName->setText(selectedRun->getName());

    int signalCount = selectedRun->sizeAllMpoints();
    int validSignalCount = selectedRun->sizeValidMpoints();

    labelSignalCount->setText(QString(" (%0 signals, %1 valid)").arg(signalCount).arg(validSignalCount));

    spinboxSignal->setMaximum(signalCount);
    if(signalCount < 1)
        spinboxSignal->setMinimum(0);
    else
        spinboxSignal->setMinimum(1);

    MPoint *point = selectedRun->getPre(spinboxSignal->value() - 1);

    for(int i = 1; i <= point->channelCount(Signal::RAW); i++)
    {
        switch(i)
        {
        case 1:
            signalPlot->setCurrentColor(QColor("red").light());
            signalPlot->addSignal(point->getSignal(1, Signal::RAW), "Channel 1", false);
            break;
        case 2:
            signalPlot->setCurrentColor(QColor("green").light());
            signalPlot->addSignal(point->getSignal(2, Signal::RAW), "Channel 2", false);
            break;
        case 3:
            signalPlot->setCurrentColor(QColor("blue").light());
            signalPlot->addSignal(point->getSignal(3, Signal::RAW), "Channel 3", false);
            break;
        case 4:
            signalPlot->setCurrentColor(QColor("yellow"));
            signalPlot->addSignal(point->getSignal(4, Signal::RAW), "Channel 4", false);
            break;
        }
    }

    /*if(point->getTriggerTime() > 0)
    {
        if((fabs(point->getMinSignalTime(Signal::RAW)) - fabs(point->getTriggerTime())) == 0)
        {
            signalPlot->setTriggerMarker(0, 0);
        }
        else if((fabs(point->getMinSignalTime(Signal::RAW) - fabs(point->getTriggerTime()))) > 0)
        {
            signalPlot->setTriggerMarker(fabs(point->getTriggerTime()) * pow(10, 9), 0);
        }
    }*/

    if(selectedRun->psPresample() > 0 && point->channelCount(Signal::RAW) > 0)
    {
        double triggerTimePoint = (point->getMaxSignalTime(Signal::RAW) * (selectedRun->psPresample() / 100)) * pow(10, 9);
        signalPlot->setTriggerMarker(triggerTimePoint, 0);
    }

    double range = 0.0;
    for(int i = 0; i < selectedRun->getNoChannels(Signal::RAW); i++)
    {
        if(selectedRun->psRange(i+1) > range)
            range = selectedRun->psRange(i+1);
    }
    if(range > 0.0)
    {
        range += (range / 5);
        signalPlot->setYView(-range, range);
    }
    else
    {
        //signalPlot->setZoomMode(SignalPlotWidgetQwt::ZOOM_RESET);
        signalPlot->setZoomMode(DataAcquisitionPlotWidget::ZOOM_RESET);
    }

    /*if(selectedRun->psRange() != PSRange::NONE)
    {
        switch(selectedRun->psRange())
        {
        case PSRange::R50mV:    signalPlot->setYView(-0.055,  0.055);   break;
        case PSRange::R100mV:   signalPlot->setYView(-0.11,   0.11);    break;
        case PSRange::R200mV:   signalPlot->setYView(-0.22,   0.22);    break;
        case PSRange::R500mV:   signalPlot->setYView(-0.55,   0.55);    break;
        case PSRange::R1V:      signalPlot->setYView(-1.1,    1.1);     break;
        case PSRange::R2V:      signalPlot->setYView(-2.1,    2.1);     break;
        case PSRange::R5V:      signalPlot->setYView(-5.1,    5.1);     break;
        case PSRange::R10V:     signalPlot->setYView(-10.1,   10.1);    break;
        case PSRange::R20V:     signalPlot->setYView(-20.1,   20.1);    break;
        }
    }
    else
    {
        signalPlot->setZoomMode(SignalPlotWidgetQwt::ZOOM_RESET);
    }*/
}


void DataAcquisitionWindow::plotZoomSelection()
{
    if(QObject::sender() == actionPlotPan)
    {
        actionPlotRectZoom->setChecked(false);
        actionPlotPan->setChecked(true);
        signalPlot->setZoomMode(DataAcquisitionPlotWidget::PLOT_PAN);
    }
    else if(QObject::sender() == actionPlotRectZoom)
    {
        actionPlotPan->setChecked(false);
        actionPlotRectZoom->setChecked(true);
        signalPlot->setZoomMode(DataAcquisitionPlotWidget::ZOOM_RECT);
    }
    else if(QObject::sender() == actionPlotZoomReset)
    {
        //TODO: set plot scaling according to range
        signalPlot->setZoomMode(DataAcquisitionPlotWidget::ZOOM_RESET);

        if(picoscope->isStreaming())
            scalePlotStreaming();
    }
}


/**
 * @brief DataAcquisitionWindow::onGuiSettingsChanged
 */
void DataAcquisitionWindow::onGuiSettingsChanged()
{
    // re-select last liisettings selection
    cbliisettings->setCurrentText(Core::instance()->guiSettings->value("acquisition","liisettings","").toString());
    // set the available filters from the current liisettings
    runSettingsWidget->cbFilter->setAvailableFilters(cbliisettings->currentLIISettings());

    toolbuttonDigitalOut1->setText(Core::instance()->guiSettings->value("acquisition", "buttonDO1text", "DO 1").toString());
    toolbuttonDigitalOut2->setText(Core::instance()->guiSettings->value("acquisition", "buttonDO2text", "DO 2").toString());
    toolbuttonDigitalOut3->setText(Core::instance()->guiSettings->value("acquisition", "buttonDO3text", "DO 3").toString());
    toolbuttonDigitalOut4->setText(Core::instance()->guiSettings->value("acquisition", "buttonDO4text", "DO 4").toString());
    toolbuttonDigitalOut5->setText(Core::instance()->guiSettings->value("acquisition", "buttonDO5text", "DO 5").toString());
    toolbuttonDigitalOut6->setText(Core::instance()->guiSettings->value("acquisition", "buttonDO6text", "DO 6").toString());
    toolbuttonDigitalOut7->setText(Core::instance()->guiSettings->value("acquisition", "buttonDO7text", "DO 7").toString());
    toolbuttonDigitalOut8->setText(Core::instance()->guiSettings->value("acquisition", "buttonDO8text", "DO 8").toString());
    toolbuttonDigitalOut9->setText(Core::instance()->guiSettings->value("acquisition", "buttonDO9text", "DO 9").toString());

    if(Core::instance()->guiSettings->hasEntry("dataacquisition", "plottype"))
    {
        switch(Core::instance()->guiSettings->value("dataacquisition", "plottype", 0).toUInt())
        {
        case 0: signalPlot->setPlotType(DataAcquisitionPlotWidget::LINE_CROSSES); break;
        case 1: signalPlot->setPlotType(DataAcquisitionPlotWidget::LINE); break;
        case 2: signalPlot->setPlotType(DataAcquisitionPlotWidget::DOTS_SMALL); break;
        case 3: signalPlot->setPlotType(DataAcquisitionPlotWidget::DOTS_MEDIUM); break;
        case 4: signalPlot->setPlotType(DataAcquisitionPlotWidget::DOTS_LARGE); break;
        }
    }
}


void DataAcquisitionWindow::runBlockSequence()
{
    if(!blockSequenceWorker.isRunning())
    {
        pbRunBlockSequence->setText("Stop Block Sequence");
        pbRunBlockSequence->setStyleSheet("QPushButton { color: crimson }");
        runSettingsWidget->setEnabled(false);
        exportSettingsWidget->setEnabled(false);
        psSettingsWidget->setEnabled(false);
        pbRunBlock->setEnabled(false);
        cbliisettings->setEnabled(false);
        rbtStreamingMode->setEnabled(false);
        blockSequenceWorker.start();
    }
    else
    {
        if(!blockSequenceWorker.isStopping())
            blockSequenceWorker.stop();
    }
    //qDebug() << "DataAcquisition: start runBlockSequence";
}


void DataAcquisitionWindow::saveCircularBuffer()
{
    int channelCount = 0;

    if(picoscopeSettings->channel(PSChannel::A))
        channelCount++;
    if(picoscopeSettings->channel(PSChannel::B))
        channelCount++;
    if(picoscopeSettings->channel(PSChannel::C))
        channelCount++;
    if(picoscopeSettings->channel(PSChannel::D))
        channelCount++;

    if(channelCount > cbliisettings->currentLIISettings().channels.size())
    {
        QMessageBox msgbox;
        msgbox.setText("Warning:");
        msgbox.setInformativeText(QString("The current LIISettings can only accommodate %0 channels, but %1 channels are selected for acquisition. No data acquisition can be performed.")
                                  .arg(cbliisettings->currentLIISettings().channels.size()).arg(channelCount));
        msgbox.exec();
        return;
    }

    LIISettings liiSettings = cbliisettings->currentLIISettings();
    channelCount = liiSettings.channels.size();

    MRunGroup *group = 0;
    bool newGroupCreated = false;

    if(runSettingsWidget->cbMRunGroup->currentData().toInt() == -1)
    {
        group = new MRunGroup("New Group");
        Core::instance()->dataModel()->registerGroup(group);
        newGroupCreated = true;
    }
    else
    {
        group = Core::instance()->dataModel()->group(runSettingsWidget->cbMRunGroup->currentData().toInt());
        qDebug() << "ID" << runSettingsWidget->cbMRunGroup->currentData().toInt() << "Title" << group->title();
    }

    QString runname = exportSettingsWidget->runName();
    exportSettingsWidget->generateRunName();

    MRun *run = new MRun(runname, channelCount, group);

    run->setLiiSettings(liiSettings);
    run->setDescription(runSettingsWidget->leDescription->toPlainText());
    run->setFilter(runSettingsWidget->cbFilter->currentText());
    run->setLaserFluence(runSettingsWidget->getLaserFluence());
    run->userDefinedParameters = runSettingsWidget->getParameterList();

    if(runSettingsWidget->lecw->isIOEnabled())
    {
        qDebug() << "--- io is enabled ---";
        if(runSettingsWidget->lecw->isSetpointValid())
            run->setLaserSetpoint(runSettingsWidget->lecw->getSetpoint());

        if(runSettingsWidget->lemw->isPositionValid())
            run->setLaserPosition(runSettingsWidget->lemw->getPosition());
    }

    try
    {
        if(!Core::instance()->devManager->getAnalogInputIsValid())
            MSG_INFO("Analog input not active, values are not saved");

        int channelCounter = 1;

        for(int i = 0; i < 4; i++)
        {
            PSChannel channel;
            float gain;
            switch(i+1)
            {
            case 1: channel = PSChannel::A; gain = psSettingsWidget->gainA(); break;
            case 2: channel = PSChannel::B; gain = psSettingsWidget->gainB(); break;
            case 3: channel = PSChannel::C; gain = psSettingsWidget->gainC(); break;
            case 4: channel = PSChannel::D; gain = psSettingsWidget->gainD(); break;
            }
            if(Core::instance()->psSettings->channel(channel))
            {
                run->setPSRange(channelCounter, PicoScopeCommon::PSRangeToDouble(Core::instance()->psSettings->range(channel)));
                run->setPSOffset(channelCounter, Core::instance()->psSettings->offset(channel));
                run->setPmtGainVoltage(channelCounter, gain);
                if(Core::instance()->devManager->getAnalogInputIsValid())
                    run->setPmtReferenceGainVoltage(channelCounter, Core::instance()->devManager->getAnalogInputAverageValue(i+1));
                channelCounter++;
            }
        }
    }
    catch(LIISimException e)
    {
        MESSAGE(e.what(),e.type());
    }

    PSRange range = PSRange::R50mV;
    for(int i = 1; i < 5; i++)
    {
        PSChannel channel;
        switch(i)
        {
        case 1: channel = PSChannel::A; break;
        case 2: channel = PSChannel::B; break;
        case 3: channel = PSChannel::C; break;
        case 4: channel = PSChannel::D; break;
        }
        if(Core::instance()->psSettings->channel(channel))
            if(Core::instance()->psSettings->range(channel) > range)
                range = Core::instance()->psSettings->range(channel);
    }
    run->setPSRange(range);
    run->setPSCoupling(Core::instance()->psSettings->coupling());
    run->setPSOffset(Core::instance()->psSettings->offset(PSChannel::A)); //TODO
    run->setPSCollectionTime(Core::instance()->psSettings->collectionTime());
    run->setPSPresample(Core::instance()->psSettings->presamplePercentage());
    run->setPSSampleInterval(Core::instance()->psSettings->sampleInterval());

    QList<double> gainList = psSettingsWidget->getGainList();

    bool bufferContentAsAverage = false;
    if(psSettingsWidget->averageCaptures())
    {
        bufferContentAsAverage = true;
        run->setAcquisitionMode("Streaming Average");
    }
    else
    {
        run->setAcquisitionMode("Streaming");
    }

    try
    {
        picoscope->getStreamBufferContent(run, bufferContentAsAverage);
    }
    catch(LIISimException &e)
    {
        if(e.type() == LIISimMessageType::ERR)
        {
            QMessageBox msgbox;
            msgbox.setText("Error:");
            msgbox.setInformativeText(e.what());
            msgbox.exec();
            //we got an error, so we assume that the mrun data is empty or incorrect and
            //remove it, also remove the group, if created in this context
            delete run;
            if(newGroupCreated)
                group->deleteLater();

            return;
        }
        else
            throw e;
    }

    if(run->parentItem())
    {
        run->parentItem()->insertChild(run);
    }

    dataModel->registerMRun(run);

    selectedRun = run;

    updateToSelectedRun();

    if(exportSettingsWidget->autoSave())
    {
        // generate export information
        SignalIORequest rq = exportSettingsWidget->generateExportRequest(run, exportSettingsWidget->saveStdev());

        // save signal data
        Core::instance()->getSignalManager()->exportSignalsManager(rq);
    }
}


void DataAcquisitionWindow::runBlock()
{
    isBusy = true;

    unsigned int channelCount = 0;
    if(picoscopeSettings->channel(PSChannel::A))
        channelCount++;
    if(picoscopeSettings->channel(PSChannel::B))
        channelCount++;
    if(picoscopeSettings->channel(PSChannel::C))
        channelCount++;
    if(picoscopeSettings->channel(PSChannel::D))
        channelCount++;

    if(channelCount > cbliisettings->currentLIISettings().channels.size())
    {
        QMessageBox msgbox;
        msgbox.setText("Warning:");
        msgbox.setInformativeText(QString("The current LIISettings can only accommodate %0 channels, but %1 channels are selected for acquisition. No data acquisition can be performed.")
                                  .arg(cbliisettings->currentLIISettings().channels.size()).arg(channelCount));
        msgbox.exec();
    }
    else
    {
        // stop streaming
        picoscope->stopStreaming();
        // refresh filename if checkbox is checked
        exportSettingsWidget->autoRefresh();

        picoscope->run();

#ifdef PICOSCOPE_TEST_MODE
    LIISettings liiSettings = cbliisettings->currentLIISettings();
    int chCount = liiSettings.channels.size();

    MRunGroup *group = 0;

    if(runSettingsWidget->cbMRunGroup->currentData().toInt() == -1)
    {
        group = new MRunGroup("New Group");
        Core::instance()->dataModel()->registerGroup(group);
    }
    else
    {
        group = Core::instance()->dataModel()->group(runSettingsWidget->cbMRunGroup->currentData().toInt());
        qDebug() << "ID" << runSettingsWidget->cbMRunGroup->currentData().toInt() << "Title" << group->title();
    }

    MRun *run = new MRun(exportSettingsWidget->runName(), chCount, group);
    exportSettingsWidget->generateRunName();

    run->setLiiSettings(liiSettings);
    run->setDescription(runSettingsWidget->leDescription->toPlainText());
    run->setFilter(runSettingsWidget->cbFilter->currentText());
    run->setLaserFluence(runSettingsWidget->getLaserFluence());
    run->userDefinedParameters = runSettingsWidget->getParameterList();

    run->setAcquisitionMode("RunBlockTest");

    if(runSettingsWidget->lecw->isIOEnabled())
    {
        if(runSettingsWidget->lecw->isSetpointValid())
            run->setLaserSetpoint(runSettingsWidget->lecw->getSetpoint());

        if(runSettingsWidget->lemw->isPositionValid())
            run->setLaserPosition(runSettingsWidget->lemw->getPosition());
    }

    try
    {
        if(!Core::instance()->devManager->getAnalogInputIsValid())
            MSG_INFO("Analog input not active, values are not saved");

        int channelCounter = 1;

        for(int i = 0; i < 4; i++)
        {
            PSChannel channel;
            float gain;
            switch(i+1)
            {
            case 1: channel = PSChannel::A; gain = psSettingsWidget->gainA(); break;
            case 2: channel = PSChannel::B; gain = psSettingsWidget->gainB(); break;
            case 3: channel = PSChannel::C; gain = psSettingsWidget->gainC(); break;
            case 4: channel = PSChannel::D; gain = psSettingsWidget->gainD(); break;
            }
            if(Core::instance()->psSettings->channel(channel))
            {
                run->setPSRange(channelCounter, PicoScopeCommon::PSRangeToDouble(Core::instance()->psSettings->range(channel)));
                run->setPSOffset(channelCounter, Core::instance()->psSettings->offset(channel));
                run->setPmtGainVoltage(channelCounter, gain);
                if(Core::instance()->devManager->getAnalogInputIsValid())
                    run->setPmtReferenceGainVoltage(channelCounter, Core::instance()->devManager->getAnalogInputAverageValue(i+1));
                channelCounter++;
            }
        }
    }
    catch(LIISimException e)
    {
        MESSAGE(e.what(),e.type());
    }

    PSRange range = PSRange::R50mV;
    for(int i = 1; i < 5; i++)
    {
        PSChannel channel;
        switch(i)
        {
        case 1: channel = PSChannel::A; break;
        case 2: channel = PSChannel::B; break;
        case 3: channel = PSChannel::C; break;
        case 4: channel = PSChannel::D; break;
        }
        if(Core::instance()->psSettings->channel(channel))
            if(Core::instance()->psSettings->range(channel) > range)
                range = Core::instance()->psSettings->range(channel);
    }
    run->setPSRange(range);
    run->setPSCoupling(Core::instance()->psSettings->coupling());
    run->setPSOffset(Core::instance()->psSettings->offset(PSChannel::A)); //TODO
    run->setPSCollectionTime(Core::instance()->psSettings->collectionTime());
    //run->setPSSampleIntervall(Core::instance()->psSettings->sampleIntervall());
    run->setPSPresample(Core::instance()->psSettings->presamplePercentage());
    run->setPSSampleInterval(Core::instance()->psSettings->sampleInterval());

    QList<double> gainList = psSettingsWidget->getGainList();

    //float starttime = (((float)Core::instance()->psSettings->presamplePercentage() / 100) * 5000) * 0.0000001;
    //qDebug() << "starttime =" << starttime;

    Signal signal_1(0, 0.0000001, 0);
    signal_1.channelID = 1;
    for(int i = 0; i < 5000; i++)
    {
        signal_1.data.append(((sin(0.001*(double)(i + 0)) + ((double)(rand() % 100) - 50) / 500) + gainList[0]) * picoscope->getRangeFactorStreamingTest(PSChannel::A));
    }
    Signal signal_2(0, 0.0000001, 0);
    signal_2.channelID = 2;
    for(int i = 0; i < 5000; i++)
    {
        signal_2.data.append(((sin(0.001*(double)(i + 500)) + ((double)(rand() % 100) - 50) / 600) + gainList[1]) * picoscope->getRangeFactorStreamingTest(PSChannel::B));
    }
    Signal signal_3(0, 0.0000001, 0);
    signal_3.channelID = 3;
    for(int i = 0; i < 5000; i++)
    {
        signal_3.data.append(((sin(0.001*(double)(i + 1000)) + ((double)(rand() % 100) - 50) / 700) + gainList[2]) * picoscope->getRangeFactorStreamingTest(PSChannel::C));
    }
    Signal signal_4(0, 0.0000001, 0);
    signal_4.channelID = 4;
    for(int i = 0; i < 5000; i++)
    {
        signal_4.data.append(((sin(0.001*(double)(i + 2000)) + ((double)(rand() % 100) - 50) / 800) + gainList[3]) * picoscope->getRangeFactorStreamingTest(PSChannel::D));
    }

    MPoint *point = run->getCreatePre(0);

    point->setTriggerTime((((double)Core::instance()->psSettings->presamplePercentage() / 100) * 5000) * 0.0000001);

    if(Core::instance()->psSettings->channel(PSChannel::A))
    {
        point->setSignal(signal_1, 1, Signal::RAW);
        point->setSignal(signal_1, 1, Signal::ABS);
    }
    if(Core::instance()->psSettings->channel(PSChannel::B))
    {
        point->setSignal(signal_2, 2, Signal::RAW);
        point->setSignal(signal_2, 2, Signal::ABS);
    }
    if(Core::instance()->psSettings->channel(PSChannel::C))
    {
        point->setSignal(signal_3, 3, Signal::RAW);
        point->setSignal(signal_3, 3, Signal::ABS);
    }
    if(Core::instance()->psSettings->channel(PSChannel::D))
    {
        point->setSignal(signal_4, 4, Signal::RAW);
        point->setSignal(signal_4, 4, Signal::ABS);
    }

    /*float gainA = psSettingsWidget->gainA();
    float gainB = psSettingsWidget->gainB();
    float gainC = psSettingsWidget->gainC();
    float gainD = psSettingsWidget->gainD();

    try
    {
        int addCounter = 1;

        if(Core::instance()->psSettings->channel(PSChannel::A))
        {
            run->setPmtGainVoltage(addCounter, gainA);
            addCounter++;
        }
        if(Core::instance()->psSettings->channel(PSChannel::B))
        {
            run->setPmtGainVoltage(addCounter, gainB);
            addCounter++;
        }
        if(Core::instance()->psSettings->channel(PSChannel::C))
        {
            run->setPmtGainVoltage(addCounter, gainC);
            addCounter++;
        }
        if(Core::instance()->psSettings->channel(PSChannel::D))
        {
            run->setPmtGainVoltage(addCounter, gainD);
        }
    }
    catch(LIISimException e)
    {
        MESSAGE(e.what(),e.type());
    }*/

    if(run->parentItem())
    {
        qDebug() << "Added run to group";
        run->parentItem()->insertChild(run);
    }

    dataModel->registerMRun(run);

    selectedRun = run;

    updateToSelectedRun();

    if(exportSettingsWidget->autoSave())
    {
        // generate export information
        SignalIORequest rq = exportSettingsWidget->generateExportRequest(run);

        // save signal data
        Core::instance()->getSignalManager()->exportSignalsManager(rq);
    }

    emit blockAcquisitionFinished();

#endif
    }
}


void DataAcquisitionWindow::processBlockReady()
{
    LIISettings liiSettings = cbliisettings->currentLIISettings();
    int chCount = liiSettings.channels.size();

    /*if(chCount < 4)
    {
       QString msg = "Please select LIISettings with at least four channels.";
       MSG_STATUS(msg);
       MSG_WARN(msg);
       QMessageBox::warning(0,"Invalid LIISettings!",msg);
       return;
    }*/

    MRunGroup *group = 0;

    if(runSettingsWidget->cbMRunGroup->currentData().toInt() == -1)
    {
        group = new MRunGroup("New Group");
    }
    else
    {
        group = Core::instance()->dataModel()->group(runSettingsWidget->cbMRunGroup->currentData().toInt());
    }

    MRun *run = new MRun(exportSettingsWidget->runName(), chCount, group);
    exportSettingsWidget->autoRefresh();

    run->setLiiSettings(liiSettings);
    run->setDescription(runSettingsWidget->leDescription->toPlainText());
    run->setFilter(runSettingsWidget->cbFilter->currentText());
    run->setLaserFluence(runSettingsWidget->getLaserFluence());
    run->userDefinedParameters = runSettingsWidget->getParameterList();

    if(blockSequenceWorker.isRunning())
        run->setAcquisitionMode("Block Sequence");
    else
        run->setAcquisitionMode("Block");

    if(runSettingsWidget->lecw->isIOEnabled())
    {
        if(runSettingsWidget->lecw->isSetpointValid())
            run->setLaserSetpoint(runSettingsWidget->lecw->getSetpoint());

        if(runSettingsWidget->lemw->isPositionValid())
            run->setLaserPosition(runSettingsWidget->lemw->getPosition());
    }

    try
    {
        if(!Core::instance()->devManager->getAnalogInputIsValid())
            MSG_INFO("Analog input not active, values are not saved");

        int channelCounter = 1;

        for(int i = 0; i < 4; i++)
        {
            PSChannel channel;
            float gain;
            switch(i+1)
            {
            case 1: channel = PSChannel::A; gain = psSettingsWidget->gainA(); break;
            case 2: channel = PSChannel::B; gain = psSettingsWidget->gainB(); break;
            case 3: channel = PSChannel::C; gain = psSettingsWidget->gainC(); break;
            case 4: channel = PSChannel::D; gain = psSettingsWidget->gainD(); break;
            }
            if(Core::instance()->psSettings->channel(channel))
            {
                run->setPSRange(channelCounter, PicoScopeCommon::PSRangeToDouble(Core::instance()->psSettings->range(channel)));
                run->setPSOffset(channelCounter, Core::instance()->psSettings->offset(channel));
                run->setPmtGainVoltage(channelCounter, gain);
                if(Core::instance()->devManager->getAnalogInputIsValid())
                    run->setPmtReferenceGainVoltage(channelCounter, Core::instance()->devManager->getAnalogInputAverageValue(i+1));
                channelCounter++;
            }
        }
    }
    catch(LIISimException e)
    {
        MESSAGE(e.what(),e.type());
    }

    PSRange range = PSRange::R50mV;
    for(int i = 1; i < 5; i++)
    {
        PSChannel channel;
        switch(i)
        {
        case 1: channel = PSChannel::A; break;
        case 2: channel = PSChannel::B; break;
        case 3: channel = PSChannel::C; break;
        case 4: channel = PSChannel::D; break;
        }
        if(picoscopeSettings->channel(channel))
            if(picoscopeSettings->range(channel) > range)
                range = picoscopeSettings->range(channel);
    }
    run->setPSRange(range);
    run->setPSCoupling(picoscopeSettings->coupling());
    run->setPSOffset(picoscopeSettings->offset(PSChannel::A)); //TODO
    run->setPSCollectionTime(picoscopeSettings->collectionTime());
    run->setPSSampleInterval(picoscopeSettings->sampleInterval());
    run->setPSPresample(picoscopeSettings->presamplePercentage());

    picoscope->getRun(run, psSettingsWidget->averageCaptures(), exportSettingsWidget->saveStdev());

    /*float gainA = psSettingsWidget->gainA();
    float gainB = psSettingsWidget->gainB();
    float gainC = psSettingsWidget->gainC();
    float gainD = psSettingsWidget->gainD();

    try
    {
        int addCounter = 1;

        if(psSettings->channel(PSChannel::A))
        {
            run->setPmtGainVoltage(addCounter, gainA);
            addCounter++;
        }
        if(psSettings->channel(PSChannel::B))
        {
            run->setPmtGainVoltage(addCounter, gainB);
            addCounter++;
        }
        if(psSettings->channel(PSChannel::C))
        {
            run->setPmtGainVoltage(addCounter, gainC);
            addCounter++;
        }
        if(psSettings->channel(PSChannel::D))
        {
            run->setPmtGainVoltage(addCounter, gainD);
        }
    }
    catch(LIISimException e)
    {
        MESSAGE(e.what(),e.type());
    }

    dataModel->registerMRun(run);

    // check if autosaving data is enabled
    if(exportSettingsWidget->autoSave())
    {
        // generate export information
        SignalIORequest rq = exportSettingsWidget->generateExportRequest(run);

        // save signal data
        Core::instance()->getSignalManager()->exportSignalsManager(rq);
    }*/
}


void DataAcquisitionWindow::processBlockProcessingFinished(MRun *run)
{
    /*float gainA = psSettingsWidget->gainA();
    float gainB = psSettingsWidget->gainB();
    float gainC = psSettingsWidget->gainC();
    float gainD = psSettingsWidget->gainD();

    try
    {
        int addCounter = 1;

        if(picoscopeSettings->channel(PSChannel::A))
        {
            run->setPmtGainVoltage(addCounter, gainA);
            addCounter++;
        }
        if(picoscopeSettings->channel(PSChannel::B))
        {
            run->setPmtGainVoltage(addCounter, gainB);
            addCounter++;
        }
        if(picoscopeSettings->channel(PSChannel::C))
        {
            run->setPmtGainVoltage(addCounter, gainC);
            addCounter++;
        }
        if(picoscopeSettings->channel(PSChannel::D))
        {
            run->setPmtGainVoltage(addCounter, gainD);
        }
    }
    catch(LIISimException e)
    {
        MESSAGE(e.what(),e.type());
    }*/

    //add the run to the (if selected) group
    //if(run->parentItem())
    //    run->group()->insertChild(run);
    //dataModel->registerMRun(run);

    //update the gui to the new run
    selectedRun = run;
    updateToSelectedRun();

    // process single signal
    SignalManager * sigManager = Core::instance()->getSignalManager();
    QList<MRun*> runs;
    runs << run;
    sigManager->processRunList(runs,Signal::RAW);

    // wait until calculation is finished
    while(sigManager->isBusy())
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    // check if autosaving data is enabled
    if(exportSettingsWidget->autoSave())
    {

        // generate export information
        SignalIORequest rq = exportSettingsWidget->generateExportRequest(run, exportSettingsWidget->saveStdev());

        // save signal data
        Core::instance()->getSignalManager()->exportSignalsManager(rq);
    }

    // change busy state
    isBusy = false;

    // start streaming again after finishing block acquisition
    if(autoStartStreaming)
        picoscope->startStreaming();

    emit blockAcquisitionFinished();
}


/**
 * @brief DataAcquisitionWindow::onPsSettingsObjChanged Called when any settings have changed so the ui
 * shows these (-> loading settings)
 */
void DataAcquisitionWindow::onPsSettingsObjChanged()
{
    QString trigger("");
    QString triggerTooltip("Active Triggers:");
    if(picoscopeSettings->getTriggerActiv(PSChannel::A))
    {
        trigger.append("A");
        triggerTooltip.append("\nA - ");
        triggerTooltip.append(picoscopeSettings->getTriggerDescription(PSChannel::A));
    }
    if(picoscopeSettings->getTriggerActiv(PSChannel::B))
    {
        if(!trigger.isEmpty())
            trigger.append(" ");
        trigger.append("B");
        triggerTooltip.append("\nB - ");
        triggerTooltip.append(picoscopeSettings->getTriggerDescription(PSChannel::B));
    }
    if(picoscopeSettings->getTriggerActiv(PSChannel::C))
    {
        if(!trigger.isEmpty())
            trigger.append(" ");
        trigger.append("C");
        triggerTooltip.append("\nC - ");
        triggerTooltip.append(picoscopeSettings->getTriggerDescription(PSChannel::C));
    }
    if(picoscopeSettings->getTriggerActiv(PSChannel::D))
    {
        if(!trigger.isEmpty())
            trigger.append(" ");
        trigger.append("D");
        triggerTooltip.append("\nD - ");
        triggerTooltip.append(picoscopeSettings->getTriggerDescription(PSChannel::D));
    }
    if(picoscopeSettings->getTriggerActiv(PSChannel::AUX))
    {
        if(!trigger.isEmpty())
            trigger.append(" ");
        trigger.append("AUX");
        triggerTooltip.append("\nAUX - ");
        triggerTooltip.append(picoscopeSettings->getTriggerDescription(PSChannel::AUX));
    }
    if(trigger.isEmpty())
    {
        labelActiveTriggers->setText("No Triggers");
        labelActiveTriggers->setToolTip("No trigger selected");
    }
    else
    {
        labelActiveTriggers->setText(trigger);
        labelActiveTriggers->setToolTip(triggerTooltip);
    }

    spinboxAveragingBufferSize->blockSignals(true);
    spinboxAveragingBufferSize->setValue(picoscopeSettings->getAveragingBufferSize());
    spinboxAveragingBufferSize->blockSignals(false);
}


void DataAcquisitionWindow::handleProcessingProgress(float percent)
{
    labelProcessing->setText(QString("Processing (").append(QString::number((double)percent)).append("%)"));
}

void DataAcquisitionWindow::handlePicoScopeErrorMsg(QString message)
{
    MSG_ERR(QString("[PicoScope Error] ").append(message));
}


void DataAcquisitionWindow::handlePicoScopeInfoMsg(QString message)
{
    MSG_STATUS(QString("[PicoScope] ").append(message));
}


void DataAcquisitionWindow::handlePicoScopeOpenStatusChanged(bool open)
{
    if(open)
    {
        labelConnection->setText("Open   ");
        labelConnection->setStyleSheet("QLabel { color : green; }");
    }
    else
    {
        labelConnection->setText("Closed ");
        labelConnection->setStyleSheet("QLabel { color : red; }");
    }
}


void DataAcquisitionWindow::handlePicoScopeStreamingStatusChanged(bool streaming)
{
    if(streaming)
    {
        labelStreaming->setStyleSheet("QLabel { color : red }");
    }
    else
    {
        labelStreaming->setStyleSheet("QLabel { color : grey }");
    }
}


void DataAcquisitionWindow::handlePicoScopeTriggeredStatusChanged(bool triggered)
{
    if(triggered)
    {
        labelTriggered->setStyleSheet("QLabel { color : blue }");
    }
    else
    {
        labelTriggered->setStyleSheet("QLabel { color : grey }");
    }
}


void DataAcquisitionWindow::handlePicoScopeBlockrunStatusChanged(bool blockrun)
{
    if(blockrun)
    {
        labelRunningBlock->setStyleSheet("QLabel { color : red }");
        runSettingsWidget->setEnabled(false);
        exportSettingsWidget->setEnabled(false);
        psSettingsWidget->setEnabled(false);
        pbRunBlock->setEnabled(false);
        cbliisettings->setEnabled(false);
    }
    else
    {
        labelRunningBlock->setStyleSheet("QLabel { color : grey }");
        runSettingsWidget->setEnabled(true);
        exportSettingsWidget->setEnabled(true);
        psSettingsWidget->setEnabled(true);
        pbRunBlock->setEnabled(true);
        cbliisettings->setEnabled(true);
    }
}


void DataAcquisitionWindow::handlePicoScopeProcessingStatusChanged(bool processing)
{
    if(processing)
    {
        labelProcessing->setStyleSheet("QLabel { color : red }");
    }
    else
    {
        labelProcessing->setStyleSheet("QLabel { color : grey }");
        labelProcessing->setText("Processing");
    }
}


void DataAcquisitionWindow::handlePicoScopeErrorStatusChanged(bool error)
{
    if(error)
    {
        buttonError->setEnabled(true);
        buttonError->setStyleSheet("QPushButton { color : red; background : transparent; }");
        buttonError->setToolTip(picoscope->getLastError());
    }
    else
    {
        buttonError->setEnabled(false);
        buttonError->setStyleSheet("QPushButton { color : grey; background : transparent; }");
        buttonError->setToolTip("");
    }
}


void DataAcquisitionWindow::errorClicked(bool clicked)
{
    QMessageBox msgbox;
    msgbox.setText("Error:");
    msgbox.setInformativeText(picoscope->getLastError());
    msgbox.exec();
}


void DataAcquisitionWindow::signalSpinnerChanged(int value)
{
    if(selectedRun)
    {
        signalPlot->detachAllCurves();

        MPoint *point = selectedRun->getPre(value - 1);

        for(int i = 1; i <= point->channelCount(Signal::RAW); i++)
        {
            switch(i)
            {
            case 1:
                signalPlot->setCurrentColor(QColor("red").light());
                signalPlot->addSignal(point->getSignal(1, Signal::RAW), "Channel 1", false);
                break;
            case 2:
                signalPlot->setCurrentColor(QColor("green").light());
                signalPlot->addSignal(point->getSignal(2, Signal::RAW), "Channel 2", false);
                break;
            case 3:
                signalPlot->setCurrentColor(QColor("blue").light());
                signalPlot->addSignal(point->getSignal(3, Signal::RAW), "Channel 3", false);
                break;
            case 4:
                signalPlot->setCurrentColor(QColor("yellow"));
                signalPlot->addSignal(point->getSignal(4, Signal::RAW), "Channel 4", false);
                break;
            }
        }

        if(point->getTriggerTime() > 0)
        {
            if((fabs(point->getMinSignalTime(Signal::RAW)) - fabs(point->getTriggerTime())) == 0)
            {
                signalPlot->setTriggerMarker(0, 0);
            }
            else if((fabs(point->getMinSignalTime(Signal::RAW) - fabs(point->getTriggerTime()))) > 0)
            {
                signalPlot->setTriggerMarker(-fabs(point->getTriggerTime()) * pow(10, 9), 0);
            }
        }

        if(selectedRun->psRange() != PSRange::NONE)
        {
            switch(selectedRun->psRange())
            {
            case PSRange::R50mV:    signalPlot->setYView(-0.055,  0.055);   break;
            case PSRange::R100mV:   signalPlot->setYView(-0.11,   0.11);    break;
            case PSRange::R200mV:   signalPlot->setYView(-0.22,   0.22);    break;
            case PSRange::R500mV:   signalPlot->setYView(-0.55,   0.55);    break;
            case PSRange::R1V:      signalPlot->setYView(-1.1,    1.1);     break;
            case PSRange::R2V:      signalPlot->setYView(-2.1,    2.1);     break;
            case PSRange::R5V:      signalPlot->setYView(-5.1,    5.1);     break;
            case PSRange::R10V:     signalPlot->setYView(-10.1,   10.1);    break;
            case PSRange::R20V:     signalPlot->setYView(-20.1,   20.1);    break;
            }
        }
        else
        {
            signalPlot->setZoomMode(DataAcquisitionPlotWidget::ZOOM_RESET);
        }
    }
}


void DataAcquisitionWindow::onChangeTriggerClicked(bool clicked)
{
    DA_TriggerDialog triggerDialog(picoscopeSettings, this);
    triggerDialog.exec();
}


void DataAcquisitionWindow::onStreamingModeToggled(QAbstractButton *button, bool state)
{
    if(!state)
        return;
    if(button == rbStreamingSingleMeasurement)
        picoscopeSettings->streaming_mode = PSStreamingMode::SingleMeasurement;
    if(button == rbStreamingAverage)
        picoscopeSettings->streaming_mode = PSStreamingMode::AverageMeasurement;
    if(button == rbStreamingBoth)
        picoscopeSettings->streaming_mode = PSStreamingMode::Both;
}


void DataAcquisitionWindow::onStreamingData(StreamPoint point)
{
    if(runIdsToPlotWhileStreamingChanged)
    {
        //signalPlot->detachAllCurves();

        signalPlot->detachStaticCurves();

        for(int i = 0; i < runIdsToPlotWhileStreaming.size(); i++)
        {
            try
            {
                MRun *run = dataModel->mrun(runIdsToPlotWhileStreaming[i]);

                for(int j = 0; j < run->getNoChannels(Signal::RAW); j++)
                {
                    signalPlot->addStaticSignal(run->getPre(0)->getSignal(j+1, Signal::RAW), run->getName().append(" Channel %0").arg(j));
                }
            }
            catch(LIISimException e)
            {
                MSG_ERR(e.what());
            }
        }

        runIdsToPlotWhileStreamingChanged = false;
    }

    //referenceWidget->draw(signalPlot);
    //point.draw(signalPlot);
    //point.draw(signalPlot, psSettingsWidget->plotDrawOrder);
    signalPlot->updateStreamPoint(&point);

    avgCounterLabel->setText(QString("Cache %0 / ").arg(point.averageBufferFilling));

    QString ofText("Overflow:");
    bool ofEnable = false;
    if(point.overflowA)
    {
        ofText.append(" A");
        ofEnable = true;
    }
    if(point.overflowB)
    {
        ofText.append(" B");
        ofEnable = true;
    }
    if(point.overflowC)
    {
        ofText.append(" C");
        ofEnable = true;
    }
    if(point.overflowD)
    {
        ofText.append(" D");
        ofEnable = true;
    }

    buttonOverflow->setText(ofText);
    buttonOverflow->setVisible(ofEnable);
}


void DataAcquisitionWindow::onReferenceRequest()
{
    referenceWidget->addReference(picoscope->getLastAverage());
}


void DataAcquisitionWindow::onBlockSequenceWorkerStopped()
{
    pbRunBlockSequence->setText("Run Block Sequence");
    pbRunBlockSequence->setStyleSheet("QPushButton { color: black }");

    runSettingsWidget->setEnabled(true);
    exportSettingsWidget->setEnabled(true);
    psSettingsWidget->setEnabled(true);
    pbRunBlock->setEnabled(true);
    cbliisettings->setEnabled(true);
    rbtStreamingMode->setEnabled(true);
}


void DataAcquisitionWindow::onBlockSequenceWorkerStatusChanged(QString status)
{
    status.append(" (Press to Stop Sequence)");
    pbRunBlockSequence->setText(status);
}


void DataAcquisitionWindow::onPlotTypeChanged(DataAcquisitionPlotWidget::PlotType type)
{
    switch(type)
    {
    case BasePlotWidgetQwt::LINE_CROSSES: Core::instance()->guiSettings->setValue("dataacquisition", "plottype", 0); break;
    case BasePlotWidgetQwt::LINE:         Core::instance()->guiSettings->setValue("dataacquisition", "plottype", 1); break;
    case BasePlotWidgetQwt::DOTS_SMALL:   Core::instance()->guiSettings->setValue("dataacquisition", "plottype", 2); break;
    case BasePlotWidgetQwt::DOTS_MEDIUM:  Core::instance()->guiSettings->setValue("dataacquisition", "plottype", 3); break;
    case BasePlotWidgetQwt::DOTS_LARGE:   Core::instance()->guiSettings->setValue("dataacquisition", "plottype", 4); break;
    }
}


#ifdef LIISIM_NIDAQMX
void DataAcquisitionWindow::onShowDeviceManager()
{
    DeviceManagerDialog deviceManagerDialog(this);
    deviceManagerDialog.exec();
}


void DataAcquisitionWindow::onAnalogInputStateChanged(bool enabled)
{
    if(enabled)
        labelAnalogInputState->setStyleSheet("QLabel { color : green; }");
    else
        labelAnalogInputState->setStyleSheet("QLabel { color : firebrick; }");
}


void DataAcquisitionWindow::onAnalogOutputStateChanged(bool enabled)
{
    if(enabled)
        labelAnalogOutputState->setStyleSheet("QLabel { color : green; }");
    else
        labelAnalogOutputState->setStyleSheet("QLabel { color : firebrick; }");
}


void DataAcquisitionWindow::onDigitalOutputStateChanged(bool enabled)
{
    if(enabled)
        labelDigitalOutputState->setStyleSheet("QLabel { color : green; }");
    else
        labelDigitalOutputState->setStyleSheet("QLabel { color : firebrick; }");
}


void DataAcquisitionWindow::onDigitalOutTriggered()
{
    if(QObject::sender() == toolbuttonDigitalOut1)
    {
        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(1))
            Core::instance()->devManager->setDigitalOutputEnabled(1, false);
        else
            Core::instance()->devManager->setDigitalOutputEnabled(1, true);

        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(1))
            toolbuttonDigitalOut1->setStyleSheet("QToolButton { color : forestgreen; }");
        else
            toolbuttonDigitalOut1->setStyleSheet("QToolButton { color : firebrick; }");
    }
    if(QObject::sender() == toolbuttonDigitalOut2)
    {
        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(2))
            Core::instance()->devManager->setDigitalOutputEnabled(2, false);
        else
            Core::instance()->devManager->setDigitalOutputEnabled(2, true);

        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(2))
            toolbuttonDigitalOut2->setStyleSheet("QToolButton { color : forestgreen; }");
        else
            toolbuttonDigitalOut2->setStyleSheet("QToolButton { color : firebrick; }");
    }
    if(QObject::sender() == toolbuttonDigitalOut3)
    {
        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(3))
            Core::instance()->devManager->setDigitalOutputEnabled(3, false);
        else
            Core::instance()->devManager->setDigitalOutputEnabled(3, true);

        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(3))
            toolbuttonDigitalOut3->setStyleSheet("QToolButton { color : forestgreen; }");
        else
            toolbuttonDigitalOut3->setStyleSheet("QToolButton { color : firebrick; }");
    }
    if(QObject::sender() == toolbuttonDigitalOut4)
    {
        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(4))
            Core::instance()->devManager->setDigitalOutputEnabled(4, false);
        else
            Core::instance()->devManager->setDigitalOutputEnabled(4, true);

        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(4))
            toolbuttonDigitalOut4->setStyleSheet("QToolButton { color : forestgreen; }");
        else
            toolbuttonDigitalOut4->setStyleSheet("QToolButton { color : firebrick; }");
    }
    if(QObject::sender() == toolbuttonDigitalOut5)
    {
        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(5))
            Core::instance()->devManager->setDigitalOutputEnabled(5, false);
        else
            Core::instance()->devManager->setDigitalOutputEnabled(5, true);

        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(5))
            toolbuttonDigitalOut5->setStyleSheet("QToolButton { color : forestgreen; }");
        else
            toolbuttonDigitalOut5->setStyleSheet("QToolButton { color : firebrick; }");
    }
    if(QObject::sender() == toolbuttonDigitalOut6)
    {
        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(6))
            Core::instance()->devManager->setDigitalOutputEnabled(6, false);
        else
            Core::instance()->devManager->setDigitalOutputEnabled(6, true);

        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(6))
            toolbuttonDigitalOut6->setStyleSheet("QToolButton { color : forestgreen; }");
        else
            toolbuttonDigitalOut6->setStyleSheet("QToolButton { color : firebrick; }");
    }
    if(QObject::sender() == toolbuttonDigitalOut7)
    {
        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(7))
            Core::instance()->devManager->setDigitalOutputEnabled(7, false);
        else
            Core::instance()->devManager->setDigitalOutputEnabled(7, true);

        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(7))
            toolbuttonDigitalOut7->setStyleSheet("QToolButton { color : forestgreen; }");
        else
            toolbuttonDigitalOut7->setStyleSheet("QToolButton { color : firebrick; }");
    }
    if(QObject::sender() == toolbuttonDigitalOut8)
    {
        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(8))
            Core::instance()->devManager->setDigitalOutputEnabled(8, false);
        else
            Core::instance()->devManager->setDigitalOutputEnabled(8, true);

        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(8))
            toolbuttonDigitalOut8->setStyleSheet("QToolButton { color : forestgreen; }");
        else
            toolbuttonDigitalOut8->setStyleSheet("QToolButton { color : firebrick; }");
    }
    if(QObject::sender() == toolbuttonDigitalOut9)
    {
        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(9))
            Core::instance()->devManager->setDigitalOutputEnabled(9, false);
        else
            Core::instance()->devManager->setDigitalOutputEnabled(9, true);

        if(Core::instance()->devManager->getDigitalOutputChannelEnabled(9))
            toolbuttonDigitalOut9->setStyleSheet("QToolButton { color : forestgreen; }");
        else
            toolbuttonDigitalOut9->setStyleSheet("QToolButton { color : firebrick; }");
    }
}


void DataAcquisitionWindow::onDigitalOutputChanged()
{
    if(Core::instance()->devManager->getDigitalOutputChannelEnabled(1))
        toolbuttonDigitalOut1->setStyleSheet("QToolButton { color : forestgreen; }");
    else
        toolbuttonDigitalOut1->setStyleSheet("QToolButton { color : firebrick; }");

    if(Core::instance()->devManager->getDigitalOutputChannelEnabled(2))
        toolbuttonDigitalOut2->setStyleSheet("QToolButton { color : forestgreen; }");
    else
        toolbuttonDigitalOut2->setStyleSheet("QToolButton { color : firebrick; }");

    if(Core::instance()->devManager->getDigitalOutputChannelEnabled(3))
        toolbuttonDigitalOut3->setStyleSheet("QToolButton { color : forestgreen; }");
    else
        toolbuttonDigitalOut3->setStyleSheet("QToolButton { color : firebrick; }");

    if(Core::instance()->devManager->getDigitalOutputChannelEnabled(4))
        toolbuttonDigitalOut4->setStyleSheet("QToolButton { color : forestgreen; }");
    else
        toolbuttonDigitalOut4->setStyleSheet("QToolButton { color : firebrick; }");

    if(Core::instance()->devManager->getDigitalOutputChannelEnabled(5))
        toolbuttonDigitalOut5->setStyleSheet("QToolButton { color : forestgreen; }");
    else
        toolbuttonDigitalOut5->setStyleSheet("QToolButton { color : firebrick; }");

    if(Core::instance()->devManager->getDigitalOutputChannelEnabled(6))
        toolbuttonDigitalOut6->setStyleSheet("QToolButton { color : forestgreen; }");
    else
        toolbuttonDigitalOut6->setStyleSheet("QToolButton { color : firebrick; }");

    if(Core::instance()->devManager->getDigitalOutputChannelEnabled(7))
        toolbuttonDigitalOut7->setStyleSheet("QToolButton { color : forestgreen; }");
    else
        toolbuttonDigitalOut7->setStyleSheet("QToolButton { color : firebrick; }");

    if(Core::instance()->devManager->getDigitalOutputChannelEnabled(8))
        toolbuttonDigitalOut8->setStyleSheet("QToolButton { color : forestgreen; }");
    else
        toolbuttonDigitalOut8->setStyleSheet("QToolButton { color : firebrick; }");

    if(Core::instance()->devManager->getDigitalOutputChannelEnabled(9))
        toolbuttonDigitalOut9->setStyleSheet("QToolButton { color : forestgreen; }");
    else
        toolbuttonDigitalOut9->setStyleSheet("QToolButton { color : firebrick; }");
}

#endif
