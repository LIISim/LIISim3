#include "signalprocessingeditor.h"

#include <QPalette>
#include <QToolButton>
#include <QButtonGroup>
#include "../../core.h"

#include "../dataItemViews/treeview/dataitemtreeview.h"
#include "../../signal/processing/processingchain.h"
#include "../../signal/processing/temperatureprocessingchain.h"
#include "../../signal/processing/plugins/temperaturecalculator.h"
#include "../../signal/processing/plugins/multisignalaverage.h"
#include "../../signal/mrun.h"
#include "../../signal/signalmanager.h"
#include "../../general/LIISimException.h"
#include "../utils/ribbontoolbox.h"
#include "memusagewidget.h"
#include "../utils/mrundetailswidget.h"

#include "../utils/calculationtoolbox.h"
#include "../utils/notificationmanager.h"

const QString SignalProcessingEditor::identifier_mainSplitter = "signal_processing_main";
const QString SignalProcessingEditor::identifier_plotSplitter = "signal_processing_plot";
const QString SignalProcessingEditor::identifier_plugSplitter = "signal_processing_plug";
const QString SignalProcessingEditor::identifier_treeDetailsSplitter = "signal_processing_tree_details";

// -----------------------
// TODO: DOCUMENTATION !!!
// -----------------------

SignalProcessingEditor::SignalProcessingEditor(QWidget *parent) : QWidget(parent)
{
    MSG_DETAIL_1("init SignalProcessingEditor");
    setWindowTitle(tr("Signal Processing Editor"));
   // resize(1000,700);

    setObjectName("SP");

    m_dataModel = Core::instance()->dataModel();
    m_sigManager = Core::instance()->getSignalManager();
    m_mrun = 0;
    m_curMPoint = 1;
    m_mutePPTWIchanges = true;
    m_showUnprocessedSignalsRaw = true;
    m_showUnprocessedSignalsAbs = true;

    // processing plugin colors
    pcolor_green     = QColor(81,221,61);
    pcolor_blue      = QColor(61,166,231);
    pcolor_red       = QColor(158,1,66);
    //pcolor_orange  -  this is not used here, please see PPTreeWidgetItem

    mainHsplit = new GridableSplitter(Qt::Horizontal);

    layVmain = new QVBoxLayout;
    layVmain->setMargin(0);
    layVmain->addWidget(mainHsplit);
    setLayout(layVmain);

    layVbox = new QVBoxLayout;
    layVbox->setMargin(0);
    QWidget* cw = new QWidget;
    cw->setLayout(layVbox);    
    mainHsplit->addWidget(cw);

    groupTree = new DataItemTreeView(DataItemTreeView::SE_GROUPS,Core::instance()->dataModel()->rootItem());
    groupTree->setObjectName("TUTORIAL_SE_RUN_TREE");
    groupTree->setToolTip("List of all groups and loaded measurement runs");
    connect(groupTree,SIGNAL(keyPressed(QKeyEvent*)),
            SLOT(keyPressEvent(QKeyEvent*)));

    connect(groupTree,SIGNAL(keyReleased(QKeyEvent*)),
            SLOT(keyReleaseEvent(QKeyEvent*)));

    leftVSplit = new QSplitter(Qt::Vertical);
    layVbox->addWidget(leftVSplit);
    leftVSplit->addWidget(groupTree);

    // TABLE VIEW FOR MRUN DETAILS
    mrunDetailsView = new MRunDetailsWidget;
    leftVSplit->addWidget(mrunDetailsView);

    mrunDetailsView->setObjectName("TUTORIAL_SE_DETAILS");

    layVbox->setSizeConstraint(QLayout::SetMinAndMaxSize);

    // SIGNAL PLOTS
    plotVsplit = new GridableSplitter(Qt::Vertical);

    rawPlot = new SignalPlotWidgetQwt;
    absPlot = new SignalPlotWidgetQwt;
    tempPlot = new SignalPlotWidgetQwt;
    m_curPlot = 0;

    rawPlot->setSignalType(Signal::RAW);
    absPlot->setSignalType(Signal::ABS);
    tempPlot->setSignalType(Signal::TEMPERATURE);

    //rawPlot->toolActionPlotMarker()->triggered(true);

    rawPlot->setObjectName("SPE_RAW_PLOT");
    absPlot->setObjectName("SPE_ABS_PLOT");
    tempPlot->setObjectName("SPE_TEMP_PLOT");
    plotVsplit->setObjectName("SPE_ALL_PLOTS");

    plotVsplit->addWidget(rawPlot);
    plotVsplit->addWidget(absPlot);
    plotVsplit->addWidget(tempPlot);    

    connect(rawPlot, SIGNAL(plotTypeChanged(BasePlotWidgetQwt::PlotType)), SLOT(onPlotTypeChanged(BasePlotWidgetQwt::PlotType)));
    connect(absPlot, SIGNAL(plotTypeChanged(BasePlotWidgetQwt::PlotType)), SLOT(onPlotTypeChanged(BasePlotWidgetQwt::PlotType)));
    connect(tempPlot, SIGNAL(plotTypeChanged(BasePlotWidgetQwt::PlotType)), SLOT(onPlotTypeChanged(BasePlotWidgetQwt::PlotType)));

    // listen to plot tool changes in raw plot,
    // used to change current tool in absolute/temperature plots
    connect(rawPlot,SIGNAL(currentToolChanged(BasePlotWidgetQwt::PlotZoomMode)),
            this,SLOT(onCurrentPlotToolChanged(BasePlotWidgetQwt::PlotZoomMode)));

    actionPlotLink = new QAction("plot link",this);
    actionPlotLink->setToolTip("link scaling of time- and y- axes of all mruns");
    actionPlotLink->setCheckable(true);
    actionPlotLink->setChecked(false);

    connect(actionPlotLink,SIGNAL(toggled(bool)),SLOT(onPlotLinkToggled(bool)));
    connect(actionPlotLink,SIGNAL(toggled(bool)),rawPlot,SLOT(setEnabledXViewLink(bool)));
    connect(actionPlotLink,SIGNAL(toggled(bool)),absPlot,SLOT(setEnabledXViewLink(bool)));
    connect(actionPlotLink,SIGNAL(toggled(bool)),tempPlot,SLOT(setEnabledXViewLink(bool)));

    rawPlot->setEnabledXViewLink(actionPlotLink->isChecked());
    absPlot->setEnabledXViewLink(actionPlotLink->isChecked());
    tempPlot->setEnabledXViewLink(actionPlotLink->isChecked());

    connect(rawPlot,SIGNAL(xViewChanged(double,double)),SLOT(onXViewChanged(double,double)));
    connect(absPlot,SIGNAL(xViewChanged(double,double)),SLOT(onXViewChanged(double,double)));
    connect(tempPlot,SIGNAL(xViewChanged(double,double)),SLOT(onXViewChanged(double,double)));

    // PROCESSING PLUGIN GUI
    plugVsplit = new GridableSplitter(Qt::Vertical);
    plugVsplit->setObjectName("SPE_PCHAINS");

    plugVsplit->copyMovesFromAndTo(plotVsplit);
    plotVsplit->copyMovesFromAndTo(plugVsplit);

    mainHsplit->addWidget(plugVsplit);
    mainHsplit->addWidget(plotVsplit);

    rawTree = new DataItemTreeView(DataItemTreeView::SE_PSTEPS);
    absTree = new DataItemTreeView(DataItemTreeView::SE_PSTEPS);
    tempTree = new DataItemTreeView(DataItemTreeView::SE_PSTEPS);

    rawTree->setObjectName("SPE_PCHAIN_RAW");
    absTree->setObjectName("SPE_PCHAIN_ABS");
    tempTree->setObjectName("SPE_PCHAIN_TEMP");

    rawTree->setColumnCount(4);
    absTree->setColumnCount(4);
    tempTree->setColumnCount(4);

    rawTree->setToolTip("List of processing steps for raw\nsignal data of measurement run\n(Right click to add/remove steps)");
    absTree->setToolTip("List of processing steps for absolute\nsignal data of measurement run\n(Right click to add/remove steps)");
    tempTree->setToolTip("List of processing steps for temperature\ntrace data of measurement run\n(Right click to add/remove steps)");

    connect(rawTree,SIGNAL(keyPressed(QKeyEvent*)), SLOT(keyPressEvent(QKeyEvent*)));
    connect(absTree,SIGNAL(keyPressed(QKeyEvent*)), SLOT(keyPressEvent(QKeyEvent*)));
    connect(tempTree,SIGNAL(keyPressed(QKeyEvent*)), SLOT(keyPressEvent(QKeyEvent*)));

    connect(rawTree, SIGNAL(treeItemModified(QTreeWidgetItem*,int)),
            SLOT(handlePPTreeItemModified(QTreeWidgetItem*,int)));
    connect(absTree, SIGNAL(treeItemModified(QTreeWidgetItem*,int)),
            SLOT(handlePPTreeItemModified(QTreeWidgetItem*,int)));
    connect(tempTree, SIGNAL(treeItemModified(QTreeWidgetItem*,int)),
            SLOT(handlePPTreeItemModified(QTreeWidgetItem*,int)));

    QStringList headerLabels;
    headerLabels << "1)" << "Raw Processing Steps" << " "  << " " ;
    rawTree->setHeaderLabels(headerLabels);
    headerLabels.clear();
    headerLabels << "2)" << "Absolute Processing Steps"<< " " << " " ;
    absTree->setHeaderLabels(headerLabels);
    headerLabels.clear();
    headerLabels << "3)" << "Temperature Trace Calculation"<< " " << " " ;
    tempTree->setHeaderLabels(headerLabels);

    // create a dummy widget containing the raw pstepstree
    //and the apply to all checkbox

    QVBoxLayout* layRawPstepBox = new QVBoxLayout;
    layRawPstepBox->setSizeConstraint(QLayout::SetMaximumSize);
    layRawPstepBox->setMargin(0);
    QWidget* dummyWidget = new QWidget;
    dummyWidget->setLayout(layRawPstepBox);

    layRawPstepBox->addWidget(rawTree);

    plugVsplit->addWidget(dummyWidget);
    plugVsplit->addWidget(absTree);
    plugVsplit->addWidget(tempTree);

    connect(m_dataModel,SIGNAL(mrunAdded(MRun*)),
            SLOT(onMRunAdded(MRun*)));

    connect(groupTree,SIGNAL(selectionChanged(QList<QTreeWidgetItem*>)),
            this, SLOT(onMRunSelectionChanged(QList<QTreeWidgetItem*>)));

    connect(m_sigManager,SIGNAL(importStateChanged(bool)),
            SLOT(handleImportStateChanged(bool)));
    connect(m_sigManager,SIGNAL(processingStateChanged(bool)),
            SLOT(handleProcessingStateChanged(bool)));

    initToolBars();

    int w1 = 250;
    int w2 = 230;
    int w3 = width()-w1-w2;

    groupTree->setMinimumWidth(255);
    groupTree->setMaximumWidth(300);
    rawTree->setMaximumWidth(400);
    QList<int> wlist;
    wlist << w1 << w2 << w3;
    mainHsplit->setSizes(wlist);

    m_mutePPTWIchanges = false;

    // init gui
    groupTree->expandAll();

    // subscribe to global gui settings signal
    connect(Core::instance()->guiSettings,
            SIGNAL(settingsChanged()),
            SLOT(onGuiSettingsChanged()));

    connect(m_sigManager, SIGNAL(importFinished()), SLOT(onSignalManagerImportFinished()));

    connect(mainHsplit, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    connect(plotVsplit, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    connect(plugVsplit, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    connect(leftVSplit, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
}


// -----------------------
// MORE GUI INITIALIZATION
// -----------------------


void SignalProcessingEditor::initToolBars()
{
    m_ribbonToolbar = new QToolBar;

    //#ifdef LIISIM_FULL
    // -----------------------
    // 'SIGNAL DATA' Toolbox
    // -----------------------
    RibbonToolBox* rtbFiles = new RibbonToolBox("SIGNAL DATA");
    rtbFiles->setObjectName("SPE_FILES_BOX");

    rtbFiles->addAction(Core::instance()->getSignalManager()->actionDataImport(),0,0);
    rtbFiles->addAction(Core::instance()->getSignalManager()->actionDataExport(),1,0);
    m_ribbonToolbar->addWidget(rtbFiles);

    m_ribbonToolbar->addSeparator();

    // -----------------------
    // 'CALCULATION' Toolbox
    // -----------------------

    CalculationToolbox *calcToolbox = new CalculationToolbox(this);
    calcToolbox->setObjectName("SPE_CALC_BOX");
    m_ribbonToolbar->addWidget(calcToolbox);
    m_ribbonToolbar->addSeparator();
    connect(calcToolbox, SIGNAL(recalc(QList<Signal::SType>)), SLOT(onCalcToolboxRecalc(QList<Signal::SType>)));

    // -----------------------
    // 'CURRENT SELECTION' Toolbox
    // -----------------------
    RibbonToolBox* rtbSelection = new RibbonToolBox("CURRENT SELECTION");
    rtbSelection->setObjectName("SPE_SELECTION_BOX");

    QLabel* descr1 = new QLabel("  Run: ");
    descr1->setToolTip("Currently selected Measurement Run");

    runNameLabel = new QLabel("no signal data");
    QFont f;
    f.setBold(true);
    runNameLabel->setFont(f);
    runNameLabel->setToolTip("Currently selected Measurement Run");
   // runNameLabel->setMinimumWidth(170);

    int toolBarHeight = 20;
    QLabel* sigLb = new QLabel("  Signal: ");

    sigSpin = new QSpinBox;
    sigSpin->setToolTip("Current signal index of Measurement Run");
    sigSpin->setMinimum(0);
    sigSpin->setMaximum(0);
    sigSpin->setMinimumHeight(toolBarHeight-1);
    sigSpin->setMinimumWidth(50);
    connect(sigSpin,SIGNAL(valueChanged(int)),SLOT(onSigIndexChanged(int)));

    sigCountLabel = new QLabel;
    sigCountLabel->setToolTip("number of signals of mrun\nand number of signals, which passed validation");

    //rtbSelection->addWidget(new QLabel(" "),0,0);
    rtbSelection->addWidget(descr1,0,0);
    rtbSelection->addWidget(runNameLabel,0,1, 1, 2); // colspan = 2
    rtbSelection->addWidget(sigLb,1,0);
    rtbSelection->addWidget(sigSpin,1,1);
    rtbSelection->addWidget(sigCountLabel,1,2);

    m_ribbonToolbar->addWidget(rtbSelection);
    m_ribbonToolbar->addSeparator();

    // -----------------------
    // 'PLOT OPTIONS' Toolbox
    // -----------------------

    RibbonToolBox* rtbPlot = new RibbonToolBox("PLOT OPTIONS");
    rtbPlot->setObjectName("SPE_PLOT_OPTIONS_BOX");

    // make these buttons look the same as in the default style
    rtbPlot->setStyleSheet("QToolButton { border: 0.5px solid rgba(0, 0, 0, 30) }");

    // add plot tool actions of raw plot to toolbar.
    // The raw plot's currentToolChanged() signal is then
    // used to copy the tool selection to the absolute and
    // temperature signal plots, see SignalProcessingEditor::onCurrentPlotChanged()
    rtbPlot->addAction(rawPlot->toolAction(SignalPlotWidgetQwt::PLOT_PAN),0,0);    
    rtbPlot->addAction(rawPlot->toolAction(SignalPlotWidgetQwt::DATA_CURSOR),0,1);
    rtbPlot->addAction(rawPlot->toolAction(SignalPlotWidgetQwt::AVG_RECT),0,2);
    rtbPlot->addAction(rawPlot->toolAction(SignalPlotWidgetQwt::FIT_RECT),0,3);
    rtbPlot->addAction(rawPlot->toolAction(SignalPlotWidgetQwt::ZOOM_RECT),1,0);
    rtbPlot->addAction(rawPlot->toolAction(SignalPlotWidgetQwt::ZOOM_RESET),1,1);
    rtbPlot->addAction(actionPlotLink,1,2);

    // activate plot pan tool as default
    rawPlot->toolAction(SignalPlotWidgetQwt::PLOT_PAN)->trigger();

    m_ribbonToolbar->addWidget(rtbPlot);

    // -----------------------
    // SPACER
    // -----------------------

    QWidget* spacer2 = new QWidget();
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_ribbonToolbar->addWidget(spacer2);

    m_ribbonToolbar->addSeparator();

    // -----------------------
    // 'NOTIFICATIONS' Toolbox
    // -----------------------

    QWidget *nc = Core::instance()->getNotificationManager()->getNotificationCenter(this);
    nc->setObjectName("SPE_NOTIFICATION_CENTER");
    m_ribbonToolbar->addWidget(nc);
    m_ribbonToolbar->addSeparator();

    // -----------------------
    // 'MEMORY USAGE' Toolbox
    // -----------------------

    MemUsageWidget* memw = new MemUsageWidget;
    memw->setObjectName("SPE_MEMORY_BOX");
    m_ribbonToolbar->addWidget(memw);    
}

// -----------------------
// HANDLE USER INTERACTION
// -----------------------

void SignalProcessingEditor::onMRunSelectionChanged(QList<QTreeWidgetItem*> selection, bool ignoreImport)
{
    if(m_sigManager->isImporting() && !ignoreImport)
    {
        mLastSelection = selection;
        return;
    }

    QRectF rv = rawPlot->getView();
    QRectF av = absPlot->getView();
    QRectF tv = tempPlot->getView();

    m_mutePPTWIchanges = true;

    m_lastRawExpandedStatus = rawTree->topLevelItemExpandedStates();
    m_lastAbsExpandedStatus = absTree->topLevelItemExpandedStates();
    m_lastTmpExpandedStatus = tempTree->topLevelItemExpandedStates();

    if(rawTree->topLevelItemCount() > 0)
        m_showUnprocessedSignalsRaw = rawTree->topLevelItem(0)->data(0,Qt::UserRole+5).toBool();

    if(absTree->topLevelItemCount() > 0)
        m_showUnprocessedSignalsAbs = absTree->topLevelItem(0)->data(0,Qt::UserRole+5).toBool();

    rawTree->clearAll();
    absTree->clearAll();
    tempTree->clearAll();
    rawTree->clearDummy();
    absTree->clearDummy();
    tempTree->clearDummy();

    rawPlot->detachAllCurves();
    absPlot->detachAllCurves();
    tempPlot->detachAllCurves();

    if(selection.isEmpty())
    {
        QString text("No run selected. Please select one from the treeview.");

        rawPlot->setPlotLabelText(text);
        absPlot->setPlotLabelText(text);
        tempPlot->setPlotLabelText(text);
        rawPlot->plotTextLabelVisible(true);
        absPlot->plotTextLabelVisible(true);
        tempPlot->plotTextLabelVisible(true);

        return;
    }

    QTreeWidgetItem* item = selection.last();
    int userData = item->data(0,Qt::UserRole).toInt();

    int data_id = item->data(0,Qt::UserRole+1).toInt();

    if(m_mrun)
    {
        m_mrun->disconnect(this);
    }

    if(userData == 2)
        m_mrun = m_dataModel->mrun(data_id);
    else
    {
        m_mrun = 0;
        if(userData==1)
        {
            runNameLabel->setText("no run selected");
        }
    }

    if(m_mrun)
    {
        connect(m_mrun,SIGNAL(dataChanged()),SLOT(onMRunDataChanged()));
        connect(m_mrun,SIGNAL(destroyed()),SLOT(onMRunDestroyed()));
        connect(m_mrun,SIGNAL(channelCountChanged(Signal::SType,int)),
                SLOT(onMRunChannelCountChanged(Signal::SType,int)));

        mrunDetailsView->setRun(m_mrun);
    }

    if(m_mrun)
    {
        runNameLabel->setText(m_mrun->getName());

        int nSigs = m_mrun->sizeAllMpoints();
        int nValid = m_mrun->sizeValidMpoints();
        sigCountLabel->setText( QString("    (%0 signals, valid: %1)")
                    .arg(nSigs)
                    .arg(nValid));

        sigSpin->setMaximum(nSigs);
        if(nSigs < 1)
            sigSpin->setMinimum(0);
        else
            sigSpin->setMinimum(1);


        VisToggableTWI* dummy1 = new VisToggableTWI(rawTree->treeWidget(),"Unprocessed");
        rawTree->setPermanentDummyItem(dummy1);
        dummy1->setVisState(m_showUnprocessedSignalsRaw);

        VisToggableTWI* dummy2 = new VisToggableTWI(absTree->treeWidget(),"Unprocessed");
        absTree->setPermanentDummyItem(dummy2);
        dummy2->setVisState(m_showUnprocessedSignalsAbs);

        VisToggableTWI* dummy3 = new VisToggableTWI(tempTree->treeWidget(), "Temperature Calculators:");
        tempTree->setPermanentDummyItem(dummy3);
        dummy3->setVisState(true);

        rawTree->setDataRoot(m_mrun->getProcessingChain(Signal::RAW),false);
        absTree->setDataRoot(m_mrun->getProcessingChain(Signal::ABS),false);
        tempTree->setDataRoot(m_mrun->getProcessingChain(Signal::TEMPERATURE),false);

        rawTree->setTopLevelItemExpandedStates(m_lastRawExpandedStatus);
        absTree->setTopLevelItemExpandedStates(m_lastAbsExpandedStatus);
        tempTree->setTopLevelItemExpandedStates(m_lastTmpExpandedStatus);

        rawPlot->setChannels(m_mrun->channelIDs(Signal::RAW));
        absPlot->setChannels(m_mrun->channelIDs(Signal::ABS));
        tempPlot->setChannels(m_mrun->channelIDs(Signal::TEMPERATURE));
    }

    replot();
    m_mutePPTWIchanges = false;
    if(actionPlotLink->isChecked())
    {
        rawPlot->setYView(rv.y(),rv.y()+rv.height());
        absPlot->setYView(av.y(),av.y()+av.height());
        tempPlot->setYView(tv.y(),tv.y()+tv.height());
    }

   updateItemColors();
}



void SignalProcessingEditor::handlePPTreeItemModified(QTreeWidgetItem *item, int col)
{
   if(m_mutePPTWIchanges)
       return;

   QObject* senderObj = QObject::sender();

   if(senderObj == rawTree)
   {
       m_curPlot = rawPlot;
       m_stype = Signal::RAW;

   }
   else if(senderObj == absTree)
   {
       m_curPlot = absPlot;
       m_stype = Signal::ABS;

   }
   else if(senderObj == tempTree)
   {
       m_curPlot = tempPlot;
       m_stype = Signal::TEMPERATURE;

   }
   plotCurrentPlugins();
}


void SignalProcessingEditor::onSigIndexChanged(int idx)
{
    if(idx < 1 )
        return;

    QRectF rv = rawPlot->getView();
    QRectF av = absPlot->getView();
    QRectF tv = tempPlot->getView();

    m_curMPoint = idx-1;
    replot();
    updateItemColors();

    if(actionPlotLink->isChecked())
    {
        rawPlot->setYView(rv.y(),rv.y()+rv.height());
        absPlot->setYView(av.y(),av.y()+av.height());
        tempPlot->setYView(tv.y(),tv.y()+tv.height());
    }
}


void SignalProcessingEditor::updateItemColors()
{
    if(!m_mrun)
        return;

    m_mutePPTWIchanges = true;
    DataItemTreeView* curTree;
    for(int p = 0; p < 3; p++)
    {
        Signal::SType stype;
        if(p == 0){
            stype = Signal::RAW;
            curTree = rawTree;
        }
        else if(p == 1){
            stype = Signal::ABS;
            curTree = absTree;
        }
        else{
            stype = Signal::TEMPERATURE;
            curTree = tempTree;
        }

        ProcessingChain* pchain = m_mrun->getProcessingChain(stype);
        if(!pchain)
            return;

        // int lastCalculatedPlugin = pchain->getIndexOfLastCalculatedPlugin(m_curMPoint);

        bool msa = false;

        for(int i = 0; i < pchain->noPlugs(); i++)
        {
            if(pchain->getPlug(i)->dirty())
                continue;
            if(!pchain->getPlug(i)->activated())
                continue;

            if(!msa)
                msa = (pchain->getPlug(i)->getName() == MultiSignalAverage::pluginName);

            int idx = i+1;

            QTreeWidgetItem* item = curTree->topLevelItem(idx);
            if(!item)
                return;


            bool valid = pchain->isValidAtStep(m_curMPoint,i);

            if(msa)
            {
                // blue
                item->setBackgroundColor(0, pcolor_blue);
                item->setBackgroundColor(1, pcolor_blue);
                item->setBackgroundColor(2, pcolor_blue);
                item->setBackgroundColor(3, pcolor_blue);
            }

            if(!msa && valid )
            {
                // green
                item->setBackgroundColor(0, pcolor_green);
                item->setBackgroundColor(1, pcolor_green);
                item->setBackgroundColor(2, pcolor_green);
                item->setBackgroundColor(3, pcolor_green);
            }

            if(!valid)
            {
                // TemperatureCalculator: mark only channels with error as RED
                if((pchain->getPlug(i)->getName() == TemperatureCalculator::pluginName)
                        && pchain->getPlug(i)->processingError())
                {
                    item->setBackgroundColor(0, pcolor_red);
                    item->setBackgroundColor(1, pcolor_red);
                    item->setBackgroundColor(2, pcolor_red);
                    item->setBackgroundColor(3, pcolor_red);
                }
                else if(pchain->getPlug(i)->getName() == TemperatureCalculator::pluginName)
                {
                    item->setBackgroundColor(0, pcolor_green);
                    item->setBackgroundColor(1, pcolor_green);
                    item->setBackgroundColor(2, pcolor_green);
                    item->setBackgroundColor(3, pcolor_green);
                }
                else if(pchain->getPlug(i)->getName() != MultiSignalAverage::pluginName)
                {
                    item->setBackgroundColor(0, pcolor_red);
                    item->setBackgroundColor(1, pcolor_red);
                    item->setBackgroundColor(2, pcolor_red);
                    item->setBackgroundColor(3, pcolor_red);
                }
            }
        }
    }
    m_mutePPTWIchanges = false;
}


void SignalProcessingEditor::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}


void SignalProcessingEditor::keyReleaseEvent(QKeyEvent *event)
{
    //if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    //    onRecalcClicked();

    QWidget::keyReleaseEvent(event);
}


// -------------------------------------
// HANDLE STATE CHANGES OF SIGNALMANAGER
// -------------------------------------


void SignalProcessingEditor::handleImportStateChanged(bool state)
{
    if(state)
    {
        setCursor(Qt::WaitCursor);
    }
    else
    {
        setCursor(Qt::ArrowCursor);


        if(!m_mrun)
            return;

        int nSigs = m_mrun->sizeAllMpoints();
        int nValid = m_mrun->sizeValidMpoints();
        sigCountLabel->setText( QString("    (%0 signals, valid: %1)")
                    .arg(nSigs)
                    .arg(nValid));

        sigSpin->setMaximum(nSigs);
        if(nSigs < 1)
            sigSpin->setMinimum(0);
        else
            sigSpin->setMinimum(1);
        rawPlot->setZoomMode(SignalPlotWidgetQwt::ZOOM_RESET);
        absPlot->setZoomMode(SignalPlotWidgetQwt::ZOOM_RESET);
        tempPlot->setZoomMode(SignalPlotWidgetQwt::ZOOM_RESET);

        // update channel id list in plot for
        // correct checkbox and colormap generation!
        rawPlot->setChannels(m_mrun->channelIDs(Signal::RAW));
        absPlot->setChannels(m_mrun->channelIDs(Signal::ABS));
        tempPlot->setChannels(m_mrun->channelIDs(Signal::TEMPERATURE));

        replot();
    }
}


void SignalProcessingEditor::handleProcessingStateChanged(bool state)
{
    if(state)
    {
        setCursor(Qt::WaitCursor);
        m_mutePPTWIchanges = true;
    }
    else
    {
        m_mutePPTWIchanges = false;
        replot();
        updateItemColors();
        setCursor(Qt::ArrowCursor);

        if(!m_mrun)
            return;

        int nSigs = m_mrun->sizeAllMpoints();
        int nValid = m_mrun->sizeValidMpoints();
        sigCountLabel->setText( QString("    (%0 signals, valid: %1)")
                    .arg(nSigs)
                    .arg(nValid));
    }
}


void SignalProcessingEditor::handleExportStateChanged(bool state)
{
    if(state)
    {
        setCursor(Qt::WaitCursor);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
    }
}


// ---------------------------------
// HANDLE CHANGES OF DATAMODEL
// ---------------------------------


void SignalProcessingEditor::onMRunAdded(MRun *mrun)
{
    groupTree->expandAll();

    /*
    ProcessingChain* tChain = mrun->getProcessingChain(Signal::TEMPERATURE);

    if(tChain->noPlugs() == 0)
    {
        ProcessingPlugin* tCalc = new TemperatureCalculator(tChain);
        tChain->addPlug(tCalc);
    }*/

    // show the first mrun which is added to the model
    if(m_dataModel->mrunCount() == 1)
    {
        m_mrun = mrun;
        runNameLabel->setText(mrun->getName());

        for(int i = 0; i < groupTree->topLevelItemCount(); i++)
        {
            if(groupTree->topLevelItem(i)->childCount()>0)
            {
                QList<QTreeWidgetItem*> initSelection;
                initSelection << groupTree->topLevelItem(i)->child(0);
                onMRunSelectionChanged(initSelection);
                break;
            }
        }
    }
}


// ---------------------------------------------
// HANDLE DATAITEM MODIFICATIONS OF CURRENT MRUN
// ---------------------------------------------


void SignalProcessingEditor::onMRunDataChanged()
{
    if(!m_mrun)
        return;

    runNameLabel->setText(m_mrun->getName());
}


void SignalProcessingEditor::onMRunDestroyed()
{
    m_mrun = 0;
    mrunDetailsView->setRun(0);
}


void SignalProcessingEditor::onMRunChannelCountChanged(Signal::SType stype, int count)
{
    if(!m_mrun)
        return;

    switch(stype)
    {
    case Signal::RAW:
        rawPlot->setChannels(m_mrun->channelIDs(Signal::RAW));
        break;
    case Signal::ABS:
        absPlot->setChannels(m_mrun->channelIDs(Signal::ABS));
        break;
    case Signal::TEMPERATURE:
        tempPlot->setChannels(m_mrun->channelIDs(Signal::TEMPERATURE));
    }
}


// ----------------------
// INTERACTION WITH PLOTS
// ----------------------


void SignalProcessingEditor::replot()
{
    m_stype = Signal::RAW;
    m_curPlot = rawPlot;
    plotCurrentPlugins();
   // if(!m_lastRawPluginSelection.isEmpty())
         //plotSelectedPlugins(m_lastRawPluginSelection);
    if(m_mrun != nullptr)
        m_curPlot->setDataTableRunName(m_mrun->name);

    m_stype = Signal::ABS;
    m_curPlot = absPlot;
    plotCurrentPlugins();
   // if(!m_lastAbsPluginSelection.isEmpty())
         //plotSelectedPlugins(m_lastAbsPluginSelection);
    if(m_mrun != nullptr)
        m_curPlot->setDataTableRunName(m_mrun->name);

    m_stype = Signal::TEMPERATURE,
    m_curPlot = tempPlot;
    plotCurrentPlugins();
   // if(!m_lastTmpPluginSelection.isEmpty())
         //plotSelectedPlugins(m_lastTmpPluginSelection);
    if(m_mrun != nullptr)
        m_curPlot->setDataTableRunName(m_mrun->name);
}


void SignalProcessingEditor::onPlotLinkToggled(bool state )
{    
    rawPlot->setZoomMode(SignalPlotWidgetQwt::ZOOM_RESET);
    absPlot->setZoomMode(SignalPlotWidgetQwt::ZOOM_RESET);
    tempPlot->setZoomMode(SignalPlotWidgetQwt::ZOOM_RESET);
}


void SignalProcessingEditor::onXViewChanged(double xmin, double xmax)
{
    rawPlot->setXView(xmin,xmax);
    absPlot->setXView(xmin,xmax);
    tempPlot->setXView(xmin,xmax);
}


/**
 * @brief SignalProcessingEditor::onCurrentPlotToolChanged
 * This slot is executed when the user changed the current plot
 * tool. The related toolbar actions only control the tool selection
 * of the 'raw' plot. This slot is used to copy the current tool selection
 * to the 'absolute' and 'temperature' signal plots.
 * @param mode
 */
void SignalProcessingEditor::onCurrentPlotToolChanged(BasePlotWidgetQwt::PlotZoomMode mode)
{    
    absPlot->setZoomMode(mode);
    tempPlot->setZoomMode(mode);    
}


void SignalProcessingEditor::plotCurrentPlugins()
{
    if(!m_curPlot)
        return;
    m_curPlot->detachAllCurves();
    m_curPlot->plotTextLabelVisible(false);

    if(!m_mrun)
        return;

    int noCh = m_mrun->getNoChannels(m_stype);
    QList<int> chids = m_mrun->channelIDs(m_stype);

    ProcessingChain* pchain = m_mrun->getProcessingChain(m_stype);

    DataItemTreeView* curTreeView =0;
    if(m_stype == Signal::RAW)
        curTreeView = rawTree;
    else if(m_stype == Signal::ABS)
        curTreeView = absTree;
    else if(m_stype == Signal::TEMPERATURE)
        curTreeView = tempTree;

    if(!pchain)
        return;

    if(!m_curPlot)
        return;

    m_curPlot->detachAllCurves();
    int curveCount = 0;

    try
    {
        int msai = pchain->indexOfPlugin(MultiSignalAverage::pluginName);

        bool allEmpty = true;

        // plot unprocessed signal data
        if( m_stype != Signal::TEMPERATURE
            && curTreeView->topLevelItemCount() >0
            && curTreeView->topLevelItem(0)->data(0,Qt::UserRole+5).toBool()
                && m_mrun->sizeAllMpoints() > m_curMPoint )
        {

            MPoint* mp = m_mrun->getPre(m_curMPoint);
            QString label;
            for(int c = 0; c < chids.size(); c++)
            {
                Signal s = mp->getSignal(chids[c],m_stype);

                label.sprintf("Channel %d ",chids[c]);

                if(s.data.isEmpty())
                    label.append(" (empty)");
                else
                    allEmpty = false;

                m_curPlot->addSignal(s,label);
            }

            if(allEmpty && m_stype == Signal::ABS)
            {
                m_curPlot->setPlotLabelText("No signal data available. Use the \"Overwrite\"-plugin to copy data from the raw signal.");
                m_curPlot->plotTextLabelVisible(true);
            }
            else if(allEmpty && m_stype == Signal::RAW)
            {
                m_curPlot->setPlotLabelText("No signal data available.");
                m_curPlot->plotTextLabelVisible(true);
            }
        }

        bool noTempCalculator = true;
        bool allTempSignalEmpty = true;
        bool tempPlugDirty = false;

        // workaround for custom plot visibility behavior of
        // temperature calculators: show/hide channels instead of step buffers
        QList<int> hiddenTchannels; // list of disabled temperature plugins
        TemperatureProcessingChain* tpchain = 0;
        if(m_stype == Signal::TEMPERATURE)
        {
            // find hidden channels,
            // for temperature calculators: add temperaturechannel id to
            // hidden-channel-list if plot visibility is diabled.
            tpchain = dynamic_cast<TemperatureProcessingChain*>(pchain);
            for(int i = 0; i < tpchain->temperatureCalculatorCont();i++)
            {
                TemperatureCalculator* tc = dynamic_cast<TemperatureCalculator*>(tpchain->getPlug(i));
                if(tc && !tpchain->getPlug(i)->plotVisibility())
                    hiddenTchannels << tc->temperatureChannelID();

                if(tc->dirty())
                    tempPlugDirty = true;

                if(tc && tc->plotVisibility()
                        && (curTreeView->topLevelItemCount() > 0
                            && curTreeView->topLevelItem(0)->data(0,Qt::UserRole+5).toBool()))
                {
                    Signal signal = tc->processedSignal(m_curMPoint, tc->temperatureChannelID());

                    QString label = QString("T%0 ").arg(tc->temperatureChannelID());

                    if(signal.data.isEmpty())
                        label.append(" (empty)");
                    else
                        allTempSignalEmpty = false;

                    //qDebug() << "SignalProcessingEditor: err: "
                    //         << tc->temperatureChannelID()
                    //         << tc->processingError();

                    if(tc->processingError())
                    {
                        // clear signal if anything was processed
                        signal.data.clear();
                        label.append(" (Calculation Error!)");                        
                    }

                    m_curPlot->addSignal(signal, label);
                }
                if(tpchain->temperatureCalculatorCont())
                    noTempCalculator = false;
            }
            m_curPlot->setMaxLegendColumns(noCh- hiddenTchannels.size());
        }


        // plot processed data
        for(int i = 0; i < pchain->noPlugs();i++)
        {
            bool msa = false;

            int pos = i;// pluginPos.at(i);
            if(pos < -1 || pos >= pchain->noPlugs())
                continue;

            if(msai > -1 && pos >= msai)
                msa = true;

            // do not print data of deactivated plugins
            if(!pchain->getPlug(pos)->activated())
                continue;

            // plot workaround for temperature processing steps ...
            if(m_stype == Signal::TEMPERATURE)
            {
                // Do not skip step plot for temperature calculators,
                // even if plot visibility is disabled!
                if(pos >= tpchain->temperatureCalculatorCont() &&
                   !pchain->getPlug(pos)->plotVisibility())
                    continue;
            }
            // for raw/abs processing chains: skip plot
            // of step if visibility is disabled
            else
            {
                if(!pchain->getPlug(pos)->plotVisibility())
                    continue;
            }

            allEmpty = true;
            for(int c=0; c< chids.size(); c++)
            {
                //skip all temp calculators here since we added them already
                if(pchain->getPlug(pos)->getName() == "Temperature Calculator")
                    continue;
                // plot workaround for temperature processing steps:
                // skip hidden channels of current step
                if(m_stype == Signal::TEMPERATURE &&
                   hiddenTchannels.contains(chids[c]))
                    continue;


                Signal s;
                if(!msa)
                    s = pchain->getPlug(pos)->processedSignal(m_curMPoint,chids[c]);
                else
                    s = pchain->getPlug(pos)->processedSignal(0,chids[c]);

                QString val;
                if(m_stype == Signal::RAW || m_stype == Signal::ABS)
                    val = QString("%0 (Step %1) - Ch%2 ").arg(pchain->getPlug(pos)->getName())
                                                         .arg(pos+1).arg(chids[c]);
                else
                    val = QString("%0 (Step %1) - T%2 ").arg(pchain->getPlug(pos)->getName())
                                                         .arg(pos).arg(chids[c]);

                if(s.data.isEmpty())
                    val.append(" (empty)");
                else
                    allEmpty = false;

                m_curPlot->addSignal(s,val);
                curveCount++;
            }
        }
        if(m_stype == Signal::TEMPERATURE && allEmpty && tempPlugDirty &&allTempSignalEmpty && (curTreeView->topLevelItemCount() > 0 && curTreeView->topLevelItem(0)->data(0,Qt::UserRole+5).toBool()))
        {
            m_curPlot->setPlotLabelText("No temperature signal data available. Please calculate a signal to show any data.");
            m_curPlot->plotTextLabelVisible(true);
        }
        else if(m_stype == Signal::TEMPERATURE && allEmpty && allTempSignalEmpty && !chids.isEmpty() && (curTreeView->topLevelItemCount() > 0 && curTreeView->topLevelItem(0)->data(0,Qt::UserRole+5).toBool()))
        {
            m_curPlot->setPlotLabelText("No temperature signal data to display. Check your visibility settings.");
            m_curPlot->plotTextLabelVisible(true);
        }
        else if(m_stype == Signal::TEMPERATURE && !(curTreeView->topLevelItemCount() > 0 && curTreeView->topLevelItem(0)->data(0,Qt::UserRole+5).toBool()) && curveCount == 0)
        {
            m_curPlot->setPlotLabelText("No temperature signal data to display. Check your visibility settings.");
            m_curPlot->plotTextLabelVisible(true);
        }
        if(m_stype == Signal::TEMPERATURE && curveCount == 0 && noTempCalculator)
        {
            m_curPlot->setPlotLabelText("No temperature signal data available. Add a temperature calculator plugin.");
            m_curPlot->plotTextLabelVisible(true);
        }
    }
    catch(LIISimException e)
    {        
        MESSAGE(e.what(), e.type());
    }
}


void SignalProcessingEditor::onPlotTypeChanged(BasePlotWidgetQwt::PlotType type)
{
    QString plotWindow;

    if(QObject::sender() == rawPlot)
        plotWindow = "plottyperaw";
    else if(QObject::sender() == absPlot)
        plotWindow = "plottypeabs";
    else if(QObject::sender() == tempPlot)
        plotWindow = "plottypetemp";

    switch(type)
    {
    case BasePlotWidgetQwt::LINE_CROSSES: Core::instance()->guiSettings->setValue("signalprocessing", plotWindow, 0); break;
    case BasePlotWidgetQwt::LINE:         Core::instance()->guiSettings->setValue("signalprocessing", plotWindow, 1); break;
    case BasePlotWidgetQwt::DOTS_SMALL:   Core::instance()->guiSettings->setValue("signalprocessing", plotWindow, 2); break;
    case BasePlotWidgetQwt::DOTS_MEDIUM:  Core::instance()->guiSettings->setValue("signalprocessing", plotWindow, 3); break;
    case BasePlotWidgetQwt::DOTS_LARGE:   Core::instance()->guiSettings->setValue("signalprocessing", plotWindow, 4); break;
    }
}


/**
 * @brief SignalProcessingEditor::onCalcToolboxRecalc Executed when the CalculationToolbox emits
 * a recalc-signal. Initiates the signal processing.
 * @param typeList List containing the signal types which should be processed.
 */
void SignalProcessingEditor::onCalcToolboxRecalc(QList<Signal::SType> typeList)
{
    if(!m_mrun)
    {
        QString msg = "No measurement run selected!";
        MSG_WARN(msg);
        MSG_STATUS(msg);
        return;
    }
    Core::instance()->getSignalManager()->processSignals(m_mrun, typeList);
}


// ------------------------------
// HANDLE GLOBAL SETTINGS SIGNALS
// ------------------------------


/**
 * @brief SignalProcessingEditor::onGuiSettingsChanged This slot
 * is executed when the global Gui Settings changed
 */
void SignalProcessingEditor::onGuiSettingsChanged()
{
    GuiSettings* gs = Core::instance()->guiSettings;

    // set Calculation Mode

    int calcMode = gs->value("se","calcMode",0).toInt();
    Core::instance()->getSignalManager()->setCalculationMode(calcMode);

    if(gs->hasEntry("signalprocessing", "plottyperaw"))
    {
        switch(Core::instance()->guiSettings->value("signalprocessing", "plottyperaw", 0).toUInt())
        {
        case 0: rawPlot->setPlotType(BasePlotWidgetQwt::LINE_CROSSES); break;
        case 1: rawPlot->setPlotType(BasePlotWidgetQwt::LINE); break;
        case 2: rawPlot->setPlotType(BasePlotWidgetQwt::DOTS_SMALL); break;
        case 3: rawPlot->setPlotType(BasePlotWidgetQwt::DOTS_MEDIUM); break;
        case 4: rawPlot->setPlotType(BasePlotWidgetQwt::DOTS_LARGE); break;
        }
    }
    if(gs->hasEntry("signalprocessing", "plottypeabs"))
    {
        switch(Core::instance()->guiSettings->value("signalprocessing", "plottypeabs", 0).toUInt())
        {
        case 0: absPlot->setPlotType(BasePlotWidgetQwt::LINE_CROSSES); break;
        case 1: absPlot->setPlotType(BasePlotWidgetQwt::LINE); break;
        case 2: absPlot->setPlotType(BasePlotWidgetQwt::DOTS_SMALL); break;
        case 3: absPlot->setPlotType(BasePlotWidgetQwt::DOTS_MEDIUM); break;
        case 4: absPlot->setPlotType(BasePlotWidgetQwt::DOTS_LARGE); break;
        }
    }
    if(gs->hasEntry("signalprocessing", "plottypetemp"))
    {
        switch(Core::instance()->guiSettings->value("signalprocessing", "plottypetemp", 0).toUInt())
        {
        case 0: tempPlot->setPlotType(BasePlotWidgetQwt::LINE_CROSSES); break;
        case 1: tempPlot->setPlotType(BasePlotWidgetQwt::LINE); break;
        case 2: tempPlot->setPlotType(BasePlotWidgetQwt::DOTS_SMALL); break;
        case 3: tempPlot->setPlotType(BasePlotWidgetQwt::DOTS_MEDIUM); break;
        case 4: tempPlot->setPlotType(BasePlotWidgetQwt::DOTS_LARGE); break;
        }
    }

    QVariant position;
    if(gs->getSplitterPosition(identifier_mainSplitter, position))
        mainHsplit->restoreState(position.toByteArray());
    if(gs->getSplitterPosition(identifier_plotSplitter, position))
        plotVsplit->restoreState(position.toByteArray());
    if(gs->getSplitterPosition(identifier_plugSplitter, position))
        plugVsplit->restoreState(position.toByteArray());
    if(gs->getSplitterPosition(identifier_treeDetailsSplitter, position))
        leftVSplit->restoreState(position.toByteArray());
}


void SignalProcessingEditor::onSignalManagerImportFinished()
{
    onMRunSelectionChanged(mLastSelection, true);
}


void SignalProcessingEditor::onSplitterMoved()
{
    if(QObject::sender() == mainHsplit)
        Core::instance()->guiSettings->setSplitterPosition(identifier_mainSplitter, mainHsplit->saveState());
    else if(QObject::sender() == plotVsplit)
        Core::instance()->guiSettings->setSplitterPosition(identifier_plotSplitter, plotVsplit->saveState());
    else if(QObject::sender() == plugVsplit)
        Core::instance()->guiSettings->setSplitterPosition(identifier_plugSplitter, plugVsplit->saveState());
    else if(QObject::sender() == leftVSplit)
        Core::instance()->guiSettings->setSplitterPosition(identifier_treeDetailsSplitter, leftVSplit->saveState());
}

