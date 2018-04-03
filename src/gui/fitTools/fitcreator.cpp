#include "fitcreator.h"

#include <QMessageBox>
#include <QHeaderView>

#include "../../core.h"
#include "../../signal/signalmanager.h"

#include "../../calculations/fit/fitrun.h"
#include "../../calculations/fit/fitdata.h"

#include "../../calculations/fit/simrun.h"

#include "../../signal/processing/processingchain.h"
#include "../../signal/processing/plugins/temperaturecalculator.h"

#include "../../calculations/numeric.h"

#include "../dataItemViews/treeview/dataitemtreeview.h"
#include "../utils/mrundetailswidget.h"

#include "ft_modelingsettingstable.h"
#include "ft_fitparamtable.h"
#include "ft_numericparamtable.h"
#include "ft_parametertable.h"
#include "ft_datavisualization.h"
#include "ft_simsettings.h"


const QString FitCreator::identifier_mainSplitter = "fit_creator_main_splitter";
const QString FitCreator::identifier_treeDetailsSplitter = "fit_creator_tree_details";
const QString FitCreator::identifier_fitListDataSplitter = "fit_creator_fit_list_data";
const QString FitCreator::identifier_plotSettingsSplitter = "fit_creator_plot_settings";
const QString FitCreator::identifier_listVisualizationSplitter = "fit_creator_list_visualization";
const QString FitCreator::identifier_HPlSeFlRvSplitter = "fit_creator_hplseflrv";


FitCreator::FitCreator(QWidget *parent) : QWidget(parent)
{
    MSG_DETAIL_1("init FitCreator");

    setObjectName("FitCreator");

    m_title = "Fit Creator";
    m_iconLocation = Core::rootDir + "resources/icons/vector.png";

    m_viewAction = new QAction(QIcon(m_iconLocation),"Fit Creator",this);
    m_viewAction->setCheckable(true);

    currentRun = nullptr;

    m_mainLayout = new QGridLayout;
    m_mainLayout->setMargin(0);
    setLayout(m_mainLayout);

    // ------------------------
    //  MRun TreeView
    // ------------------------

    MSG_DETAIL_1("init FitCreator MRun selection");

    treeView = new DataItemTreeView(DataItemTreeView::ATOOL, Core::instance()->dataModel()->rootItem());
    QStringList headerLabels;
    headerLabels << "Measurement Runs" << "Signal" << "";
    treeView->setHeaderLabels(headerLabels);
    treeView->setMaximumWidth(280);

    detailsView = new MRunDetailsWidget(this);

    mainSplitter = new QSplitter(Qt::Horizontal, this);

    treeDetailsSplitter = new QSplitter(Qt::Vertical, this);

    treeDetailsSplitter->addWidget(treeView);
    treeDetailsSplitter->addWidget(detailsView);

    mainSplitter->addWidget(treeDetailsSplitter);

    m_mainLayout->addWidget(mainSplitter);

    // ------------------------
    //  FitCreator MainBox
    // ------------------------

    MSG_DETAIL_1("init FitCreator main layout");

    // infotext, fit/cancel button
    // data selection

    MSG_DETAIL_1("init FitCreator modules");

    runPlot = new FT_RunPlot(this);
    resultVisualization = new FT_ResultVisualization(this);
    dataVisualization = new FT_DataVisualization(this);
    fitList = new FT_FitList(resultVisualization, dataVisualization, this);

    splitterFitListData = new QSplitter(Qt::Vertical, this);
    splitterFitListData->addWidget(fitList);
    splitterFitListData->addWidget(dataVisualization);

    MSG_DETAIL_1("init FitCreator buttons");

    // -------------------------------------------
    //
    // -------------------------------------------

    // splitter for top area (A):
    //  run plot (top left) and settings (top right)
    splitterPlotSettings = new QSplitter(Qt::Horizontal, this);
    splitterPlotSettings->addWidget(runPlot);

    //QWidget *listDataWidget = new QWidget(this);
    //listDataWidget->setLayout(new QVBoxLayout);
    //listDataWidget->layout()->setMargin(0);

    //listDataWidget->layout()->addWidget(fitList);
    //listDataWidget->layout()->addWidget(dataVisualization);

    // splitter for bottom area (B):
    //  result plot (bottom left) and result data (bottom right)
    splitterListVisualization = new QSplitter(Qt::Horizontal, this);
    splitterListVisualization->addWidget(resultVisualization);
    //splitterListVisualization->addWidget(listDataWidget);
    splitterListVisualization->addWidget(splitterFitListData);

    splitterListVisualization->setStretchFactor(0, 3);
    splitterListVisualization->setStretchFactor(1, 4);

    // splitter between top (A) and bottom (B) area
    splitterHPlSeFlRv = new QSplitter(Qt::Vertical, this);
    splitterHPlSeFlRv->addWidget(splitterPlotSettings);
    splitterHPlSeFlRv->addWidget(splitterListVisualization);
    splitterHPlSeFlRv->setStretchFactor(0, 8);
    splitterHPlSeFlRv->setStretchFactor(1, 7);

    mainSplitter->addWidget(splitterHPlSeFlRv);

    // -------------------------------------------
    //  Parameter Visualization ( right top box)
    // -------------------------------------------

    SettingsTree *settingsTree = new SettingsTree(this);
    settingsTree->setStyleSheet("QTreeWidget { padding-top: 10px; padding-left: 5px; padding-right: 5px }");
    settingsTree->setColumnCount(1);
    settingsTree->setRootIsDecorated(false);
    settingsTree->header()->setVisible(false);
    settingsTree->setIndentation(0);
    settingsTree->setSelectionMode(QAbstractItemView::NoSelection);
    settingsTree->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);

    QTreeWidgetItem *itemMS = new QTreeWidgetItem(settingsTree);
    itemMS->setText(0, "Modeling Settings");
    QFont font = itemMS->font(0);
    font.setBold(true);
    itemMS->setFont(0, font);

    // modeling params
    MSG_DETAIL_1("init FitCreator modelingsettings gui");
    modelingTable = new FT_ModelingSettingsTable;

    QTreeWidgetItem *itemMSWidget = new QTreeWidgetItem(itemMS);
    settingsTree->setItemWidget(itemMSWidget, 0, modelingTable);
    itemMS->setExpanded(true);

    // fit params
    MSG_DETAIL_1("init FitCreator fitparameter gui");
    fitparamTable = new FT_FitParamTable;

    QTreeWidgetItem *itemFP = new QTreeWidgetItem(settingsTree);
    itemFP->setFont(0, font);
    itemFP->setText(0, "Initial Fit Parameters");

    QTreeWidgetItem *itemFPWidget = new QTreeWidgetItem(itemFP);
    settingsTree->setItemWidget(itemFPWidget, 0, fitparamTable);
    itemFP->setExpanded(true);

    // numerical params
    MSG_DETAIL_1("init FitCreator numerical parameter vis");
    numparamTable = new FT_NumericParamTable;

    QTreeWidgetItem *itemNP = new QTreeWidgetItem(settingsTree);
    itemNP->setFont(0, font);
    itemNP->setText(0, "Numeric Settings");

    QTreeWidgetItem *itemNPWidget = new QTreeWidgetItem(itemNP);
    settingsTree->setItemWidget(itemNPWidget, 0, numparamTable);
    itemNP->setExpanded(true);

    // simulation only parameters
    simSettings = new FT_SimSettings(this);

    QTreeWidgetItem *itemSS = new QTreeWidgetItem(settingsTree);
    itemSS->setFont(0, font);
    itemSS->setText(0, "Simulation Only Settings");

    QTreeWidgetItem *itemSSWidget = new QTreeWidgetItem(itemSS);
    settingsTree->setItemWidget(itemSSWidget, 0, simSettings);
    itemSS->setExpanded(true);

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    splitterPlotSettings->addWidget(settingsTree);
    splitterPlotSettings->setStretchFactor(0, 1);
    splitterPlotSettings->setStretchFactor(1, 4);

    // connections
    MSG_DETAIL_1("init FitCreator connections");

    connect(treeView,
            SIGNAL(checkedItemsChanged(QList<QTreeWidgetItem*>)),
            SLOT(onTreeViewSelectionChanged(QList<QTreeWidgetItem*>)));

    connect(Core::instance()->getSignalManager(),
            SIGNAL(processingStateChanged(bool)),
            SLOT(onProcessingStateChanged(bool)));

    connect(Core::instance()->getSignalManager(),
                SIGNAL(fitStateChanged(bool)),
                SLOT(onFitStateChanged(bool)));

    connect(fitList, SIGNAL(startFittingClicked()), SLOT(onFitButtonReleased()));
    connect(fitList, SIGNAL(startSimulationClicked()), SLOT(onSimButtonReleased()));
    connect(fitList, SIGNAL(cancelClicked()), SLOT(onCancelButtonReleased()));

    connect(mainSplitter, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    connect(treeDetailsSplitter, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    connect(splitterFitListData, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    connect(splitterPlotSettings, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    connect(splitterListVisualization, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    connect(splitterHPlSeFlRv, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));

    connect(Core::instance()->guiSettings, SIGNAL(settingsChanged()), SLOT(onGUISettingsChanged()));

    MSG_DETAIL_1("init FitCreator done.");

    //Object naming for tutorial views
    treeView->setObjectName("FC_RUN_TREEVIEW");
    detailsView->setObjectName("FC_RUN_DETAILS");
    runPlot->setObjectName("FC_RUN_PLOT");
    resultVisualization->setObjectName("FC_RESULT_VISUALIZATION");
    dataVisualization->setObjectName("FC_DATA_VISUALIZATION");
    fitList->setObjectName("FC_FIT_LIST");

    modelingTable->setObjectName("FC_MODELING_SETTINGS");
    fitparamTable->setObjectName("FC_INIT_FIT_PARAMETERS");
    numparamTable->setObjectName("FC_NUMERIC_SETTINGS");
    simSettings->setObjectName("FC_SIMULATION_ONLY_SETTINGS");
}


FitCreator::~FitCreator() {}


// ------------------------------------------------
// HANDLERS FOR PROGRAM/DATA/SETTINGS STATE CHANGES
// ------------------------------------------------


/**
 * @brief FitCreator::onProcessingStateChanged This slot is
 * executed when signalprocessing has been started or finished
 * (see signal: SignalManager::processingStateChanged(bool)).
 * @param state
 */
void FitCreator::onProcessingStateChanged(bool state)
{
    if(state)
        setCursor(Qt::WaitCursor);
    else
    {
        setCursor(Qt::ArrowCursor);

        QList<MRun*> mruns;
        for(int runID : m_selectedRunIDs)
            mruns << Core::instance()->dataModel()->mrun(runID);

        runPlot->update(mruns);
    }
}


/**
 * @brief FitCreator::onFitStateChanged This slot is executed when the SignalManager's
 * fitStateChanged()signal is emitted
 * @param state
 */
void FitCreator::onFitStateChanged(bool state)
{
    if(state)
    {
        setCursor(Qt::WaitCursor);

        QString msg = "Fitting signals (please wait) ...";
        MSG_STATUS_CONST(msg);
        MSG_NORMAL(msg);                
    }
    else
    {
        setCursor(Qt::ArrowCursor);

        QString msg = "All fits done.";
        MSG_STATUS_CONST(msg);
        MSG_NORMAL(msg);
    }
}


// -----------------------------
// HANDLERS FOR USER INTERACTION
// -----------------------------

/**
 * @brief FitCreator::onTreeViewSelectionChanged This slot is executed
 * when the user has changed the measurement run selection.
 * It extracts a list of measurement run IDs from the given selection
 * and updates the selection information label.
 * (see DataItemTreeView::checkedItemsChanged(..))
 */
void FitCreator::onTreeViewSelectionChanged(QList<QTreeWidgetItem *> selection)
{
    QList<int> selectedRunIDs;
    for(int i = 0; i < selection.size();i++)
    {
        int userType = selection.at(i)->data(0,Qt::UserRole).toInt();

        if(userType != 2) // continue only with mrun treeitems
            continue;

        selectedRunIDs << selection.at(i)->data(0,Qt::UserRole+1).toInt();
    }
    m_selectedRunIDs = selectedRunIDs;

    if(treeView->treeWidget()->currentItem() != NULL)
    {
        int userData = treeView->treeWidget()->currentItem()->data(0, Qt::UserRole).toInt();
        int data_id = treeView->treeWidget()->currentItem()->data(0, Qt::UserRole+1).toInt();

        if(userData == 2)
            currentRun = Core::instance()->dataModel()->mrun(data_id);
        else
            currentRun = nullptr;
    }
    else
        currentRun = nullptr;

    detailsView->setRun(currentRun);

    QList<MRun*> mruns;
    for(int runID : selectedRunIDs)
        mruns << Core::instance()->dataModel()->mrun(runID);

    runPlot->update(mruns);
}


/**
 * @brief FitCreator::onFitButtonReleased This slot is executed when the
 * user released the fit button. A new Fitrun will be created and
 * executed.
 */
void FitCreator::onFitButtonReleased()
{
    // check if calculation is allowed
    if(Core::instance()->getSignalManager()->isBusy())
    {
        QString msg = "Cannot start fit: active background tasks";
        MSG_STATUS(msg);
        MSG_WARN(msg);
        return;
    }

    // Reset all messages related to HTM check
    MSG_ONCE_RESET_GROUP("HTM_checkAvailability");

    // check if all variables are available
    if(!Core::instance()->modelingSettings->heatTransferModel()->checkAvailability())
        return;

    // collect all signaldata for fit
    QList<FitData> data;

    DataModel* dm = Core::instance()->dataModel();

    if(m_selectedRunIDs.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("No MRun for FitRun selected.");
        msgBox.setInformativeText("Please select at least one MRun to perform a FitRun.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    QList<int> channel = runPlot->getSelectedChannel();
    if(channel.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("No channel for FitRun selected.");
        msgBox.setInformativeText("Please select at least one channel to perform a FitRun.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    for(int runID : m_selectedRunIDs)
    {
        MRun *run = dm->mrun(runID);
        if(!run)
            continue;

        Signal::SType stype = runPlot->getSelectedSource();
        int pointNo = run->data(2).toInt();

        if(stype == Signal::RAW || stype == Signal::ABS)
        {
            MPoint *mp = run->getPost(pointNo);
            if(!mp)
                continue;

            for(int channelNo : channel)
            {
                if(!mp->channelIDs(stype).contains(channelNo))
                    continue;

                if(runPlot->rangeValid())
                    data << FitData(runID, pointNo, stype, channelNo, true, runPlot->getRangeStart(), runPlot->getRangeEnd());
                else
                    data << FitData(runID, pointNo, stype, channelNo);
            }
        }
        else if(stype == Signal::TEMPERATURE)
        {
            int noMPoints = run->sizeValidMpoints();

            for(int channelNo : channel)
            {
                for(int pointNo = 0; pointNo < noMPoints; pointNo++)
                {
                    MPoint *mp = run->getPost(pointNo);
                    if(!mp)
                        continue;
                    if(mp->channelCount(Signal::TEMPERATURE) < 1)
                        continue;

                    QList<int> cids = mp->channelIDs(Signal::TEMPERATURE);
                    if(!cids.contains(channelNo))
                        continue;

                    if(mp->getSignal(1, Signal::TEMPERATURE).data.isEmpty())
                        continue;

                    if(pointNo == 0)
                    {
                        if(run->getProcessingChain(Signal::TEMPERATURE)->containsActiveMSA())
                        {
                            if(runPlot->rangeValid())
                                data << FitData(runID, pointNo, runPlot->getSelectedSource(), channelNo, true, runPlot->getRangeStart(), runPlot->getRangeEnd());
                            else
                                data << FitData(runID, pointNo, runPlot->getSelectedSource(), channelNo);
                            break;
                        }
                    }
                    if(runPlot->rangeValid())
                        data << FitData(runID, pointNo, Signal::TEMPERATURE, channelNo, true, runPlot->getRangeStart(), runPlot->getRangeEnd());
                    else
                        data << FitData(runID, pointNo, Signal::TEMPERATURE, channelNo);
                }
            }
        }
    }

    if(data.isEmpty())
    {
        QString msg = "Cannot start fit: no temperature signals selected.";
        MSG_STATUS(msg);
        MSG_WARN(msg);
        return;
    }

    if(data.size() > 3)
    {
        QMessageBox msgBox;
        msgBox.setText(QString("You selected %0 Signals for fit.")
                       .arg(QString::number(data.size())));
        msgBox.setInformativeText(QString("Do you really want to perform %0 fits?\nThis could take some time!")
                                  .arg(QString::number(data.size())));
        msgBox.setStandardButtons(QMessageBox::Yes |  QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();

        if(ret == QMessageBox::Cancel)
            return;
    }

    // get list of fit parameters
    QList<FitParameter> fparams = fitparamTable->fitParameters();

    // needs to assigned first, otherwise
    // signal of modeling settings will overwrite LineEdit
    double pressure = modelingTable->getPressure();
    Core::instance()->modelingSettings->setProcessPressure(pressure);

    FitSettings* fitSettings = new FitSettings;
    fitSettings->setFitParameters(fparams);

    NumericSettings* numSettings = new NumericSettings;
    numSettings->setIterations(numparamTable->iterations());
    numSettings->setOdeSolver(numparamTable->ODE());
    numSettings->setOdeSolverStepSizeFactor(numparamTable->ODE_stepSizeFactor());
    //numSettings->setStepSize(); // not used by fitting, is defined by experimental data

    FitRun* fitrun = new FitRun(FitRun::PSIZE);
    fitrun->blockSignals(true);

    fitrun->setFitData(data);
    fitrun->setFitSettings(fitSettings);
    fitrun->setNumericSettings(numSettings);

    if(runPlot->rangeValid())
        fitrun->setSection(runPlot->getRangeStart(), runPlot->getRangeEnd());

    fitrun->blockSignals(false);    
    fitrun->fitAll();
}


/**
 * @brief FitCreator::onSimButtonReleased simulates Temperature Signal using current Modeling Settings, Fit/Numeric Parameters
 */
void FitCreator::onSimButtonReleased()
{
    // Reset all messages related to HTM check
    MSG_ONCE_RESET_GROUP("HTM_checkAvailability");

    // check if all variables are available
    if(!Core::instance()->modelingSettings->heatTransferModel()->checkAvailability())
        return;

    // get list of fit parameters
    QList<FitParameter> fparams = fitparamTable->fitParameters();

    FitSettings* fitSettings = new FitSettings;
    fitSettings->setFitParameters(fparams);

    NumericSettings* numSettings = new NumericSettings;
    numSettings->setStartTime(simSettings->startTime());
    numSettings->setSimLength(simSettings->length());
    numSettings->setOdeSolver(numparamTable->ODE());

    numSettings->setOdeSolverStepSizeFactor(1); // ignore ODE stepSize for simulations

    // DEBUG step size factor:
    /*
    numSettings->setOdeSolverStepSizeFactor(numparamTable->ODE_stepSizeFactor());
    qDebug() << "FitCreator: change numSettings->setOdeSolverStepSizeFactor(1)";
    */

    numSettings->setStepSize(simSettings->stepSize());

    // needs to assigned first, otherwise
    // signal of modeling settings will overwrite LineEdit
    double pressure = modelingTable->getPressure();

    Core::instance()->modelingSettings->setProcessPressure(pressure);

    SimRun* simrun = new SimRun;    
    simrun->setFitSettings(fitSettings);
    simrun->setNumericSettings(numSettings);
    simrun->simulate();

    fitList->onNewSimRun(simrun);
}


/**
 * @brief FitCreator::onCancelButtonReleased this slot is executed when the cancel button has been released
 */
void FitCreator::onCancelButtonReleased()
{
    Numeric::canceled = true;
}


/**
 * @brief FitCreator::onCalcToolboxRecalcExecuted when the CalculationToolbox emits
 * a recalc-signal. Initiates the signal processing.
 * @param typeList List containing the signal types which should be processed.
 */
void FitCreator::onCalcToolboxRecalc(QList<Signal::SType> typeList)
{
    if(!currentRun)
    {
        QString msg = "No measurement run selected!";
        MSG_WARN(msg);
        MSG_STATUS(msg);
        return;
    }
    Core::instance()->getSignalManager()->processSignals(currentRun, typeList);
}


void FitCreator::onSplitterMoved()
{
    if(QObject::sender() == mainSplitter)
        Core::instance()->guiSettings->setSplitterPosition(identifier_mainSplitter, mainSplitter->saveState());
    else if(QObject::sender() == treeDetailsSplitter)
        Core::instance()->guiSettings->setSplitterPosition(identifier_treeDetailsSplitter, mainSplitter->saveState());
    else if(QObject::sender() == splitterFitListData)
        Core::instance()->guiSettings->setSplitterPosition(identifier_fitListDataSplitter, splitterFitListData->saveState());
    else if(QObject::sender() == splitterPlotSettings)
        Core::instance()->guiSettings->setSplitterPosition(identifier_plotSettingsSplitter, splitterPlotSettings->saveState());
    else if(QObject::sender() == splitterListVisualization)
        Core::instance()->guiSettings->setSplitterPosition(identifier_listVisualizationSplitter, splitterListVisualization->saveState());
    else if(QObject::sender() == splitterHPlSeFlRv)
        Core::instance()->guiSettings->setSplitterPosition(identifier_HPlSeFlRvSplitter, splitterHPlSeFlRv->saveState());
}


void FitCreator::onGUISettingsChanged()
{
    QVariant position;
    if(Core::instance()->guiSettings->getSplitterPosition(identifier_treeDetailsSplitter, position))
        treeDetailsSplitter->restoreState(position.toByteArray());
    if(Core::instance()->guiSettings->getSplitterPosition(identifier_mainSplitter, position))
        mainSplitter->restoreState(position.toByteArray());
    if(Core::instance()->guiSettings->getSplitterPosition(identifier_fitListDataSplitter, position))
        splitterFitListData->restoreState(position.toByteArray());
    if(Core::instance()->guiSettings->getSplitterPosition(identifier_plotSettingsSplitter, position))
        splitterPlotSettings->restoreState(position.toByteArray());
    if(Core::instance()->guiSettings->getSplitterPosition(identifier_listVisualizationSplitter, position))
        splitterListVisualization->restoreState(position.toByteArray());
    if(Core::instance()->guiSettings->getSplitterPosition(identifier_HPlSeFlRvSplitter, position))
        splitterHPlSeFlRv->restoreState(position.toByteArray());
}

