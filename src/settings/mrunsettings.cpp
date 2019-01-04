#include "mrunsettings.h"

#include <QFileInfo>

#include "../core.h"
#include "../signal/mrun.h"


/**
 * @brief MRunSettings::MRunSettings Constructor
 * @param run measurement run which should be saved (default 0)
 * @param parent parent object
 */
MRunSettings::MRunSettings(MRun* run, QObject *parent) : SettingsBase(parent)
{
    groupName = "MRunSettings";
    this->run = run;
}


/**
 * @brief MRunSettings::MRunSettings load settings from file
 * without association to a certain MRun
 * @param fname filename
 */
MRunSettings::MRunSettings(const QString &fname)
{
    groupName = "MRunSettings";
    run = 0;
    load(fname);
}



/**
 * @brief MRunSettings::~MRunSettings Destructor
 */
MRunSettings::~MRunSettings()
{

}


/**
 * @brief MRunSettings::save Saves run settings to given directory (.ini format)
 * @param dirpath path to directory where settings should be stored
 * @return success or fail
 */
bool MRunSettings::save(const QString &dirpath)
{
    if(!run)
        return false;

    // generate filename from dirpath and run name
    QString fname = QString("%0%1_settings.txt").arg(dirpath).arg(run->filename);

    //QString fname = QString(dirpath).append(run->filename);

    QSettings qs(fname,QSettings::IniFormat);

    if(!qs.isWritable())
    {
        qWarning() << "MRunSettings::save: cannot write settings to " + qs.fileName();
        return false;
    }

    qs.clear();
    qs.beginGroup(groupName);

    qs.setValue("name",run->getName());
    qs.setValue("description",run->description());
    qs.setValue("liisettings",run->liiSettings().filename);
    qs.setValue("filter",run->filterIdentifier());
    qs.setValue("laser_fluence", QString::number( run->laserFluence() ));

    for(int i = 0; i < run->getNoChannels(Signal::RAW);i++)
        qs.setValue(QString("pmt_gain_voltage_channel_%0").arg(i+1), QString::number( run->pmtGainVoltage(i+1) ));

    for(int i = 0; i < run->getNoChannels(Signal::RAW); i++)
        qs.setValue(QString("pmt_measured_gain_voltage_channel_%0").arg(i+1), QString::number(run->pmtReferenceGainVoltage(i+1)));

    //PicoScope parameters
    for(int i = 0; i < run->getNoChannels(Signal::RAW); i++)
        qs.setValue(QString("ps_range_channel_%0").arg(i+1), QString::number(run->psRange(i+1)));

    qs.setValue("ps_coupling", PicoScopeCommon::PSCouplingToInt(run->psCoupling()));

    for(int i = 0; i < run->getNoChannels(Signal::RAW); i++)
        qs.setValue(QString("ps_offset_channel_%0").arg(i+1), QString::number(run->psOffset(i+1)));

    qs.setValue("ps_ct", run->psCollectionTime());
    qs.setValue("ps_si", run->psSampleInterval());
    qs.setValue("ps_presample", run->psPresample());

    qs.setValue("laser_setpoint", run->getLaserSetpoint());
    qs.setValue("laser_position", run->getLaserPosition());

    qs.setValue("acquisition_mode", run->getAcquisitionMode());

    qs.setValue("udp_count", run->userDefinedParameters.size());

    int count = 0;
    for(auto i = run->userDefinedParameters.begin(); i != run->userDefinedParameters.end(); ++i)
    {
        qs.setValue(QString("udp_%0_identifier").arg(count), i.key());
        qs.setValue(QString("udp_%0_value").arg(count), i.value());
        count++;
    }

    qs.endGroup();
    MSG_ASYNC(QString("MRunSettings: saved run settings to %0").arg(fname),DEBUG);
    return true;
}


/**
 * @brief MRunSettings::load loads run settings from file in
 * dirpath directory and updates mrun object
 * @param dirpath path to directory where settings are stored
 * @return success or fail
 */
bool MRunSettings::load(const QString &dirpath)
{
    QString fname;
    if(run)
        fname = QString("%0%1_settings.txt").arg(dirpath).arg(run->filename);
    else
        fname = dirpath;

    //qDebug() << "MRunSettings::load::fname - " << fname;

    QFileInfo fi(fname);
    if(!fi.exists())
    {
        MSG_ASYNC(QString("MRunSettings: failed to load run settings %0").arg(fname),ERR_IO);
        return false;
    }

    QSettings qs(fname,QSettings::IniFormat);

    qs.beginGroup(groupName);
    QStringList keys = qs.allKeys();

    // copy settings to internal data
    for(int i = 0; i < keys.size(); i++)
    {
        settings.insert(keys[i],qs.value(keys[i]));
    }
    if(!run)
        return true;

    // if a MRun object has been associated, load settings to mrun

    QString str = qs.value("name","").toString();
    if(!str.isEmpty())
        run->setName(str, true);

    str = qs.value("liisettings","").toString();
    int idx = Core::instance()->getDatabaseManager()->indexOfLIISettings(str);
    if(idx > -1)
    {
        run->setLiiSettings(*Core::instance()->getDatabaseManager()->getLIISetting(idx), true);
        //qDebug() << "MRunSettings: set run's liisettings" << run->getName() << "->" << run->liiSettings().filename;
    }
    else
    {
        MSG_WARN(QString("MRunSettings::load: cannot assign "
                         "liisettings '%0' to run '%1'! "
                         "Using '%2' instead.")
                 .arg(str)
                 .arg(run->getName())
                 .arg(run->liiSettings().name));
    }

    str = qs.value("description","").toStringList().join(",");
    if(!str.isEmpty())
        //run->setDescription(str);
        run->setData(3, str);

    //qDebug() << str;

    run->setLaserFluence(qs.value("laser_fluence","0.0").toDouble(), true);

    str = qs.value("filter","").toString();

    // workarround
    if(str == "100.0")
        str = "100";

    if(str == "79.0")
        str = "79";

    if(str == "50.0")
        str = "50";

    if(str == "33.0")
        str = "33";

    if(str == "10.0")
        str = "10";

    if(str == "1.0")
        str = "1";

    // change '100' to defaultFilerName ("no Filter")
    if(str == "100")
        str = LIISettings::defaultFilterName;


    if(!str.isEmpty())
        run->setFilter(str, true);

    // handle "filter not set" state
    // This means that no filter settings have been defined during
    // MRunSettings generation (e.g. using Labview)
    // !!! Do not confuse this withh the "no filter", which is
    // a well defined filter (transmission 100%) !!!
    if(str.isEmpty()
        || str.compare("notset",Qt::CaseInsensitive) == 0
        || str.compare("not set",Qt::CaseInsensitive) == 0
        || str.compare(Filter::filterNotSetIdentifier,Qt::CaseInsensitive) == 0)
    {
        run->setFilter(Filter::filterNotSetIdentifier, true);
    }

    int noch = run->getNoChannels(Signal::RAW);
    for(int i = 0; i < noch; i++)
    {
        QString k = QString("pmt_gain_voltage_channel_%0").arg(i+1);
        if(keys.contains(k))
            run->setPmtGainVoltage(i+1,qs.value(k,0.0).toDouble(), true);
    }


    // Backward compatibility fix: load 'pmt_reference_gain_voltage_channel_%0'
    // but don't export it anymore
    for(int i = 0; i < run->getNoChannels(Signal::RAW); i++)
    {
        QString str = QString("pmt_reference_gain_voltage_channel_%0").arg(i+1);
        if(keys.contains(str))
            run->setPmtReferenceGainVoltage(i+1, qs.value(str, 0.0).toDouble());
    }

    for(int i = 0; i < run->getNoChannels(Signal::RAW); i++)
    {
        QString str = QString("pmt_measured_gain_voltage_channel_%0").arg(i+1);
        if(keys.contains(str))
            run->setPmtReferenceGainVoltage(i+1, qs.value(str, 0.0).toDouble());
    }


    //PicoScope parameters
    for(int i = 0; i < run->getNoChannels(Signal::RAW); i++)
    {
        QString str = QString("ps_range_channel_%0").arg(i+1);
        if(keys.contains(str))
            run->setPSRange(i+1, qs.value(str, 0.0).toDouble());
    }

    //if we have an old file with only a single ps_range parameter, we set all range parameters accordingly
    double range = 0.0;
    for(int i = 0; i < run->getNoChannels(Signal::RAW); i++)
    {
        if(run->psRange(i+1) > range)
            range = run->psRange(i+1);
    }

    run->setPSCoupling(PicoScopeCommon::intToPSCoupling(qs.value("ps_coupling", "0").toUInt()));

    for(int i = 0; i < run->getNoChannels(Signal::RAW); i++)
    {
        QString str = QString("ps_offset_channel_%0").arg(i+1);
        if(keys.contains(str))
            run->setPSOffset(i+1, qs.value(str, 0.0).toDouble());
    }

    run->setPSCollectionTime(qs.value("ps_ct", "0").toDouble());
    run->setPSSampleInterval(qs.value("ps_si", "0").toDouble());
    run->setPSPresample(qs.value("ps_presample", "0").toDouble());

    run->setLaserSetpoint(qs.value("laser_setpoint", "0.0").toDouble());
    run->setLaserPosition(qs.value("laser_position", "0.0").toDouble());

    run->setAcquisitionMode(qs.value("acquisition_mode", "Undefined").toString());

    int udp_count = qs.value("udp_count", "0").toInt();
    if(udp_count > 0)
    {
        for(int i = 0; i < udp_count; i++)
        {
            QString identifier = qs.value(QString("udp_%0_identifier").arg(i), QString("udp_%0").arg(i)).toString();
            QVariant value = qs.value(QString("udp_%0_value").arg(i), "-");
            run->userDefinedParameters.insert(identifier, value);
        }
    }

    qs.endGroup();

    MSG_ASYNC(QString("MRunSettings: run settings %0 loaded").arg(fname),DEBUG);

    return true;
}


/**
 * @brief MRunSettings::runName get run name settings entry
 * @return name of measurement
 */
QString MRunSettings::runName()
{
    return settings.value("name","").toString();
}


/**
 * @brief MRunSettings::liiSettingsFname get LIISettings file name settings entry
 * @return file name
 */
QString MRunSettings::liiSettingsFname()
{
    return settings.value("liisettings","").toString();
}


/**
 * @brief MRunSettings::description get description settings entry
 * from settings
 * @return description
 */
QString MRunSettings::description()
{
    return settings.value("description","").toString();
}


/**
 * @brief MRunSettings::filterName name of filter settings entry
 * @return filter name
 */
QString MRunSettings::filterName()
{
    return settings.value("filter","").toString();
}


/**
 * @brief MRunSettings::laserFluence laser fluence settings entry
 * @return laser fluence [mJ/mm^2]
 */
double MRunSettings::laserFluence()
{
    return settings.value("laser_fluence","0.0").toDouble();
}


/**
 * @brief MRunSettings::pmtChannelGainVoltage pmt channel gain voltage
 * settings entry dependent of channel-ID
 * @param channelID channel-ID
 * @return pmg channel gain voltage [V]
 */
double MRunSettings::pmtChannelGainVoltage(int channelID)
{
    QString k = QString("pmt_gain_voltage_channel_%0").arg(channelID);
    return settings.value(k, "0.0").toDouble();
}


//TODO: Docu
PSRange MRunSettings::ps_range()
{
    unsigned int range = settings.value("ps_range", "0").toUInt();
    return PicoScopeCommon::intToPSRange(range);
}


PSCoupling MRunSettings::ps_coupling()
{
    unsigned int coupling = settings.value("ps_coupling", "0").toUInt();
    return PicoScopeCommon::intToPSCoupling(coupling);
}


double MRunSettings::ps_offset()
{
    return settings.value("ps_offset", "0.0").toDouble();
}


double MRunSettings::ps_collectionTime()
{
    return settings.value("ps_ct", "0.0").toDouble();
}


double MRunSettings::ps_sampleInterval()
{
    return settings.value("ps_si", "0.0").toDouble();
}


double MRunSettings::ps_presample()
{
    return settings.value("ps_presample", "0.0").toDouble();
}

double MRunSettings::laser_setpoint()
{
    return settings.value("laser_setpoint", "0.0").toDouble();
}

double MRunSettings::laser_position()
{
    return settings.value("laser_position", "0.0").toDouble();
}

QString MRunSettings::acquisition_mode()
{
    return settings.value("acquisition_mode", "Undefined").toString();
}




