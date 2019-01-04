#include "signalplottool.h"

#include <qwt_plot.h>

#include "../../general/LIISimException.h"

#include "../../signal/mrun.h"
#include "../../signal/signalmanager.h"
#include "../../core.h"
#include <QDebug>


/**
 * @brief SignalPlotTool::SignalPlotTool constructor
 * @param parent
 */
SignalPlotTool::SignalPlotTool(QWidget *parent) : AToolBase(parent)
{
    setObjectName("AT_SPT");
    m_title = "Plotter";
    m_iconLocation = Core::rootDir + "resources/icons/chart_curve.png";

    mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    horizontalSplitter = new QSplitter(Qt::Horizontal);
    horizontalSplitter->setHandleWidth(20);
    mainLayout->addWidget(horizontalSplitter);

    plotter = new SignalPlotWidgetQwt(this);
    plotter->setObjectName("AT_SPT_PLOT");
    plotter->setZoomMode(SignalPlotWidgetQwt::PLOT_PAN);
    plotter->setShowChannelCheckBoxes(false);
    plotter->setActionShowChannelCBsVisible(false);
    plotter->setDataTableToolName(m_title);

    comboboxSignalType = new LabeledComboBox("Signal type:");
    comboboxSignalType->setMaximumWidth(150);
    comboboxSignalType->addStringItem("raw");
    comboboxSignalType->addStringItem("absolute");
    comboboxSignalType->addStringItem("temperature");

    plotterToolbar = new QToolBar;

    channelCheckboxesLayout = new QHBoxLayout;
    channelCheckboxesLayout->setMargin(0);
    channelCheckboxesLayout->addWidget(comboboxSignalType);

    QHBoxLayout *plotterSettingsLayout = new QHBoxLayout;
    plotterSettingsLayout->setMargin(0);
    plotterSettingsLayout->addLayout(channelCheckboxesLayout);
    plotterSettingsLayout->addWidget(plotterToolbar);
    plotterSettingsLayout->setAlignment(channelCheckboxesLayout, Qt::AlignLeft);
    plotterSettingsLayout->setAlignment(plotterToolbar, Qt::AlignRight);

    plotterWidget = new QWidget(this);
    QVBoxLayout *plotterLayout = new QVBoxLayout;
    plotterWidget->setLayout(plotterLayout);
    plotterLayout->addLayout(plotterSettingsLayout);
    plotterLayout->addWidget(plotter);

    horizontalSplitter->addWidget(plotterWidget);

    plotter->toolActionDataCursor()->triggered(true);

    plotterToolbar->addActions(toolbarActions());

    connect(Core::instance()->guiSettings, SIGNAL(settingsChanged()), SLOT(onGuiSettingsChanged()));
    connect(plotter, SIGNAL(plotTypeChanged(BasePlotWidgetQwt::PlotType)), SLOT(onPlotTypeChanged(BasePlotWidgetQwt::PlotType)));

    connect(comboboxSignalType, SIGNAL(currentIndexChanged(int)), SLOT(onComboboxSignalTypeIndexChanged(int)));
}


SignalPlotTool::~SignalPlotTool()
{
    for(int i = 0; i < m_plotItems.size(); i++)
    {
        SPTplotItem* pi = m_plotItems.take(0);
        if(!pi)
            continue;
        delete pi;
    }
}


// ------------------------------------
// GETTERS
// ------------------------------------

QList<QAction*> SignalPlotTool::toolbarActions()
{
    return plotter->toolActions();
}


// ------------------------------------
// HANDLE ANALYSISTOOL USER INTERACTION
// ------------------------------------

/**
 * @brief SignalPlotTool::handleSignalDataChanged implements
 * AToolBase::handleSignalDataChanged().
 * @details update the data vectors of all items in plot.
 */
void SignalPlotTool::handleSignalDataChanged()
{
    QList<int> runIds = selectedRunIds();

    for(int i = 0; i < runIds.size(); i++)
    {
        int r_id = runIds.at(i);
        SPTplotItem* pi = m_plotItems.value(r_id, 0);
        if(!pi)
            continue;

        pi->updateCurveData();
    }
    plotter->handleCurveDataChanged();

    updateChannelCheckboxes();

    //updateTempChannelTooltips();

    // emit signal for child classes
    emit mRunSelectionUpdated();
}


void SignalPlotTool::handleCurrentRunChanged(MRun *run)
{   
}


void SignalPlotTool::handleSelectedRunsChanged(const QList<MRun *> &runs)
{
    updateMRunSelection();

    updateTempChannelTooltips();

    if(runs.size() == 1)
        plotter->setDataTableRunName(runs.at(0)->name);
    else if(runs.size() > 1)
        plotter->setDataTableRunName("Multiple MRuns selected");

    // emit signal for child classes
    emit mRunSelectionUpdated();
}


void SignalPlotTool::handleSelectedStypeChanged(Signal::SType stype)
{    
    plotter->setSignalType(stype);

    updateChannelCheckboxes();

    QList<int> runIds = selectedRunIds();
    for(int i = 0; i < runIds.size();i++)
    {
        int r_id = runIds.at(i);
        SPTplotItem* pi = m_plotItems.value(r_id,0);
        if(!pi)
            continue;

        pi->setSelectedSignalType(stype);
        pi->updateCurveData();
    }
    plotter->handleCurveDataChanged(false);
    plotter->setZoomMode(SignalPlotWidgetQwt::ZOOM_RESET);

    switch(stype)
    {
        case Signal::RAW: comboboxSignalType->setCurrentIndex(0); break;
        case Signal::ABS: comboboxSignalType->setCurrentIndex(1); break;
        case Signal::TEMPERATURE: comboboxSignalType->setCurrentIndex(2); break;
    }

    // emit signal for child classes
    emit mRunSelectionUpdated();
}


void SignalPlotTool::handleSelectedChannelsChanged(const QList<int> &ch_ids)
{
    QList<int> runIds = selectedRunIds();
    for(int i = 0; i < runIds.size();i++)
    {
        int r_id = runIds.at(i);
        SPTplotItem* pi = m_plotItems.value(r_id,0);
        if(!pi)
            continue;

        pi->setSelectedChannelIDs(ch_ids);
        pi->updateCurveData();
    }

    // update the number of legend columns
    plotter->setMaxLegendColumns(ch_ids.size());

    plotter->handleCurveDataChanged();

    for(auto it = channelToCheckbox.begin(); it != channelToCheckbox.end(); ++it)
    {
        it.value()->blockSignals(true);
        it.value()->setChecked(false);
        it.value()->blockSignals(false);
    }

    for(int i = 0; i < ch_ids.size(); i++)
    {
        if(channelToCheckbox.contains(ch_ids.at(i)))
        {
            channelToCheckbox.value(ch_ids.at(i))->blockSignals(true);
            channelToCheckbox.value(ch_ids.at(i))->setChecked(true);
            channelToCheckbox.value(ch_ids.at(i))->blockSignals(false);
        }
    }

    // emit signal for child classes
    emit mRunSelectionUpdated();
}


void SignalPlotTool::onToolActivation()
{
    updateMRunSelection();

    handleSelectedStypeChanged(selectedSignalType());
    handleSelectedChannelsChanged(selectedChannelIds());
}


// --------------------------
// PRIVATE GUI UPDATE HELPERS
// --------------------------


/**
 * @brief SignalPlotTool::updateMRunSelection overwrites AToolBase::updateMRunSelection().
 * @details Create and delete plot items based on new MRun selection.
 * @param m_selectedRunIDs list of selected MRun IDs
 */
void SignalPlotTool::updateMRunSelection()
{
    QList<int> runIds = selectedRunIds();
    QList<int> channeIDs = selectedChannelIds();
    Signal::SType stype = selectedSignalType();

    QList<int> curPlots = m_plotItems.keys();

    // delete plot items which are not selected anymore
    for(int i = 0; i < curPlots.size(); i++)
    {
        int k = curPlots.at(i);
        if(!runIds.contains(k))
        {
            SPTplotItem* pi = m_plotItems.take(k);
            if(!pi)
                continue;
            delete pi;
        }
    }

    // create new plot items
    for(int i = 0; i < runIds.size(); i++)
    {
        int m_id = runIds.at(i);

        if(m_plotItems.contains(m_id))
            continue;

        MRun* run = Core::instance()->dataModel()->mrun(m_id);

        if(!run)
            continue;

        SPTplotItem* pi = new SPTplotItem(run, plotter);

        pi->setSelectedChannelIDs(channeIDs);
        pi->setSelectedSignalType(stype);
        pi->setSelectedSigIndex(0);
        pi->updateCurveData();
        m_plotItems.insert(m_id,pi);
    }

    // tell the plot that we have changed its curves!
    plotter->handleCurveDataChanged();

    //updateTempChannelTooltips();

    if(selectedRunIds().isEmpty())
    {
        plotter->setPlotLabelText("No run selected. Please select a run from the treeview to display signal data.");
        plotter->setDataTableRunName("No MRun selected");
        plotter->plotTextLabelVisible(true);
    }
    else
        plotter->plotTextLabelVisible(false);
}


void SignalPlotTool::onPlotTypeChanged(BasePlotWidgetQwt::PlotType type)
{
    switch(type)
    {
    case BasePlotWidgetQwt::LINE_CROSSES: Core::instance()->guiSettings->setValue("analysistools", "signalplotplottype", 0); break;
    case BasePlotWidgetQwt::LINE:         Core::instance()->guiSettings->setValue("analysistools", "signalplotplottype", 1); break;
    case BasePlotWidgetQwt::DOTS_SMALL:   Core::instance()->guiSettings->setValue("analysistools", "signalplotplottype", 2); break;
    case BasePlotWidgetQwt::DOTS_MEDIUM:  Core::instance()->guiSettings->setValue("analysistools", "signalplotplottype", 3); break;
    case BasePlotWidgetQwt::DOTS_LARGE:   Core::instance()->guiSettings->setValue("analysistools", "signalplotplottype", 4); break;
    }
}


void SignalPlotTool::onGuiSettingsChanged()
{
    if(Core::instance()->guiSettings->hasEntry("analysistools", "signalplotplottype"))
    {
        switch(Core::instance()->guiSettings->value("analysistools", "signalplotplottype", 0).toUInt())
        {
        case 0: plotter->setPlotType(BasePlotWidgetQwt::LINE_CROSSES); break;
        case 1: plotter->setPlotType(BasePlotWidgetQwt::LINE); break;
        case 2: plotter->setPlotType(BasePlotWidgetQwt::DOTS_SMALL); break;
        case 3: plotter->setPlotType(BasePlotWidgetQwt::DOTS_MEDIUM); break;
        case 4: plotter->setPlotType(BasePlotWidgetQwt::DOTS_LARGE); break;
        }
    }
}


void SignalPlotTool::onComboboxSignalTypeIndexChanged(int index)
{
    switch(index)
    {
    case 0: emit signalTypeChanged(Signal::RAW); break;
    case 1: emit signalTypeChanged(Signal::ABS); break;
    case 2: emit signalTypeChanged(Signal::TEMPERATURE); break;
    }
}


void SignalPlotTool::onCheckboxChannelIDStateChanged(int state)
{
    QList<int> channel;
    for(auto it = channelToCheckbox.begin(); it != channelToCheckbox.end(); ++it)
    {
        if(it.value()->isChecked())
            channel.push_back(it.key());
    }
    emit channelIDsChanged(channel);
}


void SignalPlotTool::updateChannelCheckboxes()
{
    QList<MRun*> runs = Core::instance()->dataModel()->mrunList();

    int channelCount = 0;
    QSet<int> channelIDSet;

    for(int i = 0; i < runs.size(); i++)
    {
        channelIDSet = channelIDSet + runs[i]->channelIDs(selectedSignalType()).toSet();
        if(runs[i]->getNoChannels(selectedSignalType()) > channelCount)
            channelCount = runs[i]->getNoChannels(selectedSignalType());
    }

    QList<int> channelIDList = channelIDSet.toList();
    std::sort(channelIDList.begin(), channelIDList.end());

    // do not update if channel count is the same as before
    if(channelIDList != channelIDs)
    {
        for(int i = 0; i < channelCheckboxes.size(); i++)
        {
            channelCheckboxesLayout->removeWidget(channelCheckboxes.at(i));
            delete channelCheckboxes.at(i);
        }

        channelCheckboxes.clear();
        channelToCheckbox.clear();

        for(int i = 0; i < channelIDList.size(); i++)
        {
            QCheckBox *checkbox;
            if(selectedSignalType() == Signal::RAW || selectedSignalType() == Signal::ABS)
                checkbox = new QCheckBox(QString("Ch %0").arg(channelIDList.at(i)), this);
            else
                checkbox = new QCheckBox(QString("T%0 ").arg(channelIDList.at(i)), this);

            channelCheckboxes.push_back(checkbox);
            channelCheckboxesLayout->addWidget(checkbox);

            channelToCheckbox.insert(channelIDList.at(i), checkbox);

            connect(checkbox, SIGNAL(stateChanged(int)), SLOT(onCheckboxChannelIDStateChanged(int)));
        }

        channelIDs = channelIDList;
    }

    updateTempChannelTooltips();
}


void SignalPlotTool::updateTempChannelTooltips()
{
    if(selectedSignalType() == Signal::TEMPERATURE)
    {
        QList<MRun*> runs = Core::instance()->dataModel()->mrunList();

        for(int i = 0; i < runs.size(); i++)
        {
            QList<TempCalcMetadata> metadata = runs.at(i)->tempMetadata.values();

            for(int j = 0; j < metadata.size(); j++)
            {
                QString tooltip;
                QString signalSource;

                if(metadata.at(j).signalSource == Signal::RAW)
                    signalSource = "Raw";
                else if(metadata.at(j).signalSource == Signal::ABS)
                    signalSource = "Absolute";

                tooltip.append(QString("Method: %0\nSignal Source: %1\nMaterial: %2\nE(m) source: %3")
                               .arg(metadata.at(j).method)
                               .arg(signalSource)
                               .arg(metadata.at(j).material)
                               .arg(metadata.at(j).sourceEm));

                if(metadata.at(j).method == "Two-Color" || metadata.at(j).method == "Ratio A/B")
                {
                    tooltip.append(QString("\nChannels: %0 / %1").arg(metadata.at(j).channelID1).arg(metadata.at(j).channelID2));
                }
                else if(metadata.at(j).method == "Spectrum")
                {
                    tooltip.append(QString("\nSelected Channels: "));
                    for(int channel = 0; channel < metadata.at(j).activeChannels.size(); channel++)
                    {
                        if(metadata.at(j).activeChannels.at(channel))
                            tooltip.append(QString("%0 ").arg(channel+1));
                    }

                    tooltip.append(QString("\nIterations: %0\nStart Temperature: %1\nStart Scaling Factor: %2")
                                   .arg(metadata.at(j).iterations)
                                   .arg(metadata.at(j).startTemperature)
                                   .arg(metadata.at(j).startC));
                    tooltip.append("\nBandpass Integration: ");
                    if(metadata.at(j).bandpass)
                        tooltip.append("Active");
                    else
                        tooltip.append("Inactive");
                    tooltip.append("\nWeighting: ");
                    if(metadata.at(j).weighting)
                        tooltip.append("Active");
                    else
                        tooltip.append("Inactive");
                }
                QCheckBox *checkbox = channelToCheckbox.value(metadata.at(j).tempChannelID);
                if(checkbox)
                    checkbox->setToolTip(tooltip);
                //channelCheckboxes.at(metadata.at(j).tempChannelID-1)->setToolTip(tooltip);
            }
        }
    }
}






