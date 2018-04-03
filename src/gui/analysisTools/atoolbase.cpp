#include "atoolbase.h"

#include "../../core.h"

/**
 * @brief AToolBase::AToolBase constructor
 * @param parent
 */
AToolBase::AToolBase(QWidget *parent) : QWidget(parent)
{
    m_title = "none";
    m_iconLocation = "";
    m_currentRunId = -1;
    m_selectedStype = Signal::RAW;
    m_toolActive = false;
}

// ----------------
// GETTERS
// ----------------

QIcon AToolBase::icon()
{
    return QIcon(m_iconLocation);
}

/**
 * @brief AToolBase::getTitle get title of Analysis tool
 * @return title
 */
QString AToolBase::title()
{
    return m_title;
}


/**
 * @brief AToolBase::currentRun gets MRun which is currently highlighted
 * and shown within the AnalysisTool's RunDetailsView
 * @return MRun* or NULL if no run is highlighted
 */
MRun* AToolBase::currentRun()
{
    return Core::instance()->dataModel()->mrun(m_currentRunId);
}

/**
 * @brief AToolBase::selectedRuns gets currently selected MRuns
 * @return List of MRun*
 */
QList<MRun*> AToolBase::selectedRuns()
{
    QList<MRun*> runs;
    for(int i = 0; i < m_selectedRunIds.size(); i++)
    {
        MRun* run = Core::instance()->dataModel()->mrun(m_selectedRunIds[i]);
        if(!run)
            continue;
        runs << run;
    }
    return runs;
}

/**
 * @brief AToolBase::selectedSignalType gets currently selected
 * signal type
 * @return
 */
Signal::SType AToolBase::selectedSignalType()
{
    return m_selectedStype;
}

/**
 * @brief AToolBase::selectedChannelIds gets list of
 * currently selected channel ids
 * @return
 */
QList<int> AToolBase::selectedChannelIds()
{
    return m_selectedChannelIds;
}


/**
 * @brief AToolBase::selectedRunIds returns list of MRun-IDs of selected
 * MRuns
 * @return
 */
QList<int> AToolBase::selectedRunIds()
{
    return m_selectedRunIds;
}

// -------------------------
// HANDLE SELECTION UPDATES
// -------------------------

/**
 * @brief AToolBase::onCurrentRunChanged This slot is executed
 * when the user has changed the currently highlighted run
 * within the Treeview. See: AnalysisTools::onTreeViewCurrentItemChanged()
 * @param run_id
 */
void AToolBase::onCurrentRunChanged(int run_id)
{
    m_currentRunId = run_id;
    if(m_toolActive)
        handleCurrentRunChanged(currentRun());
}

/**
 * @brief AToolBase::onSelectedRunsChanged This slot is executed
 * when the user has changed the current MRun Selection within
 * the AnalysisTool's Treeview. Also see: AnalysisTools::onTreeViewSelectionChanged()
 * @param run_ids
 */
void AToolBase::onSelectedRunsChanged(QList<int> &run_ids)
{
    m_selectedRunIds = run_ids;
    QList<MRun*> mrunList = selectedRuns();
    if(m_toolActive)
        handleSelectedRunsChanged(mrunList);
}

void AToolBase::onSelectedStypeChanged(Signal::SType stype)
{
    m_selectedStype = stype;
    if(m_toolActive)
        handleSelectedStypeChanged(stype);
}

void AToolBase::onSelectedChannelsChanged(QList<int> &ch_ids)
{
    m_selectedChannelIds = ch_ids;
    if(m_toolActive)
        handleSelectedChannelsChanged(ch_ids);
}


void  AToolBase::activateTool()
{
    m_toolActive = true;
    onToolActivation();
}


void  AToolBase::deactivateTool()
{
    m_toolActive = false;
}


