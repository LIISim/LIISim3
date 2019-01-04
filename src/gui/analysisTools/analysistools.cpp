#include "analysistools.h"

#include <QDebug>
#include <QHBoxLayout>

#include "../../core.h"
#include "../../signal/signalmanager.h"
#include "../signalEditor/memusagewidget.h"

#include "tools/signalplottool.h"
#include "tools/atoolabscalibration.h"
#include "tools/atoolcalibration.h"
#include "tools/atooltemperaturefit.h"
#include "tools/parameteranalysis.h"
#include "tools/measurementlist.h"


const QString AnalysisTools::_identifierSplitterH = "analysisToolsH";
const QString AnalysisTools::_identifierSplitterV = "analysisToolsV";


/**
 * @brief AnalysisTools::AnalysisTools Constructor
 * @param parent parent Widget
 */
AnalysisTools::AnalysisTools(QWidget *parent) : QWidget(parent)
{
    MSG_DETAIL_1("init AnalysisTools");
    QHBoxLayout* layHmain = new QHBoxLayout;
    layHmain->setMargin(0);
    setLayout(layHmain);

    m_selectedSignalType = Signal::RAW;
    firstRun = true;
    currentRun = nullptr;

    // ----------------------
    // INIT GUI ELEMENTS
    // ----------------------

    horizontalSplitter = new QSplitter(Qt::Horizontal);
    layHmain->addWidget(horizontalSplitter);

    verticalSplitterLeft = new QSplitter(Qt::Vertical);
    verticalSplitterLeft->setChildrenCollapsible(false);
    horizontalSplitter->addWidget(verticalSplitterLeft);

    // init internal toolbar (channel/signaltype selection)
    //QHBoxLayout* toolbarLayout = new QHBoxLayout;

    //LabeledComboBox* cbSigtype = new LabeledComboBox("Signal type:");
    //cbSigtype->setMaximumWidth(150);
    //cbSigtype->addStringItem("raw");
    //cbSigtype->addStringItem("absolute");
    //cbSigtype->addStringItem("temperature");
    //toolbarLayout->addWidget(cbSigtype);

    //m_checkBoxLayout = new QHBoxLayout;
    //toolbarLayout->addLayout(m_checkBoxLayout);

    //QWidget *spacer = new QWidget(this);
    //spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    //toolbarLayout->addWidget(spacer);

    //m_internalToolbar = new QToolBar;
    //toolbarLayout->addWidget(m_internalToolbar);

    QVBoxLayout* rightSideLayout = new QVBoxLayout;
    rightSideLayout->setMargin(1);
    //rightSideLayout->addLayout(toolbarLayout);

    atoolstack = new QStackedWidget;
    rightSideLayout->addWidget(atoolstack);

    QWidget* rightSideWidget = new QWidget;
    rightSideWidget->setLayout(rightSideLayout);
    horizontalSplitter->addWidget(rightSideWidget);

    treeView =  new DataItemTreeView(DataItemTreeView::ATOOL);
    treeView->setObjectName("AT_MRUN_TREEVIEW");
    QStringList headerLabels;
    headerLabels << "Measurement Runs" << "Signal" << "";
    treeView->setHeaderLabels(headerLabels);
    treeView->setColumnCount(3);
    //treeView->setIndentation(10);
    //treeView->setMaximumWidth(280);
    treeView->setDataRoot(Core::instance()->dataModel()->rootItem());
    treeView->setMultiSelectionMode(true);
    verticalSplitterLeft->addWidget(treeView);

    //runDetails = new MRunDetailsView;
    runDetails = new MRunDetailsWidget(this);
    runDetails->setObjectName("AT_MRUN_DETAILS");
    verticalSplitterLeft->addWidget(runDetails);

    //selectionWidget = new QWidget(this);
    //selectionWidget->setLayout(toolbarLayout);

    // ----------------------
    // INIT TOOLBAR
    // ----------------------

    m_ribbontoolbar = new QToolBar;

#ifdef LIISIM_FULL
    // "Files" toolbox
    RibbonToolBox* rtbFiles = new RibbonToolBox("SIGNAL DATA");
    rtbFiles->addAction(Core::instance()->getSignalManager()->actionDataImport(),0,0);
    rtbFiles->addAction(Core::instance()->getSignalManager()->actionDataExport(),1,0);

    m_ribbontoolbar->addWidget(rtbFiles);
    m_ribbontoolbar->addSeparator();
#endif

    // "View" toolbox
    rtbView = new RibbonToolBox("ANALYSIS TOOLS");
    atoolActions = new QActionGroup(this);
    atoolActions->setExclusive(true);    

    m_ribbontoolbar->addWidget(rtbView);
    m_ribbontoolbar->addSeparator();

    // "Current View" toolbox
    rtbTool = new RibbonToolBox("CURRENT VIEW");
    labelToolIcon = new QLabel();
    labelToolName = new QLabel("CurrentToolName");

    labelToolIcon->setAlignment(Qt::AlignLeft);
    labelToolName->setAlignment(Qt::AlignLeft);

    rtbTool->addWidget(labelToolIcon, 1, 1);
    rtbTool->addWidget(labelToolName, 1, 2);

    rtbTool->layoutGrid->setColumnMinimumWidth(2, 150);
    rtbTool->setMinimumWidth(200);
    m_ribbontoolbar->addWidget(rtbTool);
    m_ribbontoolbar->addSeparator();


    // "Calculation" toolbox
#ifdef LIISIM_FULL
    calcToolbox = new CalculationToolbox(this);
    m_ribbontoolbar->addWidget(calcToolbox);
    m_ribbontoolbar->addSeparator();
    connect(calcToolbox, SIGNAL(recalc(QList<Signal::SType>)), SLOT(onCalcToolboxRecalc(QList<Signal::SType>)));
#endif

    // add some space to toolbar
    QWidget* spacer2 = new QWidget();
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_ribbontoolbar->addWidget(spacer2);

    m_ribbontoolbar->addSeparator();

    // add notification center
    m_ribbontoolbar->addWidget(Core::instance()->getNotificationManager()->getNotificationCenter(this));
    m_ribbontoolbar->addSeparator();

    // add Memory usage information to toolbar
    MemUsageWidget* memw = new MemUsageWidget;
    m_ribbontoolbar->addWidget(memw);


    // ------------------
    // INIT STATE
    // ------------------

    checkboxes = new QList<QCheckBox*>();

    // add analysis tools
    SignalPlotTool* plotTool = new SignalPlotTool;
    addAtool(plotTool);

#ifdef LIISIM_FULL
    AToolAbsCalibration* calAbsTool = new AToolAbsCalibration;
    addAtool(calAbsTool);

    AToolCalibration* calTool = new AToolCalibration;
    addAtool(calTool);
#endif
    AToolTemperatureFit* fitTool = new AToolTemperatureFit;
    addAtool(fitTool);

    ParameterAnalysis *parameterTool = new ParameterAnalysis;
    addAtool(parameterTool);

    MeasurementList *measurementList = new MeasurementList;
    addAtool(measurementList);

    // connections ...

    connect(atoolActions,
            SIGNAL(triggered(QAction*)),
            SLOT(onAtoolActionTriggered(QAction*)));

    connect(Core::instance()->getSignalManager(),SIGNAL(processingStateChanged(bool)),
            SLOT(handleSignalDataChanged(bool)));

    connect(Core::instance()->getSignalManager(),SIGNAL(importStateChanged(bool)),
            SLOT(handleSignalDataChanged(bool)));

    connect(treeView,
            SIGNAL(checkedItemsChanged(QList<QTreeWidgetItem*>)),
            SLOT(onTreeViewSelectionChanged(QList<QTreeWidgetItem*>)));

     connect(treeView->treeWidget(),
             SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
             SLOT(onTreeViewCurrentItemChanged()));

     //connect(cbSigtype,SIGNAL(currentIndexChanged(int)),
     //        this,SLOT(onSignalTypeSelectionChanged(int)));

     connect(Core::instance()->guiSettings, SIGNAL(settingsChanged()), SLOT(onGUISettingsChanged()));

     // select default (plot) tool (first)
     //atoolActions->actions().at(2)->trigger();

     connect(verticalSplitterLeft, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));

     connect(horizontalSplitter, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
}


/**
 * @brief AnalysisTools::~AnalysisTools Destructor
 */
AnalysisTools::~AnalysisTools()
{
    checkboxes->clear();
    delete checkboxes;
}


/**
 * @brief AnalysisTools::addAtool private helpber method, which
 * creates all necessary connections etc.
 * @param atool AToolBase pointer
 */
void AnalysisTools::addAtool(AToolBase *atool)
{
    connect(this, SIGNAL(currentRunChanged(int)),
            atool,SLOT(onCurrentRunChanged(int)));

    connect(this, SIGNAL(selectedRunsChanged(QList<int>&)),
            atool,SLOT(onSelectedRunsChanged(QList<int>&)));

    connect(this, SIGNAL(selectedStypeChanged(Signal::SType)),
            atool,SLOT(onSelectedStypeChanged(Signal::SType)));

    connect(this, SIGNAL(selectedChannelIdsChanged(QList<int>&)),
            atool,SLOT(onSelectedChannelsChanged(QList<int>&)));

    connect(atool, SIGNAL(signalTypeChanged(Signal::SType)), SLOT(changeSignalType(Signal::SType)));

    connect(atool, SIGNAL(channelIDsChanged(QList<int>)), SLOT(changeChannelIDs(QList<int>)));

    aToolList.append( atool );
    atoolstack->addWidget(atool);

    QAction* toolbarAction = new QAction(atool->icon(), atool->title(),this);
    toolbarAction->setCheckable(true);
    atoolActions->addAction(toolbarAction);

    rtbView->addViewAction(toolbarAction, 0, atoolActions->actions().size(), 1, 1);
}


// --------------------------
// HANDLE USER INTERACTION
// --------------------------

/**
 * @brief AnalysisTools::onAtoolActionTriggered This slot is executed
 * when the user changed the current AnalysisTool (within the toolbar)
 * @param action QAction tiggered
 */
void AnalysisTools::onAtoolActionTriggered(QAction *action)
{
    dynamic_cast<AToolBase*>(atoolstack->currentWidget())->deactivateTool();

    int idx = atoolActions->actions().indexOf(action);
    atoolstack->setCurrentIndex(idx);

    //m_internalToolbar->clear();
    //m_internalToolbar->addActions(aToolList[idx]->toolbarActions());

    dynamic_cast<AToolBase*>(atoolstack->currentWidget())->activateTool();

    labelToolIcon->setPixmap(dynamic_cast<AToolBase*>(atoolstack->currentWidget())->icon().pixmap(32,32));
    labelToolName->setText(dynamic_cast<AToolBase*>(atoolstack->currentWidget())->title());

    QFont lbfont;
    lbfont.setBold(true);
    labelToolName->setFont(lbfont);

    setObjectName(atoolstack->currentWidget()->objectName());

    Core::instance()->guiSettings->setValue("AnalysisTools", "LastTool", idx);
}


/**
 * @brief AnalysisTools::onTreeViewSelectionChanged This slot is executed
 * when the user changed the MRun selection using the DataItemTreeView
 * @param selection List of selected Treewidgetitems
 */
void AnalysisTools::onTreeViewSelectionChanged(QList<QTreeWidgetItem *> selection)
{
    QList<int> selectedRunIDs;
    for(int i = 0; i < selection.size();i++)
    {
        int userType = selection.at(i)->data(0,Qt::UserRole).toInt();

        if(userType != 2) // continue only with mrun treeitems
            continue;

        selectedRunIDs << selection.at(i)->data(0,Qt::UserRole+1).toInt();
    }

    // notify AToolBase instances
    emit selectedRunsChanged(selectedRunIDs);
}


/**
 * @brief AnalysisTools::onTreeViewCurrentItemChanged This slot is
 * executed when the user changed the currently highlighted item within
 * the MRun-Treeview (eg. by moving the cursor using the arrow keys).
 */
void AnalysisTools::onTreeViewCurrentItemChanged()
{
    // show details of highlighted item (if run!)
    QTreeWidgetItem* item = treeView->treeWidget()->currentItem();
    int userData = item->data(0,Qt::UserRole).toInt();
    int data_id = item->data(0,Qt::UserRole+1).toInt();

    currentRun = 0;
    int run_id = -1;
    if(userData == 2)
    {
        currentRun = Core::instance()->dataModel()->mrun(data_id);
        run_id = currentRun->id();
    }

    runDetails->setRun(currentRun);

    // notify AToolBase instances
    emit currentRunChanged(run_id);
}


void AnalysisTools::onSignalTypeSelectionChanged(int idx)
{
    switch(idx)
    {
        case 0:
            m_selectedSignalType = Signal::RAW;     break;
        case 1:
            m_selectedSignalType = Signal::ABS;     break;
        case 2:
            m_selectedSignalType = Signal::TEMPERATURE;
    }
    updateChannelCheckboxes();

    // notify AToolBase instances
    emit selectedStypeChanged(m_selectedSignalType);
}


void AnalysisTools::onChannelSelectionChanged()
{
    m_selectedChannelIds.clear();

    // save checked channel-IDs to channelID-list
    for(int i = 0; i < checkboxes->size(); i++)
        if(checkboxes->at(i)->isChecked())
            m_selectedChannelIds.append(checkboxidx_to_chid[i]);

    // notify AToolBase instances
    emit selectedChannelIdsChanged(m_selectedChannelIds);
}


// ----------------------
// HANDLE PROGRAM STATE
// ----------------------

/**
 * @brief AnalysisTools::handleSignalDataChanged This slot
 * is execueted when the Signalprocessing has finished.
 * This method informs AToolBase Children to update/recalculate their
 * analysis results/visualizations
 * @param state
 */
void AnalysisTools::handleSignalDataChanged(bool state)
{
    if(state)
    {
        setCursor(Qt::WaitCursor);
    }
    else
    {
        /*updateChannelCheckboxes();

        for(int i = 0; i< aToolList.size(); i++)
        {
            aToolList.at(i)->handleSignalDataChanged();
        }*/

        dynamic_cast<AToolBase*>(atoolstack->currentWidget())->handleSignalDataChanged();

        if(firstRun)
        {
            firstRun = false;

            QList<MRun*> runList = Core::instance()->dataModel()->mrunList();
            QSet<int> channelIDs;

            for(int i = 0; i < runList.size(); i++)
                channelIDs = channelIDs + runList[i]->channelIDs(Signal::RAW).toSet();

            m_lastChannelIDsRawAbs = channelIDs.toList();

            emit selectedChannelIdsChanged(m_lastChannelIDsRawAbs);
        }

        setCursor(Qt::ArrowCursor);
    }
}


// --------------------
// PRIVATE HELPERS
// --------------------

void AnalysisTools::updateChannelCheckboxes()
{
    QList<MRun*> runs = Core::instance()->dataModel()->mrunList();

    int noCh = 0;
    QSet<int> chids;

    for(int i = 0; i < runs.size(); i++)
    {
        chids = chids + runs[i]->channelIDs(m_selectedSignalType).toSet();

        int ccount = runs[i]->getNoChannels(m_selectedSignalType);
        if(ccount > noCh)
            noCh =ccount;
    }
    if(runs.isEmpty())
    {
        //noCh = Core::instance()->modelingSettings->liiSettings().channels.size();
        noCh = 0;
        for(int i = 0; i < noCh; i++)
            chids.insert(i+1);
    }

    // turn channel set to list and sort it (ascending)
    QList<int> chidlist = chids.toList();
    std::sort(chidlist.begin(), chidlist.end());

    // update only if available channel ids has been changed!
    if( checkboxidx_to_chid != chidlist)
    {
        // remove checkboxes
        for(int i = 0; i < checkboxes->size(); i++)
        {
            m_checkBoxLayout->removeWidget(checkboxes->at(i));
            delete checkboxes->at(i);
        }

        checkboxes->clear();
        checkboxidx_to_chid.clear();
        m_selectedChannelIds.clear();

        // add new checkboxes

        for(int i = 0; i < chidlist.size(); i++)
        {

            QString text;
            text.sprintf("Ch %d",chidlist[i]);
            QCheckBox* checkbox = new QCheckBox(text,this);

            // setup some defaults!
            /*if(i == 0)
            {
                checkbox->setChecked(true);
                m_selectedChannelIds.append(chidlist[0]);
            }*/

            connect(checkbox,SIGNAL(released()),this,SLOT(onChannelSelectionChanged()));

            checkboxes->append(checkbox);
            checkboxidx_to_chid.append(chidlist[i]);
            m_checkBoxLayout->insertWidget(1+i,checkbox);
        }

        // update the number of legend columns
        //plotter->setMaxLegendColumns(channeIDs.size());

    }
    // update signal data for default selection
    onChannelSelectionChanged();
}


void AnalysisTools::onGUISettingsChanged()
{
    int idx = Core::instance()->guiSettings->value("AnalysisTools", "LastTool", 0).toUInt();

    if(idx >= atoolActions->actions().size())
        idx = 0;

    atoolActions->actions().at(idx)->trigger();

    QVariant position;
    if(Core::instance()->guiSettings->getSplitterPosition(_identifierSplitterH, position))
        horizontalSplitter->restoreState(position.toByteArray());

    if(Core::instance()->guiSettings->getSplitterPosition(_identifierSplitterV, position))
        verticalSplitterLeft->restoreState(position.toByteArray());
}


/**
 * @brief AnalysisTools::onCalcToolboxRecalcExecuted when the CalculationToolbox emits
 * a recalc-signal. Initiates the signal processing.
 * @param typeList List containing the signal types which should be processed.
 */
void AnalysisTools::onCalcToolboxRecalc(QList<Signal::SType> typeList)
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


void AnalysisTools::onSplitterMoved()
{
    if(QObject::sender() == horizontalSplitter)
        Core::instance()->guiSettings->setSplitterPosition(_identifierSplitterH, horizontalSplitter->saveState());
    else if(QObject::sender() == verticalSplitterLeft)
        Core::instance()->guiSettings->setSplitterPosition(_identifierSplitterV, verticalSplitterLeft->saveState());
}


void AnalysisTools::changeSignalType(Signal::SType signalType)
{
    m_selectedSignalType = signalType;

    emit selectedStypeChanged(signalType);

    if(signalType == Signal::RAW || signalType == Signal::ABS)
        emit selectedChannelIdsChanged(m_lastChannelIDsRawAbs);
    else
        emit selectedChannelIdsChanged(m_lastChannelIDsTemp);
}


void AnalysisTools::changeChannelIDs(QList<int> channelIDs)
{
    m_selectedChannelIds = channelIDs;

    if(m_selectedSignalType == Signal::RAW || m_selectedSignalType == Signal::ABS)
        m_lastChannelIDsRawAbs = channelIDs;
    else
        m_lastChannelIDsTemp = channelIDs;

    emit selectedChannelIdsChanged(channelIDs);
}

