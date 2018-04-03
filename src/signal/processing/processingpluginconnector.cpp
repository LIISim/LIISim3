#include "processingpluginconnector.h"

#include <QDebug>

#include "processingplugin.h"
#include "processingchain.h"
#include "pluginfactory.h"
#include "../mrungroup.h"
#include "../../core.h"
#include "plugins/temperaturecalculator.h"

int ProcessingPluginConnector::m_linkID_count = 1;


/**
 * @brief ProcessingPluginConnector::ProcessingPluginConnector Constructor,
 * initializes ProcessingPluginConnector and optionally creates all necessary
 * Plugin Instances (only if a parent ProcessingChain is provided).
 * @param pluginName name of observed ProcessingPlugin
 * @param linkType type of linking (all mruns, per group, single mrun)
 * @param parentChain If a parent ProcessingChain is provided, Plugin instances
 * will be created automatically and added to several ProcessingChains (dependent on linkType). Default value = NULL)
 * @param parent QObject parent, default value = NULL
 */
ProcessingPluginConnector::ProcessingPluginConnector(QString pluginName, int linkType, ProcessingChain *parentChain,bool doInit, QObject *parent) : QObject(parent)
{
    m_linkID = m_linkID_count++;
    m_linkState = linkType;
    m_pname = pluginName;
    m_dataMuteFlag = true;
    m_pchainMuteFlag = true;
    m_active = true;
    m_stype = parentChain->getSignalType();
    commonTCID = -1;

    // if initialization should be done, add a instance of the observed plugin to all runs
    if(doInit)
    {
        Signal::SType stype = parentChain->getSignalType();
        QList<MRun*> runs = Core::instance()->dataModel()->mrunList();

        for(int i = 0; i < runs.size(); i++)
        {
            ProcessingChain* pchain = runs[i]->getProcessingChain(stype);
            ProcessingPlugin* plug = PluginFactory::createInstance(m_pname,pchain);
            pchain->addPlug(plug);
            add(plug);
        }
    }

    m_dataMuteFlag = false;
    m_pchainMuteFlag = false;
}


ProcessingPluginConnector::ProcessingPluginConnector(ProcessingPlugin *p)
{
    m_linkID = m_linkID_count++;
    m_linkState = p->linkState();
    m_pname = p->getName();
    m_dataMuteFlag = true;
    m_pchainMuteFlag = true;
    m_active = true;
    m_stype = p->getStype();
    commonTCID = -1;

    add(p);
    createMissingClones(p);

    m_dataMuteFlag = false;
    m_pchainMuteFlag = false;
}



/**
 * @brief ProcessingPluginConnector::~ProcessingPluginConnector
 * Destructor.
 */
ProcessingPluginConnector::~ProcessingPluginConnector()
{
    m_plugins.clear();
}


/**
 * @brief ProcessingPluginConnector::add Adds a ProcessingPlugin to the
 * list of observed plugins. All plugins in this list must be of the same type.
 * The connector Object will subscribe to some signals of the observed
 * ProcessingPlugin and its parent ProcessingChain. This enables
 * the connector object to syncronize the ProcessingPlugin's parameters as
 * well as the ProcessingChain's topology.
 * @param p ProcessingPlugin
 * @return true on success else false
 */
bool ProcessingPluginConnector::add(ProcessingPlugin *p)
{
    if(!p)return false;

    //if(m_plugins.isEmpty())
    //    m_pname = p->getName();

    // check if plugin is already in list
    if(m_plugins.contains(p))
    {
        qDebug() << "ProcessingPluginConnector::add: plugin with ID: "
                 << p->id() << " is already observed!";
        return false;
    }

    // check if plugin name is valid
    if(m_pname != p->getName())
    {
        qDebug() << "ProcessingPluginConnector::add: invalid plugin-name '"
                 << p->getName() << "'! Can only add plugins of type '"
                 << m_pname << "'!";
        return false;
    }

    // for TemperatureCalculators: assign temperature channel ids
    if(m_pname == TemperatureCalculator::pluginName)
    {
        TemperatureCalculator* tc =dynamic_cast<TemperatureCalculator*>(p);
        if(commonTCID == -1)
        {
            if(tc->temperatureChannelID() == -1)
                tc->setTemperatureChannelID(TemperatureCalculator::generateTemperatureChannelID());
            commonTCID = tc->temperatureChannelID();
        }
        else
            tc->setTemperatureChannelID(commonTCID);
    }

    m_plugins.append(p);

    p->m_ppc = this;

  //  p->setLinkState(m_linkType);

    // observe plugin signals
    connect(p, SIGNAL(dataChanged(int,QVariant)),
            SLOT(onPlugDataChanged(int,QVariant)));

    connect(p, SIGNAL(destroyed(int)),
            SLOT(onPlugDestroyed(int)));

    connect(p->processingChain(),SIGNAL(childRemoved(DataItem*)),
            SLOT(onPlugRemovedFromChain(DataItem*)));

    connect(p->processingChain(),SIGNAL(childInserted(DataItem*,int)),
            SLOT(onPlugInsertedToChain(DataItem*,int)));

    return true;
}


// ---------------------------------------
// SETTERS FOR PROCESSINGPLUGIN PROPERTIES
// ---------------------------------------


/**
 * @brief ProcessingPluginConnector::setLinkState
 * changes linkState of all observed plugins
 * @param linkState
 */
void ProcessingPluginConnector::setPluginsLinkState(int linkState)
{
    bool oldFlag = m_dataMuteFlag;
    m_dataMuteFlag = true;
    for(int i = 0; i < m_plugins.size(); i++)
        m_plugins[i]->setLinkState(linkState);
    m_dataMuteFlag = oldFlag;
}


/**
 * @brief ProcessingPluginConnector::setPluginsActive
 * changes activated state of all observed plugins
 * @param active
 */
void ProcessingPluginConnector::setPluginsActive(bool active)
{
    m_dataMuteFlag = true;
    for(int i = 0; i < m_plugins.size(); i++)
        m_plugins[i]->setActivated(active);
    m_dataMuteFlag = false;
}


/**
 * @brief ProcessingPluginConnector::setPluginsVisible
 * changes plot visibility of all observed plugins
 * @param visible
 */
void ProcessingPluginConnector::setPluginsVisible(bool visible)
{
    m_dataMuteFlag = true;
    for(int i = 0; i < m_plugins.size(); i++)
        m_plugins[i]->setPlotVisibility(visible);
    m_dataMuteFlag = false;
}


/**
 * @brief ProcessingPluginConnector::setPluginsStepBufferFlag
 * changes step buffer flag of all observed plugins
 * @param sbFlag
 */
void ProcessingPluginConnector::setPluginsStepBufferFlag(bool sbFlag)
{
    m_dataMuteFlag = true;
    for(int i = 0; i < m_plugins.size(); i++)
        m_plugins[i]->setStepBufferEnabled(sbFlag);
    m_dataMuteFlag = false;
}


/**
 * @brief ProcessingPluginConnector::setPluginsParams
 * changes parameters of all observed plugins
 * @param params
 */
void ProcessingPluginConnector::setPluginsParams(const ProcessingPluginInputList &params)
{
    if(m_plugins.size() == 0)
        return;

    m_dataMuteFlag = true;

    ProcessingPluginInputList l0 = m_plugins[0]->getInputs();
    for(int j=0; j < params.size(); j++)
    {
        l0.setValue(params.at(j).identifier, params.at(j).value);
    }

    for(int i = 0; i < m_plugins.size(); i++)
        try{
            m_plugins[i]->setParameters(l0);
        }
        catch(LIISimException e)
        {
            MESSAGE(e.what(), e.type());
        }

    m_dataMuteFlag = false;
}


/**
 * @brief ProcessingPluginConnector::setActive If this property
 * is set to true (default value) missing copies of ProcessingPlugins
 * are created automatically when needed. Switching this behavior
 * off could be useful during session import (also see IOxml).
 * @param state
 */
void ProcessingPluginConnector::setActive(bool state)
{
    m_active = state;
    m_dataMuteFlag = !state;
    m_pchainMuteFlag = !state;
    if(state)
    {

        QList<ProcessingPlugin*> initPlugins = m_plugins;
        for(int i = 0; i < initPlugins.size(); i++)
        {
            createMissingClones(m_plugins[i]);
        }

        if(m_pname == TemperatureCalculator::pluginName)
        {
            if(m_plugins.isEmpty())
                return;

            TemperatureCalculator* tc =dynamic_cast<TemperatureCalculator*>(m_plugins[0]);
            if(tc)
                commonTCID = tc->temperatureChannelID();
            if(commonTCID < 0)
                commonTCID = TemperatureCalculator::generateTemperatureChannelID();

            for(int i = 0; i < m_plugins.size();i++)
            {
                tc =dynamic_cast<TemperatureCalculator*>(m_plugins[i]);
                if(tc)
                    tc->setTemperatureChannelID(commonTCID);
            }
        }
    }
}


// -------------------------------
// PUBLIC HELPERS FOR MRUN OBJECTS
// -------------------------------


/**
 * @brief ProcessingPluginConnector::connectMRun Adds a ProcessingPlugin instance,
 * which is connected with this connection object, to the given MRun's processing chain.
 * to a given MRun. This slot is executed when
 * a MRun has been inserted to a group also see MRunGroup::insertChild()).
 * @param mrun
 * @param groupChainIndex index of this connector object in list of connector objects within group
 */
void ProcessingPluginConnector::connectMRun(MRun *mrun, int groupChainIndex)
{
    // get the run's processing chain create a new ProcessingPlugin instance
    ProcessingChain* pchain = mrun->getProcessingChain(m_stype);
    ProcessingPlugin* p = PluginFactory::createInstance(m_pname,pchain);

    // determine the correct index of the new ProcessingPlugin within the run's processing chain
    int curGidx = -1;
    int idx =0;
    for(idx = 0; idx < pchain->noPlugs(); ++idx)
    {
        if( pchain->getPlug(idx)->linkID() != -1 )
            curGidx += 1;
        if(curGidx == groupChainIndex)
            break;
    }

    if(idx < 0 || idx >= pchain->noPlugs())
        pchain->addPlug(p);
    else
        pchain->insertPlug(p,idx);

    // initialize the parameters of the new ProcessingPlugin

    ProcessingPluginInputList params;
    bool activated = true;
    bool plotvis   = true;
    // A-Plugin: get init params from any plugin if available
    if(m_linkState == 2 )
    {
        if(m_plugins.size()>0)
        {
            params = m_plugins[0]->getInputs();
            activated = m_plugins[0]->activated();
            plotvis = m_plugins[0]->plotVisibility();
        }
    }
    // G-Plugin: get init params from any plugin within same group
    else if(m_linkState == 1)
    {
        QList<ProcessingPlugin*> res = pluginsInSameGroup(p);
        if(res.size() > 0)
        {
            params = res[0]->getInputs();
            activated = res[0]->activated();
            plotvis = res[0]->plotVisibility();
        }
    }

    // set params
    if(!params.isEmpty())
        try{
            p->setParameters(params);
        }
        catch(LIISimException e)
        {
            MESSAGE(e.what(), e.type());
        }
    else
        p->setDirty(true);

    p->setActivated(activated);
    p->setPlotVisibility(plotvis);

    // add the new ProcessingPlugin to the list of observed Plugins
    add(p);
    p->setLinkState(m_linkState);
}

/**
 * @brief ProcessingPluginConnector::disconnectMRun Removes
 * a ProcessingPlugin instance (connected with this object connection)
 * from the given MRun's processing chain.
 * @param mrun
 */
void ProcessingPluginConnector::disconnectMRun(MRun *mrun)
{
    for(int i = 0; i < m_plugins.size(); i++)
    {
        if(m_plugins[i]->mrun == mrun)
        {
            m_plugins[i]->disconnect(this);
            m_plugins[i]->m_pchain->disconnect(this);
            m_plugins[i]->m_pchain->removePlug(m_plugins[i]->position());
            m_plugins.removeAt(i);
            i--;
        }
    }
    if(m_plugins.isEmpty())
        this->deleteLater();
}


// ---------------------------------------------------------------
// PRIVATE HANDLERS FOR ADD/DELETE OPERATIONS OF PROCESSING CHAINS
// ---------------------------------------------------------------

/**
 * @brief ProcessingPluginConnector::onPlugRemovedFromChain This slot is executed
 * when any of the observed ProcessingPlugins has been removed from its parent chain.
 * The Connector Object will also remove (and delete) all other observed PluginInstance
 * from its parent chain.
 * @param dataItem ProcessingPlugin as DataItem
 */
void ProcessingPluginConnector::onPlugRemovedFromChain(DataItem *dataItem)
{
    // get Plugin (fails if dataItem has been deleted)
    ProcessingPlugin* p = dynamic_cast<ProcessingPlugin*>(dataItem);

    if( m_pchainMuteFlag )
        return;

    if(!p)return;

    // do nothing if removed plugin is not observed
    if(!m_plugins.contains(p))
        return;

    // find sender chain (fails also if dataItem has been deleted)
    ProcessingChain* pchain = p->processingChain();
    if(!pchain)return;

    m_pchainMuteFlag = true;

    for(int i = 0; i < m_plugins.size(); i++)
    {
        ProcessingPlugin* curpp = m_plugins[i];
        if(curpp==p)
            continue;

        curpp->processingChain()->removePlug(curpp->position(), false );
    }
    m_pchainMuteFlag = false;
}



/**
 * @brief ProcessingPluginConnector::onPlugInsertedToChain This slot
 * is executed when a ProcessingPlugin has been added to a parent
 * ProcessingChain of any observed ProcessingPlugin.
 * This possibly requires to add other observed Plugins to its parent chains
 * at a specific position to provide a syncronized, global order of all
 * connector objects within processing chains.
 * @param dataItem ProcessingPlugin as DataItem
 * @param position position of ProcessingPlugin within chain
 */
void ProcessingPluginConnector::onPlugInsertedToChain(DataItem *dataItem, int position)
{
    // get Plugin (fails if dataItem has been deleted)
    ProcessingPlugin* p = dynamic_cast<ProcessingPlugin*>(dataItem);

    if( m_pchainMuteFlag )
        return;

    if(!p)return;

    // do nothing if removed plugin is not observed
    if(!m_plugins.contains(p))
        return;

    // For observed plugins: First Determine the two neighboring
    // connector objects before and after the current rocessing plugin

    // ids of PPCs before/after inserted plugins
    int prevPPC = -1;
    int nextPPC = -1;

    for(int i = position+1; i < p->m_pchain->noPlugs(); i++ )
    {
        ProcessingPlugin* curpp = p->m_pchain->getPlug(i);
        if(curpp->ppc())
        {
            nextPPC = curpp->linkID();
            break;
        }
    }

    for(int i = position-1; i >= 0; i--)
    {
        ProcessingPlugin* curpp = p->m_pchain->getPlug(i);
        if(curpp->ppc())
        {
            prevPPC = curpp->linkID();
            break;
        }
    }

    m_pchainMuteFlag = true;


    // iterate through all plugins,
    // and insert plugin after prevous PPC, before next PPC or at the end
    // of its processing chain to provide a global valid ordering
    // of processing plugins managed by connector objects
    for(int i = 0; i < m_plugins.size(); i++)
    {
        ProcessingPlugin* curpp = m_plugins[i];
        if( curpp==p )
            continue;

        ProcessingChain* pchain = curpp->m_pchain;

        bool success = false;

        if(prevPPC > -1)
        {
            for(int j = 0; j < pchain->noPlugs(); j++)
            {
                ProcessingPlugin* pp = pchain->getPlug(j);
                if(pp->linkID() == prevPPC)
                {
                    curpp->m_pchain->insertPlug( curpp, j+1);
                    success = true;
                    break;
                }
            }
        }

        if(!success && nextPPC > -1)
        {
            for(int j = 0; j < pchain->noPlugs(); j++)
            {
                ProcessingPlugin* pp = pchain->getPlug(j);
                if(pp->linkID() == nextPPC)
                {
                    curpp->m_pchain->insertPlug( curpp, j-1);
                    success = true;
                    break;
                }
            }
        }

        if(!success)
        {
            curpp->m_pchain->insertPlug( curpp, curpp->m_pchain->noPlugs() );
        }
    }
    m_pchainMuteFlag = false;
}


// ---------------------------------------------
// PRIVATE HANDLERS FOR PROCESSINGPLUGIN SIGNALS
// ---------------------------------------------

/**
 * @brief ProcessingPluginConnector::onPlugDataChanged This slot is executed when
 * the "dataChanged" signal is emitted by one of the observed plugins. This happens
 * when the parameters or the active state of a plugin changes.
 * This method updates the data of all other plugins according to the sender plugin
 * @param pos   position modified in data vector
 * @param data  new data value
 */
void ProcessingPluginConnector::onPlugDataChanged(int pos, QVariant data)
{
    if(m_dataMuteFlag)
        return;

    m_dataMuteFlag = true;

    // plugin parameter data changed
    if(pos == 3 && data.toBool() == true)
    {
        // find the ProcessingPlugin whose parameters were modified
        ProcessingPlugin* p = findSenderPlugin(QObject::sender());

        if(!p){
            m_dataMuteFlag = false;
            return;
        }

        // get new/current plugin parametrization
        ProcessingPluginInputList params = p->getInputs();

        QList<ProcessingPlugin*> c = clones(p);

        // copy parameters to all other plugins
        for(int i = 0; i < c.size(); i++)
        {
            try{
                c[i]->setParameters( params );
            }
            catch(LIISimException e)
            {
                MESSAGE(e.what(), e.type());
            }
        }
    }
    // active state changed
    else if( pos == 1)
    {
        ProcessingPlugin* p = findSenderPlugin(QObject::sender());

        if(!p){
            m_dataMuteFlag = false;
            return;
        }

        QList<ProcessingPlugin*> c = clones(p);

        for(int i = 0; i < c.size(); i++)
            c[i]->setActivated(p->activated());

    }
    else if (pos == 5) //plot visibility
    {

        ProcessingPlugin* p = findSenderPlugin(QObject::sender());

        if(!p){
            m_dataMuteFlag = false;
            return;
        }

        QList<ProcessingPlugin*> c = clones(p);

        for(int i = 0; i < c.size(); i++)
            c[i]->setPlotVisibility( p->plotVisibility());

    }
    else if (pos == 6) //step buffer flag
    {

        ProcessingPlugin* p = findSenderPlugin(QObject::sender());

        if(!p){
            m_dataMuteFlag = false;
            return;
        }

        QList<ProcessingPlugin*> c = clones(p);

        for(int i = 0; i < c.size(); i++)
            c[i]->setStepBufferEnabled( p->stepBufferEnabled());

    }
    else if (pos == 4) // link state
    {
        ProcessingPlugin* p = findSenderPlugin(QObject::sender());

        if(!p){
            m_dataMuteFlag = false;
            return;
        }

         // now consider different state transitions
         // from previous link state to new link state

         int prevLinkState = p->m_previousLinkState;
         int linkState = p->linkState();
         m_linkState = linkState;

         // transition A->G and A->S : change link state of all observed A plugins to G/S
         if(   prevLinkState == 2 && linkState == 1
            || prevLinkState == 2 && linkState == 0)
         {
             for(int i = 0; i < m_plugins.size(); i++)
                 if(m_plugins[i]->linkState() == 2) //only A plugins!
                    m_plugins[i]->setLinkState(linkState);

         }


         // transition G->S : change link state to S for all observed plugins within group of p
         if( prevLinkState == 1 && linkState == 0)
         {
            QList<ProcessingPlugin*> c = pluginsInSameGroup(p);
            for(int i = 0; i < c.size(); i++)
                c[i]->setLinkState(0);
         }

         // transition S->G : create missing clones, copy parameters of p to all group processors
         if( prevLinkState == 0 && linkState == 1)
         {
             createMissingClones(p,false,false);
             ProcessingPluginInputList params = p->getInputs();
             QList<ProcessingPlugin*> c = pluginsInSameGroup(p);
             for(int i = 0; i < c.size(); i++)
             {
                 try{
                     c[i]->setParameters(params);
                 }
                 catch(LIISimException e)
                 {
                     MESSAGE(e.what(), e.type());
                 }
                 c[i]->setLinkState(linkState);
             }
         }

         // transition S->A and G->A: create missing clones, copy parameters to all plugins
         if(  prevLinkState == 0 && linkState == 2
            ||prevLinkState == 1 && linkState == 2)
         {
             createMissingClones(p,false,false);
             ProcessingPluginInputList params = p->getInputs();
             for(int i = 0; i < m_plugins.size(); i++)
             {
                 try{
                     m_plugins[i]->setParameters(params);
                 }
                 catch(LIISimException e)
                 {
                     MESSAGE(e.what(), e.type());
                 }
                 m_plugins[i]->setLinkState(linkState);
             }
         }


         if(prevLinkState == -1)
             createClonesOnAllRuns(p,true,true);


         // transition S->NOLINK, G->NOLINK, A->NOLINK: delete all plugins, this implies
         // the destruction of this PPC!
         if(linkState == -1)
         {
             // disconnect all plugins and processingchains
             for(int i = 0; i < m_plugins.size(); i++)
             {
                 m_plugins[i]->disconnect(this);
                 m_plugins[i]->m_pchain->disconnect(this);
                 m_plugins[i]->setLinkState(-1);
             }
             // clear plugin list and commit suicide
             m_plugins.clear();
             this->deleteLater();
         }
    }
    m_dataMuteFlag = false;
}



/**
 * @brief ProcessingPluginConnector::onPlugDestroyed This slot is executed if one of the
 * observed plugins is deleted.
 * @param id
 */
void ProcessingPluginConnector::onPlugDestroyed(int id)
{
    // find sender plugin within list
    ProcessingPlugin* p = 0;
    for(int i = 0; i < m_plugins.size(); i++)
    {
        if(m_plugins[i]->id() == id)
        {
            p = m_plugins[i];
            break;
        }
    }

    if(!p)
    {
        //qDebug() << "ProcessingPluginConnector::onPlugDestroyed its too late ... plugin is already dead, program crashes soon! "<<QObject::sender() << p;
        return;
    }

    // update plugin list, remove plugin(s)

    // -------------------------------------------------------------------------------
    // Option 1: delete all all plugins (and this PPC) if any clone has been destroyed
    // -------------------------------------------------------------------------------

    QList<ProcessingPlugin*> c = m_plugins;

    // -------------------------------------------------------------------------------
    // Option 2: delete all all plugins dependant of A/G/S state
    // -------------------------------------------------------------------------------
    //QList<ProcessingPlugin*> c = clones(p);

    for(int i = 0; i < c.size(); i++)
    {
        c[i]->disconnect(this);
        c[i]->m_pchain->disconnect(this);
        if(c[i]->position() != -1)
            c[i]->m_pchain->removePlug(c[i]->position());
        m_plugins.removeOne(c[i]);
    }

    m_plugins.removeOne(p);

    // delete this connector if the plugin list is empty now
    if(m_plugins.size() == 0)
    {
        this->deleteLater();
    }
}


// ---------------
// PRIVATE HELPERS
// ---------------


/**
 * @brief ProcessingPluginConnector::findSenderPlugin find plugin within internal list based
 * on QObject pointer.
 * @param sender
 * @return ProcessingPlugin-pointer or NULL
 */
ProcessingPlugin* ProcessingPluginConnector::findSenderPlugin(QObject *sender)
{
    ProcessingPlugin* p = 0;
    for(int i = 0; i < m_plugins.size(); i++)
    {
        if(m_plugins[i] == sender)
        {
            p = m_plugins[i];
            break;
        }
    }
    return p;
}


/**
 * @brief ProcessingPluginConnector::clones returns a subset of all observed plugins
 * containing the plugins which should behave exaclty like p.
 * Note: The resulting plugins are in the same link state (and for G, they are
 * also in the same group).
 * This implies that all resulging plugins have the same parameterset.
 * @param p
 * @return
 */
QList<ProcessingPlugin*> ProcessingPluginConnector::clones(ProcessingPlugin *p)
{
    QList<ProcessingPlugin*> clones;


    // no clones for single plugin
    if(p->linkState() == 0)
        return clones;
    else if(p->linkState() == 1) // G
    {
        // get all plugins of ppc within same group and same link state
        int gid = p->m_pchain->mrun()->parentItem()->id();
        for(int i = 0; i < m_plugins.size(); i++)
        {
            if(m_plugins[i]->m_pchain->mrun()->parentItem() &&
               m_plugins[i]->m_pchain->mrun()->parentItem()->id() == gid
               && m_plugins[i]->linkState() == 1
               && m_plugins[i] != p)
                clones << m_plugins[i];
        }
    }
    else if(p->linkState() == 2) // A
    {
        // get all plugins of ppc with same link state
        for(int i = 0; i < m_plugins.size(); i++)
        {
            if(m_plugins[i]->linkState() == 2
               && m_plugins[i] != p)
                clones << m_plugins[i];
        }
    }

    return clones;
}


/**
 * @brief ProcessingPluginConnector::pluginsInSameGroup returns a subset of
 * the observed plugins where all plugins belong the same group as p. Note
 * that the plugins may not be perfect clones (eg. different link state, parameters, etc.).
 * @param p
 * @return
 */
QList<ProcessingPlugin*> ProcessingPluginConnector::pluginsInSameGroup(ProcessingPlugin *p)
{
    // get all plugins of ppc within same group and same link state
    int gid = p->m_pchain->mrun()->parentItem()->id();
    QList<ProcessingPlugin*> res;
    for(int i = 0; i < m_plugins.size(); i++)
    {
        if(m_plugins[i]->m_pchain->mrun()->parentItem()->id() == gid
           && m_plugins[i] != p)
            res << m_plugins[i];
    }
    return res;
}


/**
 * @brief ProcessingPluginConnector::createClonesOnAllRuns
 * add ProcessingPlugin instance (connected whith this connector object)
 * to all mruns
 * @param p ProcessingPlugin instance which should be copied to all runs
 * @param copyParams If set to true (default) the parametrization of p
 * will be copied to the new ProcessingPlugin instances
 * @param copyLinkState If set to true (default) the link state of p
 * will be copied to the new ProcessingPlugin instances
 */
void ProcessingPluginConnector::createClonesOnAllRuns(ProcessingPlugin *p, bool copyParams, bool copyLinkState)
{
    if(!m_active)
        return;

    bool oldFlag = m_dataMuteFlag;
    m_dataMuteFlag = true;

    QList<MRun*> mruns = Core::instance()->dataModel()->mrunList();
    if(m_plugins.size() == mruns.size())
    {
        m_dataMuteFlag = oldFlag;
        return;
    }

    QList<ProcessingPlugin*> tmp = m_plugins;
    ProcessingPluginInputList params = p->getInputs();

    for(int i = 0; i < mruns.size() ; i++)
    {
        MRun* m = mruns[i];
        bool success = false;
        for(int j = 0; j < tmp.size(); j++)
        {
            if(tmp[j]->mrun == m)
            {
                tmp.removeAt(j);
                success =true;
                break;
            }
        }
        if(!success)
        {
            ProcessingPlugin* clone = PluginFactory::createInstance(m_pname,m->getProcessingChain(m_stype));

            try{
                if(copyParams)
                    clone->setParameters(params);
            }
            catch(LIISimException e)
            {
                MESSAGE(e.what(), e.type());
            }

            clone->setPlotVisibility(p->plotVisibility());
            clone->setActivated(p->activated());

            m->getProcessingChain(m_stype)->addPlug(clone);
            add(clone);
            if(copyLinkState)
                clone->setLinkState(p->linkState());
        }
    }
    m_dataMuteFlag = oldFlag;
}


/**
 * @brief ProcessingPluginConnector::createMissingClones Dependent of the link
 * state of p, missing plugin clones of p are created. This may be necessary
 * for certain state transitions of p (eg. S->G, S->A, G->A)
 * create missing plugin instances for this PPC. All runs should own an instance of a observed
 * plugin
 * @param p
 * @param copyParams copy the parameters of p to the new clones (default value = true)
 * @param copyLinkState copy the current link state of p to the new clones (default value = true)
 */
void ProcessingPluginConnector::createMissingClones(ProcessingPlugin *p, bool copyParams, bool copyLinkState)
{
    if(!m_active)
        return;

    bool oldFlag = m_dataMuteFlag;
    m_dataMuteFlag = true;

    QList<MRun*> mruns = Core::instance()->dataModel()->mrunList();
    if(m_plugins.size() == mruns.size())
    {
        m_dataMuteFlag = oldFlag;
        return;
    }

    // -------------------------------------------------------------
    // make shure that ALL mruns contain a plugin instance
    // -------------------------------------------------------------

    // add ProcessingPlugin instance (connected whith this connector object) on all mruns
    createClonesOnAllRuns(p,copyParams,copyLinkState);
    m_dataMuteFlag = oldFlag;
}
