#include "fitsettings.h"

#include "../logging/msghandlerbase.h"

FitSettings::FitSettings(QObject *parent) : SettingsBase(parent)
{
    MSG_DETAIL_1("init FitSettings");
    groupName = "FitSettings";

    m_fitParameters = availableFitParameters();

    // FitMode::TEMP settings
    m_bp_integration = false;
    m_weighting = false;

    // m_sourceEm should always be defined where it is used
    // (if not error is thrown in Temperature::checkEmSource();
    m_sourceEm = "";

    init();
}


FitSettings::~FitSettings()
{
   // qDebug() << "~FitSettings() ";
}


/**
 * @brief FitSettings::availableFitParameters returns a list of supported
 * FitParameters for GUI
 * @return
 */
QList<FitParameter> FitSettings::availableFitParameters()
{
    QList<FitParameter> params;    
    params << FitParameter(0, "Particle diameter", "nm", 20.0, 1.0, 150.0, 20.0);
    params << FitParameter(1, "Gas temperature", "K", 1500.0, 300.0, 5000.0, 500.0);
    params << FitParameter(2, "Peak temperature", "K", 2500.0, 300.0, 5000.0, 100.0);

    return params;
}


void FitSettings::init()
{    
    // initialize fit parameters
    QStringList keys = settings.keys();
    for(int i = 0; i < keys.size(); i++)
    {
        QString key = keys[i];

        // find Fitparameter entries
        if(key.startsWith("FitParameter"))
        {
            QStringList strl = key.split('-');
            if(strl.length() < 2)
                continue;
            int paramident = strl.last().toInt();

            // try to find matching parameter in list
            bool found = false;
            for(int j = 0; j < m_fitParameters.size(); j++)
            {
                if(m_fitParameters[j].ident() == paramident)
                {
                    found = true;
                    m_fitParameters[j].readFrom(settings);
                    break;
                }
            }
            if(!found) // or create a new one
                m_fitParameters.append(FitParameter(paramident,settings));
        }
    }
}


void FitSettings::setFitParameters(QList<FitParameter> &fparams)
{
    m_fitParameters = fparams;
    for(int i = 0; i < m_fitParameters.size();i++)
        m_fitParameters[i].writeTo(settings);
    emit settingsChanged();
}


int FitSettings::getNoEnabledFitParameters()
{
    int k = 0;
    for(int i = 0; i < m_fitParameters.size(); i++)
        if(m_fitParameters.at(i).enabled()) k++;

    return k;
}


void FitSettings::setBandpassIntegrationActive(bool active)
{
    m_bp_integration = active;
}


void FitSettings::setWeightingActive(bool active)
{
    m_weighting = active;
}


void FitSettings::setSourceEm(QString sourceEm)
{
    m_sourceEm = sourceEm;
}


void FitSettings::write(QSettings &dest)
{
    for(int i = 0; i < m_fitParameters.size();i++)
        m_fitParameters[i].writeTo(settings);
    //TODO: write fitparams to settings
    SettingsBase::write(dest);
}


void FitSettings::writeToXML(QXmlStreamWriter &w)
{
    for(int i = 0; i < m_fitParameters.size();i++)
        m_fitParameters[i].writeTo(settings);

    SettingsBase::writeToXML(w);
}


QString FitSettings::toString()
{
    QString res;

    res.append(groupName+"\n");    
    for(int i = 0; i < m_fitParameters.size(); i++)
    {
        res.append(QString("\n\t%0").arg(m_fitParameters[i].toString()));
    }
    return res;
}

