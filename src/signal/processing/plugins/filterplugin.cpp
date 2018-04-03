#include "filterplugin.h"

#include "../../../core.h"
#include "../../mrun.h"
#include <QDebug>

QString FilterPlugin::descriptionFileName = "filter.html"; // TODO
QString FilterPlugin::iconFileName = "iconfile"; // TODO
QString FilterPlugin::pluginName = "ND-Filter correction";

QList<Signal::SType> FilterPlugin::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW << Signal::ABS;


FilterPlugin::FilterPlugin(ProcessingChain *parentChain) : ProcessingPlugin(parentChain)
{
    shortDescription = "Apply ND-filter correction on each datapoint\n\
    1) Filter transmission values are stored in LIISettings and can be retrieved by an identifier\n\
    2) These values are then used on each channel for correction";

    // default values
    identifier   = "100";
    copyFromRun = true;

    // pass stdev to next processing step
    // new stdev is calculated within processSignalImplementation()
    preserveStdev = true;

    ProcessingPluginInput cbFilterSource;
    cbFilterSource.type = ProcessingPluginInput::COMBOBOX;
    cbFilterSource.labelText = "Filter value source";
    cbFilterSource.identifier = "cbFilterSource";
    cbFilterSource.value = "copy from run settings;copy from run settings;set manually";
    cbFilterSource.tooltip = "Retrieve filter identifier automatically from MRunSettings or set manually";
    inputs << cbFilterSource;

    onMRunLIISettingsChanged();
    connect(mrun,
            SIGNAL(LIISettingsChanged()),
            SLOT(onMRunLIISettingsChanged()));
    connect(mrun,
            SIGNAL(MRunDetailsChanged()),
            SLOT(onMRunLIISettingsChanged()));
}


/**
 * @brief FilterPlugin::onMRunLIISettingsChanged This slot
 * is executed when the LIISettings of mrun have been modified.
 */
void FilterPlugin::onMRunLIISettingsChanged()
{

    // remember old filter value selection
    QString lastFilterIdent = inputs.getValue("cbIdentifier").toString();

    // remove old input field ...
    for(int i = 0; i < inputs.size(); i++)
        if(inputs[i].identifier == "cbIdentifier")
        {
            inputs.removeAt(i);
            break;
        }

    // create new input field
    ProcessingPluginInput cbIdentifier;
    cbIdentifier.type = ProcessingPluginInput::COMBOBOX;
    cbIdentifier.labelText = "Filter identifier";
    cbIdentifier.identifier = "cbIdentifier";
    cbIdentifier.enabled = !copyFromRun;
    cbIdentifier.tooltip = "Select filter identifier (available identifiers for current LIISettings are shown)";

    LIISettings curSettings = mrun->liiSettings();
    QString res;

    // check if filter value should be copied from run
    if(copyFromRun)
        res.append(mrun->filter().identifier);
    else
        res.append(lastFilterIdent);

    for(int i=0; i < curSettings.filters.size(); i++)
        res.append(";").append(curSettings.filters[i].identifier);

    cbIdentifier.value = res;
    inputs.insert(1,cbIdentifier); // add value to input list

    setFromInputs();
    emit dataChanged(7); // notify gui
}


/**
 * @brief FilterPlugin::getName implementation of virtual function
 * @return name of plugin
 */
QString FilterPlugin::getName()
{
    return pluginName;
}

void FilterPlugin::setFromInputs()
{
    QString filterSourceStr = inputs.getValue("cbFilterSource" ).toString();

    bool copyFromRun_new = true;
    if(filterSourceStr == "set manually")
        copyFromRun_new = false;

    // check if Filter Value Source has changed
    if(copyFromRun != copyFromRun_new)
    {
        // check if filter value should be copied from run settings
        if(copyFromRun_new)
        {
            QString value;
            // add current value
            value.append(mrun->filter().identifier);
            LIISettings curSettings = mrun->liiSettings();
            for(size_t i=0; i < curSettings.filters.size(); i++)
            {
                value.append(";"+curSettings.filters.at(i).identifier);
            }
            inputs.setValue("cbIdentifier",value);
            inputs.setEnabled("cbIdentifier",false);
        }
        else
        {
            inputs.setEnabled("cbIdentifier",true);
        }
        // notify gui
        emit dataChanged(7);
    }

    copyFromRun = copyFromRun_new;
    identifier   = inputs.getValue("cbIdentifier").toString();

    // check if new identifier value is valid
    bool lastFilterFound = false;
    LIISettings curSettings = mrun->liiSettings();
    for(int i=0; i < curSettings.filters.size(); i++)
        if(curSettings.filters[i].identifier == identifier)
        {
            lastFilterFound = true;
            break;
        }

    // if invalid filter -> set default filter
    if(!lastFilterFound)
        identifier = LIISettings::defaultFilterName;
}


bool FilterPlugin::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{    
    // throw an error if the run's filter has not been set
    if(mrun->filterIdentifier() == Filter::filterNotSetIdentifier)
        throw LIISimException("FilterPlugin: Cannot process signal data (filter not set)!",ERR_CALC);

    // throw an error if the run's filter is not defined in liisettings
    if(!mrun->liiSettings().isFilterDefined(mrun->filterIdentifier()))
        throw LIISimException("FilterPlugin: Filter not defined in LIISettings!",ERR_CALC);


    int noPts = in.data.size();

    LIISettings curSettings = mrun->liiSettings();
    Channel curChannel = curSettings.channels.at(in.channelID-1);

    // do nothing if no filters available or 100% setting
    if(curSettings.filters.size() == 0)
    {
        out = in;
        return true;
    }

    double filter_transmission;

    // find filter by identifier (transmission)
    Filter curFilter = curSettings.filter(identifier);
    filter_transmission = curFilter.getTransmission(curChannel.wavelength);

    // qDebug() << "FilterPlugin: " << QString::number(curFilter.getTransmission(curChannel.wavelength));

    if(filter_transmission == 0.0)
    {
        out = in;
        throw LIISimException(QString("FilterPlugin: Invalid "
                                      "filter transmission value %0")
                              .arg(filter_transmission));
    }

    double v;
    // use Calibration operation on every datapoint
    for(int i = 0; i < noPts; i++)
    {
        v = in.data.at(i) / filter_transmission;
        out.data.append(v);
    }

    //calculate stdev only if available
    if(!in.stdev.isEmpty())
    {
        noPts = in.stdev.size();
        double v;

        for(int i = 0; i < noPts; i++)
        {
            v = in.stdev.at(i) / filter_transmission;
            out.stdev.append(v);
        }
    }


    // TODO: check if filter exists for specific channel
    return true; // we do not make any validation here
}


QString FilterPlugin::getParameterPreview()
{
    QString str = QString("(ID=%0)").arg(identifier);
    return str;
}
