#include "mrun.h"
#include <QDebug>
#include <QMutexLocker>
#include "../general/LIISimException.h"
#include "processing/processingchain.h"
#include "processing/temperatureprocessingchain.h"
#include "../core.h"
#include "mrungroup.h"
#include "processing/processingpluginconnector.h"

/**
 * @brief MRun::MRun Constructor
 * @param name name of Measurement Run
 * @param channelCount_raw_abs number of channels for raw/absolute signals
 */
MRun::MRun(QString name, int channelCount_raw_abs, MRunGroup *group)
    : DataItem(group)
{    
    this->name = name;
    filename = name.append("_settings.txt");
    setData(0,name);
    setData(1,QColor(0,0,255)); // blue
    setData(2,0);
    setData(3,"no description");

    this->channelCount_raw_abs = channelCount_raw_abs;
    busy = false;

    pchainRaw = new ProcessingChain(this, Signal::RAW);
    pchainAbs = new ProcessingChain(this, Signal::ABS);

    pchainTemp = new TemperatureProcessingChain(this);

    connect(pchainRaw, SIGNAL(pluginGoneDirty()), SLOT(onPluginGoneDirty()));
    connect(pchainAbs, SIGNAL(pluginGoneDirty()), SLOT(onPluginGoneDirty()));

    m_calcState = new MRunCalculationStatus(this);

    insertChild(pchainRaw);
    insertChild(pchainAbs);
    insertChild(pchainTemp);

    // !!!
    m_liiSettings = Core::instance()->modelingSettings->defaultLiiSettings();
    connect(Core::instance()->getDatabaseManager(),
            SIGNAL(signal_contentChanged(int)),
            SLOT(onDBmodified(int)));

    setData(4,m_liiSettings.name);

    setLaserFluence(0.0);

    m_pmtGainVoltage.fill(0.0, channelCount_raw_abs);
    for(int i = 0; i < channelCount_raw_abs; i++)
        setPmtGainVoltage(i+1,0.0);

    m_pmtReferenceGainVoltage.fill(0.0, channelCount_raw_abs);
    for(int i = 0; i < channelCount_raw_abs; i++)
        setPmtReferenceGainVoltage(i+1, 0.0);

    m_filterSetting = LIISettings::defaultFilterName;

    //Set PicoScope default parameters
    m_psRange.fill(0.0, channelCount_raw_abs);
    for(int i = 0; i < channelCount_raw_abs; i++)
        setPSRange(i+1, 0.0);

    ps_range = PSRange::NONE;
    ps_coupling = PSCoupling::NONE;

    m_psOffset.fill(0.0, channelCount_raw_abs);
    for(int i = 0; i < channelCount_raw_abs; i++)
        setPSOffset(i+1, 0.0);

    ps_offset = 0;
    ps_collectionTime = 0;
    ps_sampleInterval = 0;
    ps_presample = 0;

    laserPosition = 0;
    laserSetpoint = 0;


    if(group)
    {
        // add run to given group
        group->insertChild(this);
        // register run to global datamodel
        if(Core::instance())
            Core::instance()->dataModel()->registerMRun(this);
    }
}


MRun::MRun(QString name, QString filename, int channelCount_raw_abs, MRunGroup *group) : DataItem(group)
{
    this->name = name;
    if(filename.isEmpty())
        this->filename = name;
    else
        this->filename = filename;
    setData(0,name);
    setData(1,QColor(0,0,255)); // blue
    setData(2,0);
    setData(3,"no description");

    this->channelCount_raw_abs = channelCount_raw_abs;
    busy = false;

    pchainRaw = new ProcessingChain(this, Signal::RAW);
    pchainAbs = new ProcessingChain(this, Signal::ABS);

    pchainTemp = new TemperatureProcessingChain(this);

    connect(pchainRaw, SIGNAL(pluginGoneDirty()), SLOT(onPluginGoneDirty()));
    connect(pchainAbs, SIGNAL(pluginGoneDirty()), SLOT(onPluginGoneDirty()));

    m_calcState = new MRunCalculationStatus(this);

    insertChild(pchainRaw);
    insertChild(pchainAbs);
    insertChild(pchainTemp);

    // !!!
    m_liiSettings = Core::instance()->modelingSettings->defaultLiiSettings();
    connect(Core::instance()->getDatabaseManager(),
            SIGNAL(signal_contentChanged(int)),
            SLOT(onDBmodified(int)));

    setData(4,m_liiSettings.name);

    setLaserFluence(0.0);

    m_pmtGainVoltage.fill(0.0, channelCount_raw_abs);
    for(int i = 0; i < channelCount_raw_abs; i++)
        setPmtGainVoltage(i+1,0.0);

    m_pmtReferenceGainVoltage.fill(0.0, channelCount_raw_abs);
    for(int i = 0; i < channelCount_raw_abs; i++)
        setPmtReferenceGainVoltage(i+1, 0.0);

    m_filterSetting = LIISettings::defaultFilterName;

    //Set PicoScope default parameters
    m_psRange.fill(0.0, channelCount_raw_abs);
    for(int i = 0; i < channelCount_raw_abs; i++)
        setPSRange(i+1, 0.0);

    ps_range = PSRange::NONE;
    ps_coupling = PSCoupling::NONE;

    m_psOffset.fill(0.0, channelCount_raw_abs);
    for(int i = 0; i < channelCount_raw_abs; i++)
        setPSOffset(i+1, 0.0);

    ps_offset = 0;
    ps_collectionTime = 0;
    ps_sampleInterval = 0;
    ps_presample = 0;

    laserPosition = 0;
    laserSetpoint = 0;

    if(group)
    {
        // add run to given group
        group->insertChild(this);
        // register run to global datamodel
        if(Core::instance())
            Core::instance()->dataModel()->registerMRun(this);
    }
}


/**
 * @brief MRun::~MRun Destructor
 * @details deletes all signal data
 */
MRun::~MRun()
{
    QList<ProcessingPluginConnector*> mppcs = ppcs();
    for(int i = 0; i < mppcs.size(); i++)
    {
        mppcs[i]->disconnectMRun(this);
    }

    QMutexLocker lock1(&mutexMPointLists);

    while(!pre.isEmpty())
    {
        MPoint* p = pre.first();
        delete p;
        pre.pop_front();
    }

    while(!post.isEmpty())
    {
        MPoint* p = post.first();
        delete p;
        post.pop_front();
    }


   if(pchainRaw != NULL)
        delete pchainRaw;

    if(pchainAbs != NULL)
        delete pchainAbs;

    if(pchainTemp != NULL)
        delete pchainTemp;

    QString msg = "Measurement run closed: "+m_data[0].toString();
    MSG_STATUS(msg);
    MSG_NORMAL(msg);
}


/**
 * @brief MRun::group get parent group of measurement run
 * @return pointer to parent group or null
 */
MRunGroup* MRun::group()
{
    if(!parentItem())return 0;
    return Core::instance()->dataModel()->group(parentItem()->id());
}

/**
 * @brief MRun::importRequest returns a SignalIORequest which can be used
 * to "recreate" this mrun (including naming, description, groups, etc.)
 * @return SignalIORequest
 */
SignalIORequest MRun::importRequest()
{
    //update some fields
    m_importRequest.runname = this->name;
    m_importRequest.description = this->description();
    m_importRequest.group_id = this->parentItem()->id();
    m_importRequest.userData.insert(3,m_liiSettings.filename);
    return m_importRequest;
}

/**
 * @brief MRun::setImportRequest stores the given SignalIORequest to MRun
 * @param irq
 */
void MRun::setImportRequest(const SignalIORequest &irq)
{
    m_importRequest = irq;

    // import informations changed (see also "mrun.h")
    emit dataChanged(8);
}


/**
 * @brief MRun::getName
 * @return name of Measurement Run
 */
QString MRun::getName()
{
    return name;
}

void MRun::setName(const QString &name)
{
    this->name = name;
    setData(0,name);
    emit MRunDetailsChanged();
}


/**
 * @brief MRun::description get short discription text.
 * The description text is created optionally by user for each MRun individually
 * @return description text
 */
QString MRun::description()
{
    return data(3).toString();
}

/**
 * @brief MRun::setDescription set description text of MRun
 * @param descr
 */
void MRun::setDescription(const QString &descr)
{
    setData(3,descr);
    emit MRunDetailsChanged();
}


/**
 * @brief MRun::laserFluence get laser fluence
 * @return laser fluence [mJ/mm^2]
 */
double MRun::laserFluence()
{
    return m_laserFluence;
}


/**
 * @brief MRun::setLaserFluence set laser fluence
 * @param laserFluence [mJ/mm^2]
 */
void MRun::setLaserFluence(double laserFluence)
{
    m_laserFluence = laserFluence;

    // also set a filed in data vector
    // -> dataChanged signal will be emitted.
    setData(5, laserFluence);
    emit MRunDetailsChanged();
}


/**
 * @brief MRun::pmtGainVoltage get PMT channel gain voltage (for raw or
 * absolute channels only!)
 * @param channelID channelID
 * @return PMT gain voltage for certain channelID
 */
double MRun::pmtGainVoltage(int channelID)
{
    if(channelID < 1 || channelID > m_pmtGainVoltage.size())
    {
        MSG_ERR(QString("MRun::pmtGainVoltage invalid channel id %0").arg(channelID));
        return 0.0;
    }
    return m_pmtGainVoltage.value(channelID-1,0.0);
}


/**
 * @brief MRun::setPmtGainVoltage set PMT channel gain voltage
 * TODO: check valid voltage range?
 * TODO: voltage unit?
 * @param channelID
 * @param voltage
 */
void MRun::setPmtGainVoltage(int channelID, double voltage)
{
    if(channelID < 1 || channelID > m_pmtGainVoltage.size())
    {
        MSG_ERR(QString("MRun::setPmtGainVoltage invalid channel id %0").arg(channelID));
        return;
    }
    m_pmtGainVoltage[channelID-1] = voltage;

    // also set a filed in data vector
    // -> dataChanged signal will be emitted.
    setData(6, QVariant::fromValue(m_pmtGainVoltage));
    //qDebug() << "MRun::setPmtGainVoltage" <<data(6).value<QVector<double>>();
    emit MRunDetailsChanged();
}


/**
 * @brief MRun::pmtGainVoltageReference Get PMT measured gain voltage
 * @param channelID Channel ID
 * @return PMT reference gain voltage for certain channelID
 */
double MRun::pmtReferenceGainVoltage(int channelID)
{
    if(channelID < 1 || channelID > m_pmtReferenceGainVoltage.size())
    {
        MSG_ERR(QString("MRun::pmtGainVoltageReference invalid channel id %0").arg(channelID));
        return 0.0;
    }
    return m_pmtReferenceGainVoltage.value(channelID-1, 0.0);
}


/**
 * @brief MRun::setPmtGainVoltageReference Set PMT measured gain voltage
 * @param channelID Channel ID
 * @param voltage PMT reference gain voltage
 */
void MRun::setPmtReferenceGainVoltage(int channelID, double voltage)
{
    if(channelID < 1 || channelID > m_pmtReferenceGainVoltage.size())
    {
        MSG_ERR(QString("MRun::pmtGainVoltageReference invalid channel id %0").arg(channelID));
        return;
    }
    m_pmtReferenceGainVoltage[channelID-1] = voltage;
}


double MRun::psRange(int channelID)
{
    if(channelID < 1 || channelID > m_psRange.size())
    {
        MSG_ERR(QString("MRun::psRange invalid channel id %0").arg(channelID));
        return 0.0;
    }
    return m_psRange.value(channelID-1, 0.0);
}

void MRun::setPSRange(int channelID, double range)
{
    if(channelID < 1 || channelID > m_psRange.size())
    {
        MSG_ERR(QString("MRun::setPSRange invalid channel id %0").arg(channelID));
        return;
    }
    m_psRange[channelID-1] = range;
}


double MRun::psOffset(int channelID)
{
    if(channelID < 1 || channelID > m_psOffset.size())
    {
        MSG_ERR(QString("MRun::psOffset invalid channel id %0").arg(channelID));
        return 0.0;
    }
    return m_psOffset.value(channelID-1, 0.0);
}

void MRun::setPSOffset(int channelID, double offset)
{
    if(channelID < 1 || channelID > m_psOffset.size())
    {
        MSG_ERR(QString("MRun::setPSOffset invalid channel id %0").arg(channelID));
        return;
    }
    m_psOffset[channelID-1] = offset;
}




/**
 * @brief MRun::filterIdentifier
 * @return filter identifier
 */
QString MRun::filterIdentifier()
{
    return m_filterSetting;
}


/**
 * @brief MRun::filter
 * @return current Filter Object from LIISettings
 */
Filter MRun::filter()
{
    QMutexLocker lock(&settingsMutex);
    return m_liiSettings.filter(m_filterSetting);
}

LIISettings MRun::liiSettings()
{
    QMutexLocker lock(&settingsMutex);
    return m_liiSettings;
}


/**
 * @brief MRun::setFilter set current filter by identifier
 * @param identifier
 * @return true if given filter identifier exists (in current LIISettings)
 */
bool MRun::setFilter(const QString &identifier)
{
   QMutexLocker lock(&settingsMutex);

    // get filter from current liisettings
    // LIISettings.filter() returns a default 'no Filter' when
    // the filter with given identifier cannot be fount.
    m_filterSetting = m_liiSettings.filter(identifier).identifier;
    lock.unlock();

    // also set a field in data vector
    // -> dataChanged signal will be emitted.
    setData(7,m_filterSetting);
    emit MRunDetailsChanged();

    return true;
}


/**
 * @brief MRun::setLiiSettings set LIISettings of MRun
 * @param liiSettings
 */
void MRun::setLiiSettings(const LIISettings &liiSettings)
{
    QMutexLocker lock(&settingsMutex);
    QString settingsName = m_liiSettings.name;

    if(liiSettings.channels.size() != channelCount_raw_abs)
    {
        QString message = QString("%0 - Cannot change LIISettings to %1, invalid number of channels!").arg(name).arg(liiSettings.name);
        //MSG_WARN("Cannot change LIISettings, invalid number of channels!");
        //MSG_STATUS("Cannot change LIISettings, invalid number of channels!");
        MSG_WARN(message);
        MSG_ASYNC(message, LIISimMessageType::STATUS_5000);

        lock.unlock();
        setData(4,settingsName);
        emit LIISettingsChanged();
        return;
    }

    m_liiSettings = liiSettings;
    settingsName = m_liiSettings.name;

    lock.unlock();
    setData(4,settingsName);
    emit LIISettingsChanged();

    // if a filter has been set, try to reset it. This
    // includes a check if the current filter setting is available in new LIISettings.
    if(!m_filterSetting.isEmpty())
        setFilter(m_filterSetting);

     emit MRunDetailsChanged();
}


/**
 * @brief MRun::getNoChannels
 * @return number of channels of Measurement Run
 */
int MRun::getNoChannels(Signal::SType stype)
{
    if(stype == Signal::TEMPERATURE)
        return temperatureChannelIDs.size();

    return channelCount_raw_abs;
}


/**
 * @brief MRun::addTemperatureChannel adds an additional temperature channel
 * to this MRun.
 * @return channel-ID of new channel
 */
int MRun::addTemperatureChannel(int tchid)
{
    if(tchid < 0)
        return -1;

    temperatureChannelIDs << tchid;
    for(int i = 0; i < pre.size(); i++)
    {
        pre[i]->addTemperatureChannel(tchid);
        post[i]->addTemperatureChannel(tchid);
    }

    emit channelCountChanged(Signal::TEMPERATURE,
                             getNoChannels(Signal::TEMPERATURE));
    return tchid;
}


void MRun::removeTemperatureChannel(int ch_id)
{
    temperatureChannelIDs.removeOne(ch_id);
    for(int i = 0; i < pre.size(); i++)
    {
        pre[i]->removeTemperatureChannel(ch_id);
        post[i]->removeTemperatureChannel(ch_id);
    }
    emit channelCountChanged(Signal::TEMPERATURE,
                             getNoChannels(Signal::TEMPERATURE));
}


/**
 * @brief MRun::channelIDs
 * @param stype
 * @return
 */
QList<int> MRun::channelIDs(Signal::SType stype)
{
    // return channel count of random MPoint (channel count
    // is equal for all MPoints!)
    if(!post.isEmpty())
        return post[0]->channelIDs(stype);

    QList<int> res;
    return res;
}


/**
 * @brief MRun::isValidChannelID checks if a channel id is valid
 * @param id
 * @param stype
 * @return
 */
bool MRun::isValidChannelID(int id, Signal::SType stype)
{
    bool res = true;
    if(post.isEmpty())
        return false;

    res = post[0]->isValidChannelID(id,stype,false);
    return res;
}



/**
 * @brief MRun::getProcessingChain get Processing chain for given signal type
 * @param stype signal type
 * @return ProcessingChain of MRun
 */
ProcessingChain* MRun::getProcessingChain(Signal::SType stype)
{
    switch(stype)
    {
    case Signal::RAW:
        return pchainRaw;
    case Signal::ABS:
        return pchainAbs;
    case Signal::TEMPERATURE:
        return pchainTemp;
    default:
        return 0;
    }
}


/**
 * @brief MRun::getPre gets or creates a new MPoint object
 * @param idx index of requested MPoint
 * @return Mpoint from unprocessed data list or new MPoint
 * @details This method returns the i-th element of the unprocessed
 * signal data list if the requested index is valid. If the i-th element
 * is equal to the size of the data list a new MPoint is added to the list.
 * Otherwise an exception is trhown!
 */
MPoint * MRun::getCreatePre(int idx)
{
    QMutexLocker lock(&mutexMPointLists);

    // return valid MPoint
    if(idx < pre.size())
        return pre.at(idx);

    // create new MPoint
     if(idx == pre.size())
    {
        MPoint* mp = new MPoint(channelCount_raw_abs);
        pre.push_back(mp);

        MPoint* mpPost = new MPoint(channelCount_raw_abs);
        post.push_back(mpPost);


        for(int t = 0; t < temperatureChannelIDs.size(); t++)
        {
            mp->addTemperatureChannel(temperatureChannelIDs[t]);
            mpPost->addTemperatureChannel(temperatureChannelIDs[t]);
        }

        validPointsIdx.push_back(idx);

        return pre.at(idx);
    }

    QString msg;
    msg.sprintf("MRrun:getCreatePre(): requested invalid index from unprocessed data: %d", idx);
    throw LIISimException(msg,ERR_NULL);
}


/**
 * @brief get unprocessed data (start of processing chain)
 * @param idx index
 * @return Measurement Point at given index
 * @details throws LIISimException if index is invalid!
 */
MPoint* MRun::getPre(int idx)
{
    QMutexLocker lock(&mutexMPointLists);
    if(!pre.isEmpty() && idx < pre.size() && idx >= 0)
        return pre.at(idx);

    QString msg;
    msg.sprintf("MRrun::getPre(): requested invalid index from unprocessed data: %d", idx);
    throw LIISimException(msg,ERR_NULL);
}


/**
 * @brief get processed MPoints (end of processing chain)
 * @param idx index
 * @return Measurement Point at given index
 * @details throws LIISimException if index is invalid!
 */
MPoint* MRun::getPost(int idx)
{
    QMutexLocker lock(&mutexMPointLists);
    if(!post.isEmpty() && idx < post.size()  && idx>=0)
        return post.at(idx);

    QString msg;
    msg.sprintf("MRrun: requested invalid index from processed data: %d", idx);
    throw LIISimException(msg,ERR_NULL);
}


/**
 * @brief MRun::getPostPre Returns the post-MPoint at idx, if available,
 * otherwise returns the pre-MPoint at idx. If idx is invalid an exception
 * is thrown.
 * @param idx MPoint index
 * @return
 */
MPoint* MRun::getPostPre(int idx)
{
    QMutexLocker lock(&mutexMPointLists);

    if(!post.isEmpty() && idx < post.size()  && idx>=0)
        return post.at(idx);

    if(!pre.isEmpty() && idx < pre.size() && idx >= 0)
        return pre.at(idx);

    QString msg("MRun::getPostPre: requested invalid index from processed data: %0");
    msg.arg(idx);
    throw LIISimException(msg, ERR_NULL);
}


/**
 * @brief get Number of all loaded Measurement Points
 * @return number of Measurement Points
 */
int MRun::sizeAllMpoints()
{
    QMutexLocker lock(&mutexMPointLists);
    if(pre.isEmpty())
        return 0;
    return pre.size();
}


/**
 * @brief get valid, unprocessed Measurement Point
 * @param idx index
 * @return validated Measurement Point
 */
MPoint* MRun::getValidPre(int idx)
{
    QMutexLocker lock(&mutexMPointLists);
    if(!validPointsIdx.isEmpty() && idx < validPointsIdx.size()  && idx >= 0)
        return pre.at(validPointsIdx.at(idx));

    QString msg;
    msg.sprintf("MRrun::getValidPre() requested invalid index from validated, unprocessed data: %d", idx);
    throw LIISimException(msg,ERR_NULL);
}


/**
 * @brief get valid, processed Measurement Point
 * @param idx index
 * @return validated Measurement Point after preprocessing
 */
MPoint* MRun::getValidPost(int idx)
{
    QMutexLocker lock(&mutexMPointLists);
    if(!validPointsIdx.isEmpty() && idx < validPointsIdx.size()  && idx >= 0)
        return post.at(validPointsIdx.at(idx));

    QString msg;
    msg.sprintf("MRrun::getValidPost() requested invalid index from validated, processed data: %d", idx);
    throw LIISimException(msg,ERR_NULL);
}


MPoint* MRun::getValidPostPre(int idx)
{
    QMutexLocker lock(&mutexMPointLists);

    if(!validPointsIdx.isEmpty() && idx < validPointsIdx.size()  && idx >= 0)
        return post.at(validPointsIdx.at(idx));

    if(!validPointsIdx.isEmpty() && idx < validPointsIdx.size()  && idx >= 0)
        return pre.at(validPointsIdx.at(idx));

    QString msg("MRun::getValidPostPre(): requested invalid index from validated, unprocessed data: %0");
    msg.arg(idx);
    throw LIISimException(msg, ERR_NULL);
}


/**
 * @brief MRun::getRealIdxFromValidList
 * @param validListIdx index in valid mpoint list
 * @return index in data mpoint lsit
 */
int MRun::getRealIdxFromValidList(int validListIdx)
{
    QMutexLocker lock(&mutexMPointLists);
    if( !validPointsIdx.isEmpty() && validListIdx < validPointsIdx.size()  && validListIdx>=0)
        return validPointsIdx.at(validListIdx);

    QString msg;
    msg.sprintf("MRrun::getRealIdxFromValidList() requested invalid index from validated, processed data: %d", validListIdx);
    throw LIISimException(msg,ERR_NULL);
}


/**
 * @brief number of valid Measurement Points
 * @return
 */
int MRun::sizeValidMpoints()
{
    //QMutexLocker lock(&mutexMPointLists);
    if(validPointsIdx.isEmpty()) return 0;

    return validPointsIdx.size();
}


/**
 * @brief MRun::updateValidList update valid point list from both processing chains
 */
void MRun::updateValidList()
{
    //qDebug() << "MRun::updateValidList";
    //QMutexLocker lock(&mutexMPointLists);

    validPointsIdx.clear();

    if(pre.isEmpty())return;
    int noMpts = pre.size();

    for(int i=0; i < noMpts; i++)
    {
        bool valid1 = pchainRaw->isValid(i);
        bool valid2 = pchainAbs->isValid(i);
        bool valid3 = pchainTemp->isValid(i);

        pre.at(i)->rawValid = valid1;
        pre.at(i)->absValid = valid2;
        pre.at(i)->tempValid = valid3;

//        qDebug() << "MRun::updateValidList:   mpidx " << i
//                 << " valids: " << valid1 << valid2 << valid3;

        if(valid1 || valid2 || valid3)
        {
           validPointsIdx.push_back(i);
        }
    }
}

void MRun::copyProcessingStepsFrom(MRun *mrun)
{
    if(isBusy())
    {
        qDebug() << "MRun::copyProcessingStepsFrom failed (mrun is busy!)";
        return;
    }

    pchainRaw->copyFrom(mrun->pchainRaw);
    pchainAbs->copyFrom(mrun->pchainAbs);
    pchainTemp->copyFrom(mrun->pchainTemp);
}

QList<ProcessingPluginConnector*> MRun::ppcs(bool globalsOnly)
{
    QList<ProcessingPluginConnector*> ppcs;
    ppcs.append( pchainRaw->getPPCs(globalsOnly) );
    ppcs.append( pchainAbs->getPPCs(globalsOnly)  );
    ppcs.append( pchainTemp->getPPCs(globalsOnly)  );
    return ppcs;
}




/**
 * @brief MRun::onDBmodified This slot is executed if the user
 * has edited any DatabaseContent
 * @param id
 */
void MRun::onDBmodified(int id)
{
    DatabaseManager* dbm = Core::instance()->getDatabaseManager();
    // qDebug() << "MRun onDBModified";
    // db has been modified (eg rescan...)
    if(id == -1 )
    {
        int index = dbm->indexOfLIISettings(m_liiSettings.filename);
        if(index < 0)
        {
           // current liisettings have been deleted, use default settings instead!
           m_liiSettings = Core::instance()->modelingSettings->defaultLiiSettings();
        }
        else
           m_liiSettings = *dbm->getLIISetting(index);

        // TODO: check if settings are still valid for this mrun!
        setData(4,m_liiSettings.name);

        // if a filter has been set, try to reset it. This
        // includes a check if the current filter setting is available in new LIISettings.
        if(!m_filterSetting.isEmpty())
            setFilter(m_filterSetting);

        // TODO: should be replaced by emit dataChanged()
        emit LIISettingsChanged();
        emit MRunDetailsChanged();
        return;
    }

    // update settings if modified
    if(id == m_liiSettings.ident)
    {
        LIISettings* ls = dbm->liiSetting(m_liiSettings.ident);
        if(!ls)return;
        m_liiSettings = *ls;

        // TODO: check if settings are still valid for this mrun!
        setData(4,m_liiSettings.name);

        // if a filter has been set, try to reset it. This
        // includes a check if the current filter setting is available in new LIISettings.
        if(!m_filterSetting.isEmpty())
            setFilter(m_filterSetting);

        // TODO: should be replaced by emit dataChanged()
        emit MRunDetailsChanged();
        emit LIISettingsChanged();
    }
}


/**
 * @brief MRun::updateCalculationStatus, executed before signals of this run are
 * processed. This method should include tests to make sure that all run
 * run properties (eg. LiiSettings, Filters, etc.) are consistent.
 */
void MRun::updateCalculationStatus()
{
    // reset old calculation status
    m_calcState->reset();

    // TODO: check LII/Filters etc...

    // check if the filter has been set
    if(m_filterSetting == Filter::filterNotSetIdentifier)
        calculationStatus()->addWarining("Filter not set in run settings!");

    // check if current filter is defined in current LIISettings
    else if(!m_liiSettings.isFilterDefined(m_filterSetting))
        calculationStatus()->addWarining(QString("Filter '%0' not found in LiiSettings '%1'").arg(m_filterSetting).arg(liiSettings().name));

    // m_calcState->addWarining("Yep, This is a warning!");
    // m_calcState->addError("Oo, Error during MRun::updateCalculationStatus!");
}


void MRun::onPluginGoneDirty()
{
    m_calcState->setPluginChanged();
}
