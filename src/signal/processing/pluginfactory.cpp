#include "pluginfactory.h"

#include <QApplication>
#include <QStringList>
#include <QFileDialog>
#include <QDateTime>

#include "processingplugin.h"
#include "processingchain.h"
#include "processingpluginconnector.h"

//basic operations
#include "plugins/arithmetic.h"
#include "plugins/baseline.h"
#include "plugins/calibration.h"
#include "plugins/filterplugin.h"
#include "plugins/getsignalsection.h"
#include "plugins/multisignalaverage.h"
#include "plugins/normalize.h"
#include "plugins/overwrite.h"
#include "plugins/resolutionreducer.h"
#include "plugins/signalarithmetic.h"
#include "plugins/simpledatareducer.h"
#include "plugins/swapchannels.h"
#include "plugins/xshiftsignals.h"
#include "plugins/transfer.h"

// mathematic tools
#include "plugins/convolution.h"
#include "plugins/movingaverage.h"
#include "plugins/savitzkygolay.h"

//validation
#include "plugins/simplepeakvalidator.h"

//temperature
#include "plugins/temperaturecalculator.h"

#include "../../general/LIISimException.h"
#include "../mrun.h"
#include "../../core.h"
#include "../mrungroup.h"

/***
 *
 *  Plugins are loaded within this file
 *
 *
 *
    1) Include header file:

        #include "plugins/arithmetic.h"

    2) Add Plugin to <namelist>

        PluginFactory::PluginFactory():

        nameList << Arithmetic::pluginName;

    3) Add Plugin to pluginNames list if supported for signal type

        PluginFactory::getAvailablePluginNames():

        if(Arithmetic::supportedSignalTypes.contains(stype))
            pluginNames << Arithmetic::pluginName;

    4) Create Instance of PluginClass

        PluginFactory::createInstance():

        if(pluginName == Arithmetic::pluginName)
            return new Arithmetic( parentChain);
*
 * */

PluginFactory* PluginFactory::m_instance = 0;

/**
 * @brief PluginFactory::PluginFactory Konstructor
 */
PluginFactory::PluginFactory() : QObject(0)
{
    MSG_DETAIL_1("init PluginFactory");
    QStringList nameList;
    nameList << Arithmetic::pluginName;
    nameList << Baseline::pluginName;
    nameList << Calibration::pluginName;
    nameList << Convolution::pluginName;
    nameList << FilterPlugin::pluginName;
    nameList << GetSignalSection::pluginName;
    nameList << MovingAverage::pluginName;
    nameList << MultiSignalAverage::pluginName;
    nameList << Normalize::pluginName;
    nameList << Overwrite::pluginName;
    nameList << ResolutionReducer::pluginName;    
    nameList << SavitzkyGolay::pluginName;
    nameList << SignalArithmetic::pluginName;
    nameList << SimplePeakValidator::pluginName;
    nameList << SimpleDataReducer::pluginName;
    nameList << SwapChannels::pluginName;
    nameList << TemperatureCalculator::pluginName;
    nameList << XShiftSignals::pluginName;
    nameList << Transfer::pluginName;

    for(int i = 0; i < nameList.size(); i++)
    {
        QString n = nameList.at(i);
        QAction* ac_noLink = new QAction(n,this);
        QAction* ac = new QAction(n,this);
        QAction* ac2 = new QAction(n,this);
        QAction* ac3 = new QAction(n,this);

        ac_noLink->setData(-1);
        ac->setData(0);
        ac2->setData(1);
        ac3->setData(2);

        connect(ac_noLink,SIGNAL(triggered()),SLOT(addActionTriggered()));
        connect(ac,SIGNAL(triggered()),SLOT(addActionTriggered()));
        connect(ac2,SIGNAL(triggered()),SLOT(addActionTriggered()));
        connect(ac3,SIGNAL(triggered()),SLOT(addActionTriggered()));

        m_actions_addPlug_noLink.append(ac_noLink);
        m_actions_addPlug_S.append(ac);
        m_actions_addPlug_G.append(ac2);
        m_actions_addPlug_A.append(ac3);
    }

    m_rawPlugs = getAvailablePluginNames(Signal::RAW);
    m_absPlugs = getAvailablePluginNames(Signal::ABS);
    m_tempPlugs= getAvailablePluginNames(Signal::TEMPERATURE);
    m_pchain = 0;

    m_action_save_pchain = new QAction("Save Processing Chain",this);
    m_action_load_pchain = new QAction("Load Processing Chain from file",this);

    connect(m_action_save_pchain,SIGNAL(triggered()),SLOT(saveChain()));
    connect(m_action_load_pchain,SIGNAL(triggered()),SLOT(loadChain()));

    // create actions for state changes of plugins
    m_currentContextPlugin = 0;
    m_stateChangeActions = new QActionGroup(this);
    m_stateChangeActions->setExclusive(true);
    m_stateAction_all = new QAction("linked, same parameters for all runs",this);
    m_stateAction_group = new QAction("linked, same parameters for all runs in group",this);
    m_stateAction_single = new QAction("linked, individual parameters per run",this);
    m_stateAction_noLink = new QAction("no link",this);
    m_stateAction_all->setCheckable(true);
    m_stateAction_group->setCheckable(true);
    m_stateAction_single->setCheckable(true);
    m_stateAction_noLink->setCheckable(true);
    m_stateChangeActions->addAction(m_stateAction_all);
    m_stateChangeActions->addAction(m_stateAction_group);
    m_stateChangeActions->addAction(m_stateAction_single);
    m_stateChangeActions->addAction(m_stateAction_noLink);
    m_stateAction_noLink->setChecked(true);

    connect(m_stateAction_all,SIGNAL(triggered(bool)),
            SLOT(stateChangeActionTriggered(bool)));
    connect(m_stateAction_group,SIGNAL(triggered(bool)),
            SLOT(stateChangeActionTriggered(bool)));
    connect(m_stateAction_single,SIGNAL(triggered(bool)),
            SLOT(stateChangeActionTriggered(bool)));
    connect(m_stateAction_noLink,SIGNAL(triggered(bool)),
            SLOT(stateChangeActionTriggered(bool)));

}


/**
 * @brief PluginFactory::getAvailablePluginNames Get a list of all available pluginnames
 * @return String List of all available processing plugins
 * @details (The GUI uses this list to create a plugin selection)
 */
QStringList PluginFactory::getAvailablePluginNames(Signal::SType stype)
{
    QStringList pluginNames;

    if(Arithmetic::supportedSignalTypes.contains(stype))
        pluginNames << Arithmetic::pluginName;

    if(Calibration::supportedSignalTypes.contains(stype))
        pluginNames << Calibration::pluginName;

    if(FilterPlugin::supportedSignalTypes.contains(stype))
        pluginNames << FilterPlugin::pluginName;

    if(SignalArithmetic::supportedSignalTypes.contains(stype))
        pluginNames << SignalArithmetic::pluginName;

    if(Baseline::supportedSignalTypes.contains(stype))
        pluginNames << Baseline::pluginName;

    if(Normalize::supportedSignalTypes.contains(stype))
        pluginNames << Normalize::pluginName;

    if(Overwrite::supportedSignalTypes.contains(stype))
        pluginNames << Overwrite::pluginName;

    if(ResolutionReducer::supportedSignalTypes.contains(stype))
        pluginNames << ResolutionReducer::pluginName;

    if(SwapChannels::supportedSignalTypes.contains(stype))
        pluginNames << SwapChannels::pluginName;

    if(GetSignalSection::supportedSignalTypes.contains(stype))
        pluginNames << GetSignalSection::pluginName;

    if(MovingAverage::supportedSignalTypes.contains(stype))
        pluginNames << MovingAverage::pluginName;

    if(Transfer::supportedSignalTypes.contains(stype))
        pluginNames << Transfer::pluginName;

#ifdef LIISIM_FULL

    // these plugins are experimental and not tested!

    if(Convolution::supportedSignalTypes.contains(stype))
        pluginNames << Convolution::pluginName;

    if(SavitzkyGolay::supportedSignalTypes.contains(stype))
        pluginNames << SavitzkyGolay::pluginName;

    if(XShiftSignals::supportedSignalTypes.contains(stype))
        pluginNames << XShiftSignals::pluginName;

    if(SimplePeakValidator::supportedSignalTypes.contains(stype))
        pluginNames << SimplePeakValidator::pluginName;

#endif

    if(TemperatureCalculator::supportedSignalTypes.contains(stype))
        pluginNames << TemperatureCalculator::pluginName;

    if(MultiSignalAverage::supportedSignalTypes.contains(stype))
        pluginNames << MultiSignalAverage::pluginName;

    if(SimpleDataReducer::supportedSignalTypes.contains(stype))
        pluginNames << SimpleDataReducer::pluginName;

    return pluginNames;
}


/**
 * @brief PluginFactory::createInstance create an plugin instance with given name for given signal type
 * @param pluginName name of plugin
 * @param stype signal type which should be processed by plugin
 * @return ProcessingPlugin
 * @details The caller takes the responsibility for deleting the returned object!
 */
ProcessingPlugin* PluginFactory::createInstance(QString pluginName,  ProcessingChain *parentChain)
{
    if(pluginName == Arithmetic::pluginName)
        return new Arithmetic( parentChain);
    else if(pluginName == Calibration::pluginName)
        return new Calibration( parentChain);
    else if(pluginName == FilterPlugin::pluginName)
        return new FilterPlugin( parentChain);
    else if(pluginName == SignalArithmetic::pluginName)
        return new SignalArithmetic( parentChain);
    else if(pluginName == Baseline::pluginName)
        return new Baseline( parentChain);
    else if(pluginName == Normalize::pluginName)
        return new Normalize( parentChain);
    else if(pluginName == Overwrite::pluginName)
        return new Overwrite( parentChain);
    else if(pluginName == ResolutionReducer::pluginName)
        return new ResolutionReducer( parentChain);
    else if(pluginName == SwapChannels::pluginName)
        return new SwapChannels( parentChain);
    else if(pluginName == XShiftSignals::pluginName)
        return new XShiftSignals( parentChain);
    else if(pluginName == GetSignalSection::pluginName)
        return new GetSignalSection( parentChain);
    else if(pluginName == Convolution::pluginName)
        return new Convolution( parentChain);
    else if(pluginName == MovingAverage::pluginName)
        return new MovingAverage( parentChain);
    else if(pluginName == SavitzkyGolay::pluginName)
        return new SavitzkyGolay( parentChain);
    else if(pluginName == SimplePeakValidator::pluginName)
        return new SimplePeakValidator( parentChain);
    else if(pluginName == TemperatureCalculator::pluginName)
        return new TemperatureCalculator( parentChain);
    else if(pluginName == MultiSignalAverage::pluginName)
        return new MultiSignalAverage( parentChain);
    else if(pluginName == SimpleDataReducer::pluginName)
        return new SimpleDataReducer( parentChain);
    else if(pluginName == Transfer::pluginName)
        return new Transfer(parentChain);

    throw LIISimException("PluginManager: cannot create instance of "+pluginName);
}


QList<QAction*> PluginFactory::pluginCreationActions(ProcessingChain *pchain, int pluginLinkType)
{
    if(m_pchain)
        m_pchain->disconnect(this);

    m_pchain = pchain;

    if(m_pchain)
        connect(m_pchain,SIGNAL(destroyed()),SLOT(onPchainDestroyed()));

    QList<QAction*> startList;
    if(pluginLinkType == -1)
        startList = m_actions_addPlug_noLink;
    else if(pluginLinkType == 0)
        startList = m_actions_addPlug_S;
    else if(pluginLinkType == 1)
        startList = m_actions_addPlug_G;
    else if(pluginLinkType == 2)
        startList = m_actions_addPlug_A;

    QList<QAction*> resList;

    for(int i = 0; i < startList.size(); i++)
    {
        QString pname = startList.at(i)->text();
        switch (pchain->getSignalType()) {
        case Signal::RAW:
            if(m_rawPlugs.contains(pname))
                resList.append(startList.at(i));
            break;
        case Signal::ABS:
            if(m_absPlugs.contains(pname))
                resList.append(startList.at(i));
            break;
        case Signal::TEMPERATURE:
            if(m_tempPlugs.contains(pname))
                resList.append(startList.at(i));
        }
    }
    return resList;
}



QList<QAction*> PluginFactory::recentChains()
{
    return m_recentPchains;
}


void PluginFactory::addActionTriggered()
{
    if(!m_pchain)
        return;
    Signal::SType stype= m_pchain->getSignalType();
    QAction* ac = qobject_cast<QAction*> ( QObject::sender());
    int pluginLinkType = ac->data().toInt();

    if(!ac)
        return;

    QString pname = ac->text();

    if(Core::instance()->getSignalManager()->isBusy())
    {
        QString msg = "Cannot add plugin (active backgrouds tasks!)";
        MSG_NORMAL(msg);
        MSG_STATUS(msg);
        return;
    }

    if(pluginLinkType > -1)
    {
        ProcessingPluginConnector* c = new ProcessingPluginConnector(pname,pluginLinkType,m_pchain);
        c->setPluginsLinkState(pluginLinkType);
    }
    else
    {
        ProcessingPlugin* plug = createInstance(pname,m_pchain);

        m_pchain->addPlug(plug);

        // for single instance TemperatureCalcualtors we need to initialize
        // a new unique TemperatureChannelID
        if(pname == TemperatureCalculator::pluginName)
        {
            TemperatureCalculator* tc =dynamic_cast<TemperatureCalculator*>(plug);
            tc->setTemperatureChannelID(TemperatureCalculator::generateTemperatureChannelID());
        }
    }
    m_pchain = 0;
}



QList<QAction*> PluginFactory::pluginStateChangeActions(ProcessingPlugin *currentContextPlugin)
{
    // change current context plugin

    if(m_currentContextPlugin)
        m_currentContextPlugin->disconnect(this);

    m_currentContextPlugin = currentContextPlugin;

    if(m_currentContextPlugin)
    {
        connect(m_currentContextPlugin,SIGNAL(destroyed()),SLOT(onCurrentContextPluginDestroyed()));

        // update checked states of actions
        int ls = m_currentContextPlugin->linkState();
        if(ls == 0)
            m_stateAction_single->setChecked(true);
        else if(ls == 1)
            m_stateAction_group->setChecked(true);
        else if(ls == 2)
            m_stateAction_all->setChecked(true);
        else if(ls == -1)
            m_stateAction_noLink->setChecked(true);
    }
    return m_stateChangeActions->actions();
}


void PluginFactory::stateChangeActionTriggered(bool state)
{
    if(!state)return;
    if(!m_currentContextPlugin)return;

    QObject* s = QObject::sender();
    if(s == m_stateAction_all)
         m_currentContextPlugin->setLinkState(2);
    else if( s == m_stateAction_group)
        m_currentContextPlugin->setLinkState(1);
    else if( s == m_stateAction_single)
        m_currentContextPlugin->setLinkState(0);
    else if( s == m_stateAction_noLink)
        m_currentContextPlugin->setLinkState(-1);

}


void PluginFactory::onPchainDestroyed()
{
    m_pchain = 0;
}


void PluginFactory::onCurrentContextPluginDestroyed()
{
    m_currentContextPlugin = 0;
}


void PluginFactory::loadChain(QString fname)
{
    QList<QVariant> acData = m_action_load_pchain->data().toList();

    if(acData.size() < 2)
        return;

    int runID = acData.first().toInt();
    int pchID = acData.last().toInt();

    MRun* run = Core::instance()->dataModel()->mrun(runID);

    if(!run)
        return;

    ProcessingChain* pchain = qobject_cast<ProcessingChain*>(
                run->findChildById(pchID));

    if(Core::instance()->getSignalManager()->isBusy())
    {
        QString msg = "Cannot load processing chain (active background tasks!)";
        MSG_INFO(msg);
        MSG_STATUS(msg);
        return;
    }

    if(!pchain)
        return;

    if(fname.isEmpty())
        fname = QFileDialog::getOpenFileName(QApplication::focusWidget () ,
                                     "load processing steps",
                                     Core::instance()->generalSettings->procChainDirectory(),
                                     ".ini (*.ini)");
    if(fname.isEmpty())
        return;

    try
    {
        pchain->load(fname);
        MSG_DEBUG("loaded processing steps from: "+fname);
        MSG_STATUS("loaded processing steps from: "+fname);

        addRecentChain(fname);
    }
    catch(LIISimException e)
    {
        pchain->clearAll();
        MSG_ERR(e.what());
        MSG_STATUS(e.what());
    }
}


/**
 * @brief PluginFactory::onRecentChainTriggerd This slot is executed if an action
 * from the "recent actions" list has been triggered
 */
void PluginFactory::onRecentChainTriggerd()
{
    QAction* ac = qobject_cast<QAction*> ( QObject::sender());

    if(!ac)
        return;

    QString fname = ac->data().toString();

    loadChain(fname);
}


/**
 * @brief PluginFactory::addRecentChain adds a processing chain with given filename to the
 * "recent processing chains" list
 * @param fname
 */
void PluginFactory::addRecentChain(QString fname)
{

    for(int i = 0; i < m_recentPchains.size(); i++)
    {
        if(m_recentPchains.at(i)->data().toString() == fname)
        {
            return;
        }
    }

    QAction* ac = new QAction(fname,this);
    ac->setData(fname);
    connect(ac,SIGNAL(triggered()),SLOT(onRecentChainTriggerd()));

    m_recentPchains.append(ac);

    if(m_recentPchains.size() > 5)
    {
        m_recentPchains.removeFirst();
    }

    QStringList newSettingsValue;
    for(int i = 0; i < m_recentPchains.size();i++)
    {
        newSettingsValue.append(m_recentPchains.at(i)->data().toString());
    }
    Core::instance()->guiSettings->setValue("se", "recentChains", newSettingsValue);
}


void PluginFactory::saveChain()
{
    QList<QVariant> acData = m_action_save_pchain->data().toList();

    if(acData.size() < 2)
        return;

    int runID = acData.first().toInt();
    int pchID = acData.last().toInt();

    MRun* run = Core::instance()->dataModel()->mrun(runID);

    if(!run)
        return;

    ProcessingChain *pchain = qobject_cast<ProcessingChain*>(
                run->findChildById(pchID));

    if(Core::instance()->getSignalManager()->isBusy())
    {
        QString msg = "Cannot save processing chain (active background tasks!)";
        MSG_INFO(msg);
        MSG_STATUS(msg);
        return;
    }

    if(!pchain)
        return;

    if(pchain->noPlugs() == 0)
    {
        MSG_STATUS("Why saving an empty processing chain? canceled.");
        return;
    }

    try
    {
        QString dirname = Core::instance()->generalSettings->procChainDirectory();
        if(!dirname.endsWith("/"))
            dirname.append("/");

        QString typestr = "";
        switch(pchain->getSignalType())
        {
            case Signal::RAW:
                typestr = "_raw_";
                break;
            case Signal::ABS:
                typestr = "_abs_";
                break;
            case Signal::TEMPERATURE:
                typestr = "_temperature_";
                break;
        }

        QDateTime now = QDateTime::currentDateTime();
        QString datestr = now.toString("yyyy-MM-dd");
        QString fname = dirname + "pchain" + typestr + datestr;

        fname = QFileDialog::getSaveFileName(QApplication::focusWidget ()
                                             ,"save processing steps",
                                             fname,
                                             ".ini (*.ini)");
        if(fname.isEmpty())
            return;

        pchain->save(fname);
        addRecentChain(fname);

        MSG_DEBUG("saved processing steps: "+fname);
        MSG_STATUS("saved processing steps: "+fname);

    }
    catch(LIISimException e)
    {
        MSG_ERR(e.what());
        MSG_STATUS(e.what());
    }
}


void PluginFactory::handleApplicationStartup()
{
    QList<QVariant> lastChains = Core::instance()->guiSettings->value("se","recentChains").toList();

    for(int i = 0; i < lastChains.size();i++)
    {
        addRecentChain(lastChains.at(i).toString());
    }
}

