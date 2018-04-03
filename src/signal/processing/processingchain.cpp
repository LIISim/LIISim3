#include "processingchain.h"

#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "../mrun.h"
#include "../../general/LIISimException.h"
#include <QDebug>
#include <QMutexLocker>
#include <QTime>
#include <QSettings>
#include <QFile>
#include "pluginfactory.h"
#include "../../core.h"
#include "plugins/multisignalaverage.h"
#include "plugins/temperaturecalculator.h"
#include "plugins/dummyplugin.h"
#include "processingpluginconnector.h"
#include "processingplugin.h"

/**
 * @brief ProcessingChain::ProcessingChain Constructor
 * @param mrun Measurement run the processing chain belongs to
 * @param stype signal type the processing chain should operate on
 * @param parent parent object (default = 0)
 */
ProcessingChain::ProcessingChain(MRun* mrun,  Signal::SType stype)
    :  DataItem(mrun)
{
    this->stype = stype;
    this->m_mrun = mrun;

    m_msaPosition = -1;
    m_isMovinPlugin = false;

    switch(stype){
        case Signal::RAW:
            setData(0,"Processing steps for raw signals");
            break;
        case Signal::ABS:
            setData(0,"Processing steps for absolute signals");
            break;
        case Signal::TEMPERATURE:
            setData(0,"Processing steps for temperature signals");
    }
}


/**
 * @brief ProcessingChain::~ProcessingChain Destructor
 */
ProcessingChain::~ProcessingChain()
{
    clearAll();
}

void ProcessingChain::clearAll()
{
    while(!plugs.isEmpty())
    {
        ProcessingPlugin* p = plugs.first();
        plugs.pop_front();
        delete p;
    }
}



/**
 * @brief ProcessingChain::getMRunName
 * @return name of measurement run the chain is applied to
 */
QString ProcessingChain::getMRunName()
{
    return m_mrun->getName();
}


/**
 * @brief ProcessingChain::getSignalType
 * @return signal type the chain operates on
 */
Signal::SType ProcessingChain::getSignalType()
{
    return stype;
}


/**
 * @brief ProcessingChain::noPlugs get number of registered plugins
 * @return number of plugins
 */
int ProcessingChain::noPlugs()
{
   // QMutexLocker lock(&mutexPlugList);
    return plugs.size();
}


/**
 * @brief ProcessingChain::getPlug get ProcessingPlugin at given indes
 * @param idx index of plugin
 * @return requested plugin
 * @details Trhows LIISimException if the requested index exceeds plugin list boundaries.
 */
ProcessingPlugin* ProcessingChain::getPlug(int idx)
{
    //QMutexLocker lock(&mutexPlugList);
    if(idx < 0 || idx > plugs.size())
    {
        QString val;
        return 0;
        val.sprintf("%d",idx);
        throw LIISimException("ProcessingChain.getPlug: invalid index: "+val);
    }
    return plugs.at(idx);
}


/**
 * @brief ProcessingChain::getPluginByID
 * @param id
 * @return
 */
ProcessingPlugin* ProcessingChain::getPluginByID(int id)
{
    ProcessingPlugin* res = 0;
    for(int i = 0;i < plugs.size();i++)
        if(plugs.at(i)->id() == id)
            res = plugs.at(i);
    return res;
}


/**
 * @brief ProcessingChain::movePlugin move PPlugin to a certaoin position in chain
 * @param from index of plugin which should be moved
 * @param to destination index
 * @param insertBefore if set to true Plugin will be inserted before destination index
 * (default false)
 */
void ProcessingChain::movePlugin(int from, int to, bool insertBefore)
{
    if(Core::instance()->getSignalManager()->isBusy())
    {
        QString msg = "cannot move processing step (active background tasks!)";
        MSG_NORMAL(msg);
        MSG_STATUS(msg);
        return;
    }

    if(from < 0 || from >= plugs.size() || to < 0 || to >= plugs.size() )
        return;

    if(from == to)
        return;

    m_isMovinPlugin = true;
    ProcessingPlugin* p = plugs[from];
    removePlug(from,false);
    if(to>from)
    {
        if(insertBefore)
            insertPlug(p,to-1);
        else
            insertPlug(p,to);
    }
    else
    {
        if(insertBefore)
            insertPlug(p,to);
        else
            insertPlug(p,to+1);
    }

    m_isMovinPlugin = false;
}


void ProcessingChain::initializeCalculation()
{

    m_msaPosition = this->indexOfPlugin(MultiSignalAverage::pluginName);


    for(int i = 0; i < plugs.size(); i++)
    {
        plugs[i]->initializeCalculation();
    }

    // if a plugin in list needs to be recalculated,
    // all successors need to be recalculated too!
    bool recalculate = false;
    if(!plugs.isEmpty())
        plugs.first()->setDirty(true);
}


/**
 * @brief ProcessingChain::addPlug register a ProcessingPlugin to this chain
 * @param p ProcessingPlugin
 * @details The chain takes care of deleting the given ProcessingPlugin. The Plugin
 * will be inserted at the end of the plugin list.
 */
void ProcessingChain::addPlug(ProcessingPlugin *p)
{

    plugs.push_back(p);

    int noPlugs =plugs.size();
    p->positionInChain = noPlugs-1;

    connect(p, SIGNAL(dataChanged(int,QVariant)), SLOT(onPluginDataChanged(int,QVariant)));


    // all plugins need to be recalculated if we resize the step data !!!
    // TODO: prevent step data recalculation by copying the old data ...
    // plugs.first()->setDirty(true);

    this->insertChild(p);
    p->onAddedToPchain();
}


void ProcessingChain::insertPlug(ProcessingPlugin *p, int position)
{
    if(position < 0 )
        position = 0;
    else if( position > plugs.size())
        position = plugs.size();

    plugs.insert(position,p);
    // update chain positions of all plugins
    for(int i = 0; i < plugs.size(); i++)
    {
        plugs.at(i)->positionInChain = i;
    }
    this->insertChild(p,position);


    p->setDirty(true);
    p->onAddedToPchain();
}


/**
 * @brief ProcessingChain::removePlug removes ProcessingPlugin at given index from processing chain.
 * Deletion of the Plugin instance can be disabled.
 * @param idx index of the ProcessingPluing within chain
 * @param deletePlugin Optional parameter (default value: delete instance).
 * If set to false the ProcssingPlugin instance will not be deleted.
 */
void ProcessingChain::removePlug(int idx,bool deletePlugin)
{

    if(idx < 0 || idx >= plugs.size())
    {
        QString msg;
        msg.sprintf("ProcessingChain::removePlug: requested invalid index: %d",idx);
        qDebug() << msg;
        return;
        //throw LIISimException(msg);
    }

    ProcessingPlugin* p = plugs.at(idx);
    if(deletePlugin)
    {
        delete p;
    }

    plugs.removeAt(idx);

    // update chain positions of all plugins
    for(int i = 0; i < plugs.size(); i++)
    {
        plugs.at(i)->positionInChain = i;
    }

    if(!deletePlugin)
    {
        this->removeChild(p);
    }

    if( !plugs.isEmpty() && idx < plugs.size() && idx >= 0)
    {
        plugs.at(idx)->setDirty(true);
    }
}


/**
 * @brief ProcessingChain::getStepSignalPre get results from unprocessed signal
 * @param mpIdx index of MPoint
 * @param chID  channel id of requested signal
 * @param stepIdx index of calculation step
 * @return Signal at given MPoint index, channel id and intermediate step position
 */
Signal ProcessingChain::getStepSignalPre(int mpIdx, int chID , int stepIdx)
{
    //QMutexLocker lock2(&mutexStepData);
    //QMutexLocker lock(&mutexPlugList);

    if(stepIdx < 0)
        return m_mrun->getPre(mpIdx)->getSignal(chID,stype);
    else
    {
        return plugs.at(stepIdx)->processedSignal(mpIdx,chID);
        // return stepData[stepIdx][mpIdx][chID-1];
    }
}


/**
 * @brief ProcessingChain::isValid check if a measurement point has passed validation for this chain
 * @param mpIdx index in MPoint data list
 * @return true if the signals managed by this chain have passed all validations
 */
bool ProcessingChain::isValid(int mpIdx)
{
   // QMutexLocker lock2(&mutexPassedPlugins);
   // QMutexLocker lock(&mutexPlugList);

    // no plugins => all signals managed by this chain are valid
    if(plugs.isEmpty())
        return true;

    if(mpIdx < 0 || mpIdx >= m_mrun->sizeAllMpoints())
    {
        qDebug() << "\tProcessingChain::isValidinvalid mpoint index";
        return false;
        QString msg;
        msg.sprintf("ProcessingChain::isValid: index exceeds boundaries: ", mpIdx);
        throw LIISimException(msg);
    }
    //qDebug() << "testing size done";

    int counter =0;

    for(int i = 0; i < plugs.size(); i++)
    {

        if(plugs[i]->validAt(mpIdx) || !plugs[i]->activated())
            counter++;
    }

    // validation has failed if not all plugins have been calculated!
    if(counter != plugs.size())
    {
        return false;
    }
    return true;
}


bool ProcessingChain::isValidAtStep(int mpIdx, int step)
{
  // no plugins => all signals managed by this chain are valid
  //  if(plugs.isEmpty())
  //      return true;

    if(mpIdx < 0 || mpIdx >= m_mrun->sizeAllMpoints())
    {
        return false;
       // QString msg;
       // msg.sprintf("ProcessingChain::isValid: index exceeds boundaries: ", mpIdx);
       // throw LIISimException(msg);
    }

    if(step < 0 || step >= plugs.size())
    {
        return false;
    }

    bool res = plugs[step]->validAt(mpIdx);

    return res;
}


/**
 * @brief ProcessingChain::contains check if the chain contains a plugin with given name
 * @param pluginName name of ProcessingPlugin
 * @return true if processingchain contains a plugin with given name
 */
bool ProcessingChain::contains(QString &pluginName)
{
    //QMutexLocker lock(&mutexPlugList);

    for(int i = 0; i < plugs.size(); i++)
    {
        if( plugs.at(i)->getName() == pluginName)
            return true;
    }
    return false;
}


/**
 * @brief ProcessingChain::containsActivePlugin  check if the chain contains an active plugin with given name
 * @param pluginName pluginName name of ProcessingPlugin
 * @return true if processingchain contains an active plugin with given name
 */
bool ProcessingChain::containsActivePlugin(QString &pluginName)
{
    //QMutexLocker lock(&mutexPlugList);

    for(int i = 0; i < plugs.size(); i++)
    {
        if( plugs.at(i)->getName() == pluginName && plugs.at(i)->activated())
            return true;
    }
    return false;
}


/**
 * @brief ProcessingChain::containsActiveMSA returns true
 * if this chain contains an active MultiSignalAverage Plugin instance.
 * For TemperatureCalculator Plugins: Source Chain is also considered
 * @return
 */
bool ProcessingChain::containsActiveMSA()
{
    for(int i = 0; i < plugs.size(); i++)
    {
        ProcessingPlugin* p = plugs.at(i);

        if(p->getName() == MultiSignalAverage::pluginName && p->activated())
            return true;

        if(stype == Signal::TEMPERATURE &&
           p->getName() == TemperatureCalculator::pluginName)
        {
            bool res = false;
            QString st = p->getInputs().getValue("signalType").toString();
            if(st == "Raw")
                res = m_mrun->getProcessingChain(Signal::RAW)->containsActiveMSA();
            else if(st == "Absolute")
                res = m_mrun->getProcessingChain(Signal::ABS)->containsActiveMSA();
            if(res)return res;
        }
    }
    return false;
}


/**
 * @brief ProcessingChain::indexOfPlugin returns index of first occurance
 * of plugin with given name
 * @param pluginName name of processingplugin
 * @param considerInactive also consider inactive plugins
 * @return index of plugin or -1 if plugin is not found or inactive
 */
int ProcessingChain::indexOfPlugin(QString &pluginName, bool considerInactive)
{
    for(int i = 0; i < plugs.size(); i++)
    {
        if( plugs.at(i)->getName() == pluginName)
        {
            if(considerInactive)
                return i;
            else
                if( plugs.at(i)->activated())
                    return i;
        }
    }
    return -1;
}


/**
 * @brief ProcessingChain::save save processing chain to .ini file
 * @param fname filename
 */
void ProcessingChain::save(QString fname)
{
    QSettings settings(fname,QSettings::IniFormat);
    settings.clear();

    for(int i = 0; i < plugs.size(); i++)
    {
        ProcessingPlugin* p = plugs.at(i);

        if(p->getName() == DummyPlugin::pluginName)
            continue;

        QString gname;
        gname.sprintf("proc_step_%d",i);
        settings.beginGroup(gname);

        settings.setValue("pName", p->getName() );
        settings.setValue("pActivated",p->activated());
        settings.setValue("pLinkState",p->linkState());
        settings.setValue("pLinkType",p->linkType());
        settings.setValue("pVisible",p->plotVisibility());
        settings.setValue("sbFlag",p->stepBufferEnabled());

        ProcessingPluginInputList l = p->getInputs();

        for(int j = 0; j < l.size(); j++)
        {
            ProcessingPluginInput pi = l.at(j);

            settings.setValue(pi.identifier+"-"+pi.typeToString(), pi.value );
        }
        settings.endGroup();
    }
}


/**
 * @brief ProcessingChain::load load processing chain from .ini file
 * @param fnamefilename
 */
void ProcessingChain::load(QString fname)
{
    // check if file exists
    QFile file( fname );
    if(!file.exists())
    {
        MSG_ERR("cannot load processing chain: "+fname+" (file does not exist!)");
        return;
    }
    file.close();

    //clearAll();

    // read values from file
    QSettings settings(fname,QSettings::IniFormat);
    QStringList avPlugs = PluginFactory::getAvailablePluginNames(stype);

    int i = 0;
    while(true)
    {
        QString gname;
        gname.sprintf("proc_step_%d",i);
        settings.beginGroup(gname);
        if(!settings.contains("pName"))
        {
            settings.endGroup();
            break;
        }
        QString pname = settings.value("pName").toString();

        QStringList keys = settings.allKeys();
        ProcessingPluginInputList l;

        bool activated = settings.value("pActivated",true).toBool();
        bool visible = settings.value("pVisible",true).toBool();
        //int linkType = settings.value("pLinkType",-1).toInt();
        int linkState = settings.value("pLinkState",-1).toInt();
        bool sbFlag = settings.value("sbFlag",true).toBool();

        for(int j = 0; j < keys.size(); j++)
        {
            QString k = keys.at(j);

            if( k == "pName" ||
                k == "pActivated" ||
                k == "pVisible" ||
                k == "pLinkType" ||
                k == "pLinkState" ||
                k == "sbFlag")
                continue;

            QStringList keySplit = k.split("-");
            if(keySplit.size()<2)
                throw LIISimException("ProcessingChain::load: invalid key: "+k);

            ProcessingPluginInput pi;
            pi.identifier = keySplit.at(0);
            pi.setTypeFromInt(keySplit.at(1).toInt());
            pi.value = settings.value(k);

            l.push_back( pi );
        }

        if(!avPlugs.contains(pname) )
        {
            throw LIISimException("ProcessingChain::load: no plugin found with name '"+pname+"' for signal type of chain");
        }

        // load 'not linked' plugins at least as 'S-linked'
        if(linkState == -1)
            linkState = 0;


        ProcessingPluginConnector* ppc =new ProcessingPluginConnector(pname,linkState,this);
        ppc->setPluginsLinkState(linkState);
        ppc->setPluginsParams(l);
        ppc->setPluginsActive(activated);
        ppc->setPluginsVisible(visible);
        ppc->setPluginsLinkState(linkState);
        ppc->setPluginsStepBufferFlag(sbFlag);

        i++;
        settings.endGroup();
    }
}


/**
 * @brief ProcessingChain::copyFrom copy processing steps from given chain, if possible
 * @param pchain ProcessingChain
 * @throws LIISimException
 */
void ProcessingChain::copyFrom(ProcessingChain *pchain)
{
    if(pchain == NULL)
        throw LIISimException("ProcessingChain: Error while copying processing chain, nullpointer passed!",LIISimMessageType::ERR_NULL);

    if(this->stype != pchain->getSignalType() )
        throw LIISimException("ProcessingChain: Error while copying processing chain, incompatible signal types!",LIISimMessageType::ERR);

    clearAll();

    for(int i = 0; i < pchain->noPlugs(); i++)
    {
        ProcessingPlugin* cur_p = pchain->getPlug(i);
        ProcessingPlugin* p = PluginFactory::createInstance(cur_p->getName(),this);
        p->setActivated(cur_p->activated());
        p->setParameters(cur_p->getInputs());
        this->addPlug(p);
    }
}


QList<ProcessingPluginConnector*> ProcessingChain::getPPCs(bool globalsOnly)
{
    QList<ProcessingPluginConnector*> ppcs;

    for(int i = 0; i < plugs.size(); i++ )
    {
        if(globalsOnly && plugs[i]->linkType() != 2)
            continue;

        if(plugs[i]->ppc())
            ppcs.append(plugs[i]->ppc());
    }

    return ppcs;
}


void ProcessingChain::onPluginDataChanged(int pos = -1, QVariant value = 0)
{
    if((pos == 2 && value.toBool()) || (pos == 1))
    {
        emit pluginGoneDirty();
    }
}

