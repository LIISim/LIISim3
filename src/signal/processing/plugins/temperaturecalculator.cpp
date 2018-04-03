#include "temperaturecalculator.h"

#include <QDebug>
#include "../../../core.h"
#include "../../mrun.h"
#include "../../mpoint.h"
#include "../../../general/LIISimException.h"
#include "../../../calculations/temperature.h"
#include "../processingchain.h"
#include "../processingpluginconnector.h"
#include "multisignalaverage.h"

// init static members
QString TemperatureCalculator::descriptionFileName  = "temperatureCalculator.html"; // TODO
QString TemperatureCalculator::iconFileName         = "iconfile"; // TODO
QString TemperatureCalculator::pluginName           = "Temperature Calculator";

// avoid instantiation of plugin by returning empty list
QList<Signal::SType> TemperatureCalculator::supportedSignalTypes = QList<Signal::SType>()
        << Signal::TEMPERATURE;

QMap<int,int> TemperatureCalculator::globalTemperatureChannelIDmap = QMap<int,int>();


/**
 * @brief TemperatureCalculator::TemperatureCalculator Constructor
 * @param parent
 */
TemperatureCalculator::TemperatureCalculator(ProcessingChain *parentChain) : ProcessingPlugin(parentChain)
{
    m_tempChannelID = -1; //parentChain->mrun()->addTemperatureChannel();

    chId1 = 1;
    chId2 = 2;
    chId_copySignals = 1;

    inputSignalType = Signal::ABS;
    inputType = "Absolute";
    method = "Two-Color";
    sourceEm = "function"; // 'function, 'values', 'from Drude theory'

    bpIntegration = false;
    weighting = false;
    iter = 100;
    startT = 2500.0;
    startC = 1.0;
    autoStartC = true;

    activeChannels = new QList<bool>;

    executeSyncronized = true;
    msaMode = false;
    msaSearched = false;
    preserveStdev = true;

    shortDescription = "Calculate temperature signal from channel data (Two-color pyrometry or spectral fit)";

    ProcessingPluginInput cbMethod;
    cbMethod.type = ProcessingPluginInput::COMBOBOX;
    cbMethod.labelText = "Method";
    cbMethod.identifier = "cbMethod";

#ifdef LIISIM_FULL
    cbMethod.value = "Two-Color;Two-Color;Ratio A/B;Spectrum;Test;copy from signals";
    cbMethod.tooltip = "1) Two-Color: Uses Two-Color pyrometry to calculate temperature trace for two channels\n"
                       "2) Ratio: outputs only ratio between two channels\n"
                       "3) Spectrum: Least-square minimization of Planck's law to the data of the selected channels (can be visualized in AnalysisTools)\n"
                       "4) Testing of new algorithms"
                       "5) Copy channel from signals (workarround for temperature import; no calculation)";
#else
    cbMethod.value = "Two-Color;Two-Color;Spectrum";
    cbMethod.tooltip = "1) Two-Color: Uses Two-Color pyrometry to calculate temperature "
                       "trace for two channels\n"
                       "2) Spectrum: Least-square minimization of Planck's law to the selected channels "
                       "(can be visualized in AnalysisTools)";
#endif

    inputs << cbMethod;

    ProcessingPluginInput signalType;
    signalType.type = ProcessingPluginInput::COMBOBOX;
    signalType.value = "Absolute;Raw;Absolute";
    signalType.labelText = "Signal source";
    signalType.identifier = "signalType";
    signalType.tooltip = "Source for temperature calculation";

    inputs << signalType;

    ProcessingPluginInput cbMaterial;
    cbMaterial.type = ProcessingPluginInput::COMBOBOX;
    cbMaterial.value = "";
    cbMaterial.labelText = "Material";
    cbMaterial.identifier = "cbMaterial";
    cbMaterial.tooltip = "Optical properties for temperature calculation are taken from this material\n\
global: Material is taken from global setting in calculation toolbox (top ribbon: CALCULATION)";

    inputs << cbMaterial;

    // fill combobox with database content
    updateMaterialBox();


    ProcessingPluginInput cbSourceEm;
    cbSourceEm.type = ProcessingPluginInput::COMBOBOX;
    cbSourceEm.value = "function;values;function;from Drude theory";
    cbSourceEm.labelText = "E(m) source";
    cbSourceEm.identifier = "cbSourceEm";
    cbSourceEm.tooltip = "Select which E(m) values should be used for calculation \n"
                         " - E(m) is defined in Material (.txt) files \n"
                         " - Em: single values are provided for each center wavelength \n"
                         " - Em_func: wavelength dependent function \n"
                         " - from Drude theory: E(m) is calculated from plasma frequency (omega_p) and relaxation time (tau) \n"
                         "Please see the User Guide for further information";



    inputs << cbSourceEm;


    // Parameters: "Two-Color" (GroupID = 1)

    ProcessingPluginInput cbSig1;
    cbSig1.type = ProcessingPluginInput::COMBOBOX;
    cbSig1.groupID = 1;
    cbSig1.value = "";
    cbSig1.labelText = "Signal A";
    cbSig1.identifier = "cbSig1";
    cbSig1.tooltip = "Signal A";

    inputs << cbSig1;

    ProcessingPluginInput cbSig2;
    cbSig2.type = ProcessingPluginInput::COMBOBOX;
    cbSig2.groupID = 1;
    cbSig2.value = "";
    cbSig2.labelText = "Signal B";
    cbSig2.identifier = "cbSig2";
    cbSig2.tooltip = "Signal B";

    inputs << cbSig2;


    // Parameters: "Spectrum" (GroupID = 2)


    ProcessingPluginInput inputSelectChannel;
    inputSelectChannel.type = ProcessingPluginInput::CHECKBOX_GROUP;
    inputSelectChannel.groupID = 2;
    inputSelectChannel.value = "";
    inputSelectChannel.labelText = "Select Channels";
    inputSelectChannel.identifier = "inputSelectChannel";
    inputSelectChannel.tooltip = "Select channels used for spectral fit";

    inputs << inputSelectChannel;


    ProcessingPluginInput inputBPInt;
#ifdef LIISIM_FULL
    inputBPInt.type = ProcessingPluginInput::CHECKBOX;
#else
    inputBPInt.type = ProcessingPluginInput::NOGUI;
#endif
    inputBPInt.groupID = 2;
    inputBPInt.value = bpIntegration;
    inputBPInt.labelText = "Bandpass integration";
    inputBPInt.identifier = "inputBPInt";
    inputBPInt.tooltip   = "Uses bandpass filter width to integrate over temperature spectrum (experimental!)";

    inputs << inputBPInt;


    ProcessingPluginInput inputWeighting;
#ifdef LIISIM_FULL
    inputWeighting.type = ProcessingPluginInput::CHECKBOX;
#else
    inputWeighting.type = ProcessingPluginInput::NOGUI;
#endif
    inputWeighting.groupID = 2;
    inputWeighting.value = weighting;
    inputWeighting.labelText = "Weighting (standard deviation)";
    inputWeighting.identifier = "inputWeighting";
    inputWeighting.tooltip   = "Uses standard deviation from MultiSignalAverage plugin (experimental!)";

    inputs << inputWeighting;


    ProcessingPluginInput inputAutoStartC;
#ifndef LIISIM_LITE
    inputAutoStartC.type = ProcessingPluginInput::CHECKBOX;
#else
    inputAutoStartC.type = ProcessingPluginInput::NOGUI;
#endif
    inputAutoStartC.groupID    = 2;
    inputAutoStartC.value      = autoStartC;
    inputAutoStartC.labelText  = "Automatic initial scaling factor";
    inputAutoStartC.identifier = "inputAutoStartC";
    inputAutoStartC.tooltip    = "Calculates initial scaling factor from signal peak (this prevents numerical stability problems)";
    inputAutoStartC.enabled    = false;

    inputs << inputAutoStartC;


    ProcessingPluginInput inputIter;
    inputIter.type = ProcessingPluginInput::DOUBLE_FIELD;
    inputIter.groupID = 2;
    inputIter.value = iter;
    inputIter.minValue = 1;
    inputIter.maxValue = 500;
    inputIter.labelText = "Iterations";
    inputIter.identifier = "inputIter";
    inputIter.tooltip = "Maximum number of iteration used per data point";

    inputs << inputIter;


    ProcessingPluginInput inputStartT;
    inputStartT.type = ProcessingPluginInput::DOUBLE_FIELD;
    inputStartT.groupID = 2;
    inputStartT.value = startT;
    inputStartT.minValue = 300;
    inputStartT.maxValue = 10000;
    inputStartT.labelText = "Start temperature";
    inputStartT.identifier = "inputStartT";
    inputStartT.tooltip = "Start temperature for least-quare fit";

    inputs << inputStartT;


    // this field is currently not used: use automatic scaling factor instead
    ProcessingPluginInput inputStartC;
    inputStartC.type = ProcessingPluginInput::NOGUI;
    inputStartC.groupID = 2;
    inputStartC.value = startC;
    inputStartC.minValue = 1E-30;
    inputStartC.maxValue = 1E30;
    inputStartC.labelText = "Start scaling factor (C)";
    inputStartC.identifier = "inputStartC";
    inputStartC.tooltip = "Start scaling factor for least-quare fit";
    inputStartC.enabled = false;

    inputs << inputStartC;


    // Parameters: "Test" (GroupID = 3)

    ProcessingPluginInput inputSelectChannel_Test;
    inputSelectChannel_Test.type = ProcessingPluginInput::CHECKBOX_GROUP;
    inputSelectChannel_Test.groupID = 3;
    inputSelectChannel_Test.value = "";
    inputSelectChannel_Test.labelText = "Select Channels";
    inputSelectChannel_Test.identifier = "inputSelectChannel_Test";
    inputSelectChannel_Test.tooltip = "";

    inputs << inputSelectChannel_Test;

    ProcessingPluginInput inputIter_Test;
    inputIter_Test.type = ProcessingPluginInput::DOUBLE_FIELD;
    inputIter_Test.groupID = 3;
    inputIter_Test.value = iter;
    inputIter_Test.minValue = 1;
    inputIter_Test.maxValue = 500;
    inputIter_Test.labelText = "Iterations";
    inputIter_Test.identifier = "inputIter_Test";
    inputIter_Test.tooltip = "";

    inputs << inputIter_Test;

    ProcessingPluginInput inputStartT_Test;
    inputStartT_Test.type = ProcessingPluginInput::DOUBLE_FIELD;
    inputStartT_Test.groupID = 3;
    inputStartT_Test.value = startT;
    inputStartT_Test.minValue = 1;
    inputStartT_Test.maxValue = 10000;
    inputStartT_Test.labelText = "Start temperature";
    inputStartT_Test.identifier = "inputStartT_Test";
    inputStartT_Test.tooltip = "";

    inputs << inputStartT_Test;

    ProcessingPluginInput inputStartC_Test;
    inputStartC_Test.type = ProcessingPluginInput::DOUBLE_FIELD;
    inputStartC_Test.groupID = 3;
    inputStartC_Test.value = startC;
    inputStartC_Test.minValue = 1E-25;
    inputStartC_Test.maxValue = 1E25;
    inputStartC_Test.labelText = "Start scaling factor (C)";
    inputStartC_Test.identifier = "inputStartC_Test";
    inputStartC_Test.tooltip = "Start scaling factor for least-quare fit";

    inputs << inputStartC_Test;


    // Parameters: "copy from signals" (GroupID = 4)

    ProcessingPluginInput cbSig_copySignals;
    cbSig_copySignals.type = ProcessingPluginInput::COMBOBOX;
    cbSig_copySignals.groupID = 4;
    cbSig_copySignals.value = "";
    cbSig_copySignals.labelText = "Channel:";
    cbSig_copySignals.identifier = "cbSig_copySignals";
    cbSig_copySignals.tooltip = "Copy this channel to the new temperature channel (no calculation)";

    inputs << cbSig_copySignals;

    // General parameters

    ProcessingPluginInput tchid;
    tchid.type = ProcessingPluginInput::NOGUI;
    tchid.value = "-1";
    tchid.identifier = "tchid";
    inputs << tchid;

    // this call is necessary to create checkboxes of channel selection
    onLIISettingsChanged();

    // show only parameters for selected method
    onMethodChanged();

    m_sourcePchain = mrun->getProcessingChain(inputSignalType);
    setMRun(mrun);

    // observe changes of modeling settings
    connect(Core::instance()->modelingSettings,
            SIGNAL(materialSpecChanged()),
            SLOT(onMaterialChanged()));

    connect(Core::instance()->getDatabaseManager(),
            SIGNAL(signal_contentChanged()),
            SLOT(updateMaterialBox()));

    connect(mrun,
           SIGNAL(MRunDetailsChanged()),
           SLOT(onMRunChanged()));

    connect(mrun,
            SIGNAL(LIISettingsChanged()),
            SLOT(onLIISettingsChanged()));
}


TemperatureCalculator::~TemperatureCalculator()
{
    if(globalTemperatureChannelIDmap.contains(m_tempChannelID))
    {
        int newcount = globalTemperatureChannelIDmap.value(m_tempChannelID) -1;
        globalTemperatureChannelIDmap.insert(m_tempChannelID,
            newcount);
    }
    // qDebug() << "~TemperatureCalculator() " << globalTemperatureChannelIDmap;
    mrun->removeTemperatureChannel(m_tempChannelID);
}


/**
 * @brief TemperatureCalculator::getName implementation of virtual function
 * @return  name of plugin
 */
QString TemperatureCalculator::getName()
{
    return pluginName;
}


/**
 * @brief TemperatureCalculator::temperatureChannelID
 * @return ID of temperature channel of this plugins
 */
int TemperatureCalculator::temperatureChannelID()
{
    return m_tempChannelID;
}


/**
 * @brief TemperatureCalculator::temperatureChannelIDs
 * @return list of global temperature channel IDs
 */
QList<int> TemperatureCalculator::temperatureChannelIDs()
{
    return globalTemperatureChannelIDmap.keys();
}


int TemperatureCalculator::generateTemperatureChannelID()
{
    int newcid = -1;

    // look for an existing, unused channel id in map;
    QList<int> keys = globalTemperatureChannelIDmap.keys();

    for(int i = 0; i < keys.size(); i++)
    {
        if(globalTemperatureChannelIDmap.value(keys[i]) == 0)
        {
            newcid = keys[i];
            break;
        }
    }

    // generate a new channel id if all existing channel ids are in use
    if( newcid == -1)
    {
        int maxel = -1;
        for(int i =0; i < keys.size(); i++)
        {
            if(keys[i] > maxel)
                maxel = keys[i];
        }
        if(maxel == -1)
            maxel = 0;

        newcid = maxel+1;
        globalTemperatureChannelIDmap.insert(newcid,0);
    }

    // qDebug() << "TemperatureCalculator::generateTemperatureChannelID()" << globalTemperatureChannelIDmap;
    return newcid;
}


void TemperatureCalculator::setTemperatureChannelID(int tchid)
{
    //  qDebug() << "TemperatureCalculator::setTemperatureChannelID from" << m_tempChannelID << " to " <<tchid ;
    if(tchid == -1 )
        return;
    if(m_tempChannelID == tchid)
        return;
    if(m_tempChannelID > -1)
        mrun->removeTemperatureChannel(m_tempChannelID);
    m_tempChannelID = tchid;

    globalTemperatureChannelIDmap.insert( tchid,
          globalTemperatureChannelIDmap.value(tchid,0)+1);

    inputs.setValue("tchid",m_tempChannelID);
    mrun->addTemperatureChannel(m_tempChannelID);

    // qDebug() << "TemperatureCalculator::setTemperatureChannelID " << globalTemperatureChannelIDmap;
    // emit dataChanged(3,true);
}


/**
 * @brief TemperatureCalculator::onAddedToPchain This slot
 * is executed when a Plugin has been added to a ProcessingChain
 */
void TemperatureCalculator::onAddedToPchain()
{
    ProcessingPlugin::initializeCalculation();
}


void TemperatureCalculator::setMRun(MRun *mrun)
{
    this->mrun = mrun;

    // setup the signal selection comboboxes
    int numCh = mrun->getNoChannels(inputSignalType);
    ProcessingPluginInput* cbSig1 = inputs.getPluginInput("cbSig1");
    ProcessingPluginInput* cbSig2 = inputs.getPluginInput("cbSig2");
    ProcessingPluginInput* cbSig_copySignals = inputs.getPluginInput("cbSig_copySignals");
    QString res, res2;
    QString str;

    for(int i=0; i < numCh; i++)
    {
        // set default value for combobox Signal 1
        if(i == 0)
            res.append(str.sprintf("Channel %d;", i+1));


        // set default value for combobox Signal 2
        if(numCh == 1 && i == 0)
        {
            // first channel es default
            res2.append(str.sprintf("Channel %d;", i+1));
        }
        else if(numCh != 1 && i == 0)
        {
            // second channel as default
            res2.append(str.sprintf("Channel %d;", i+2));
        }

        // add channel to list
        str.sprintf("Channel %d", i+1);
        res.append(str);
        res2.append(str);

        // add seperator (not for last entry)
        if(i < numCh - 1)
        {
            res.append(";");
            res2.append(";");
        }
    }
    cbSig1->value = res;
    cbSig2->value = res2;

    // set for copy signals channel combobox same channels as for Two-Color:Signal A
    cbSig_copySignals->value = res;
}


void TemperatureCalculator::updateMaterialBox()
{
    // TODO: check if Em_func is available for Spectrum mode

    ProcessingPluginInput* cbMaterial = inputs.getPluginInput("cbMaterial");
    QString str;

    QList<DatabaseContent*> *materials = Core::instance()->getDatabaseManager()->getMaterials();

    str.append(selected_material);
    str.append(";global");

    for(int i=0; i < materials->size(); i++)
    {
        str.append(QString(";%0").arg(materials->at(i)->name));
    }

    cbMaterial->value = str;

    // signal 7: input fields modified (see also: ProcessingPlugin.h)
    emit dataChanged(7);

}


void TemperatureCalculator::updateMRunMetadata()
{
    if(mrun != NULL)
    {
        TempCalcMetadata metadata;
        metadata.tempChannelID = temperatureChannelID();
        metadata.method = method;

        if(selected_material == "global")
            metadata.material = Core::instance()->modelingSettings->materialSpec().name;
        else
            metadata.material = selected_material;

        metadata.sourceEm       = sourceEm;
        metadata.signalSource   = inputSignalType;
        metadata.channelID1     = chId1;
        metadata.channelID2     = chId2;
        metadata.iterations     = iter;
        metadata.startTemperature = startT;
        metadata.startC         = startC;
        metadata.autoStartC     = autoStartC;
        metadata.activeChannels = *activeChannels;
        metadata.bandpass       = bpIntegration;
        metadata.weighting      = weighting;
        mrun->tempMetadata.insert(temperatureChannelID(), metadata);
    }
}


void TemperatureCalculator::setFromInputs()
{
    // if method was changed, toggle visibility of parameter inputs
    if(inputs.getValue("cbMethod").toString() != method)
        onMethodChanged();

    method = inputs.getValue("cbMethod").toString();

    inputType = inputs.getValue("signalType").toString();

    if(inputType == "Raw")
        inputSignalType = Signal::RAW;
    else if(inputType == "Absolute")
        inputSignalType = Signal::ABS;

   selected_material = inputs.getValue("cbMaterial").toString();

   sourceEm = inputs.getValue("cbSourceEm").toString();

    // Two-Color:

    QString sigCh1 = inputs.getValue("cbSig1").toString();
    if( !sigCh1.isEmpty())
        chId1 = sigCh1.remove("Channel ").toInt();


    QString sigCh2 = inputs.getValue("cbSig2").toString();
    if( !sigCh2.isEmpty())
        chId2 = sigCh2.remove("Channel ").toInt();

    // copy from signals

    QString sigCh_copySignals = inputs.getValue("cbSig_copySignals").toString();
    if( !sigCh_copySignals.isEmpty())
        chId_copySignals = sigCh_copySignals.remove("Channel ").toInt();

    // Spectrum:

    QString inputSelectChannelStr;

    if(method == "Spectrum")
    {

        bpIntegration   = inputs.getValue("inputBPInt").toBool();
        weighting       = inputs.getValue("inputWeighting").toBool();

        iter        = inputs.getValue("inputIter").toInt();
        startT      = inputs.getValue("inputStartT").toDouble();
        startC      = inputs.getValue("inputStartC").toDouble();
        autoStartC  = inputs.getValue("inputAutoStartC").toBool();

        inputSelectChannelStr = inputs.getValue("inputSelectChannel").toString();
    }
    // Test:
    else
    {
        iter   = inputs.getValue("inputIter_Test").toInt();
        startT = inputs.getValue("inputStartT_Test").toDouble();
        startC = inputs.getValue("inputStartC_Test").toDouble();

        inputSelectChannelStr = inputs.getValue("inputSelectChannel_Test").toString();
    }

    activeChannels->clear();

    QStringList list = inputSelectChannelStr.split(";");

    foreach(QString s, list)
    {
        if(s != "")
        {
          // convert string to bool
          activeChannels->append((s.toInt() != 0));
          //qDebug() << s <<  (s.toInt() != 0);
        }
    }

    // General

    int tcid = inputs.getValue("tchid").toInt();
    setTemperatureChannelID(tcid);
}


/**
 * @brief TemperatureCalculator::processSignal implements virtual function
 * @param in input signal
 * @param out output signal
 * @param mpIdx index of current measurement point
 * @return true if the signal has passed validation
 */
bool TemperatureCalculator::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{

    // -----------------------------------------------------------
    // General checks specific for temperature channel management

    // do nothing if input channel does not match the output temperature channel
    if(in.channelID != m_tempChannelID)
    {
        out = in;
        return true;
    }

    QMutexLocker lock(&variableMutex);
    if(msaSearched)
        lock.unlock();

    // get current measurement point, after preprocessing
    if(msaMode)
    {
        // qDebug() << "TC: sigsize " << msaTemp.data.size();
        out = msaTemp;
        return true;
    }

    // debug output for active channels
    /*
    QString str;
    for(int j = 0; j < activeChannels->size(); j++)
        str.append(QString::number(activeChannels->at(j)));

    qDebug() << mrun->getName() << method << str;
    */

    // debug output for valid signals
    /*
    if(!msaMode && !pchain->isValid(mpIdx) )
    {
       // qDebug() << "invalid at " << mpIdx;
        return true;
    }
    */

    if(m_sourcePchain->getSignalType() != inputSignalType)
        m_sourcePchain = mrun->getProcessingChain(inputSignalType);

    // -----------------------------------------------------------
    // main processing
    try {

        // reset error state
        mProcessingError = false;

        MPoint* mp = mrun->getPost(mpIdx);
        LIISettings curSettings = mrun->liiSettings();

        // check if number of channels matches
        int channelcount = curSettings.channels.size();

        if(channelcount != mrun->getNoChannels(inputSignalType))
            throw LIISimException("LIISettings (" + curSettings.name
                                  +") and mrun (" + mrun->getName()
                                  + ") are NOT compatible: different number of channels");

        // check this only for two-color pyrometry
        if(method == "Two-Color")
        {
            if(chId1 <= 0 || chId2 <= 0)
                throw LIISimException("no channels selected!");

            if(chId1 == chId2 )
                throw LIISimException("same input signals!");

            if(chId1 > channelcount)
                throw LIISimException(
                        QString("invalid channel id (%0) selected. Number of channels defined in LIISettings '%1': %2")
                                      .arg(chId1)
                                      .arg(curSettings.name)
                                      .arg(channelcount));

            if(chId2 > channelcount)
                throw LIISimException(
                        QString("invalid channel id (%0) selected. Number of channels defined in LIISettings '%1': %2")
                                      .arg(chId2)
                                      .arg(curSettings.name)
                                      .arg(channelcount));
        }

        // always set material to global setting first (see CalculationToolBox):
        Material material = Core::instance()->modelingSettings->materialSpec();

        // try to get selected name and overwrite material
        if(selected_material != "global")
        {
            QList<DatabaseContent*> materials = *Core::instance()->getDatabaseManager()->getMaterials();
            for(int k = 0; k < materials.size(); k++)
            {
                if(selected_material == materials.at(k)->name)
                    material = Material(*Core::instance()->getDatabaseManager()->getMaterial(k));
            }
        }


        if(method == "Two-Color")
        {
            // returns empty signal if error occurs
            out = Temperature::calcTemperatureFromTwoColor(curSettings,
                                                           material,
                                                           sourceEm,
                                                           mp,
                                                           chId1, chId2,
                                                           inputSignalType);
        }
        else if(method == "Spectrum")
        {
            if(!activeChannels->contains(true))
            {
                throw LIISimException("Spectrum: No channels selected");
            }

            // returns empty signal if error occurs
            out = Temperature::calcTemperatureFromSpectrum(curSettings,
                                                           material,
                                                           sourceEm,
                                                           mp,
                                                           inputSignalType,
                                                           activeChannels,
                                                           bpIntegration,
                                                           weighting,
                                                           autoStartC,
                                                           iter,
                                                           startT,
                                                           startC
                                                           );            
        }
        else if(method == "Test")
        {
            if(!activeChannels->contains(true))
            {
                throw LIISimException("Test: No channels selected");
            }

            // returns empty signal if error occurs
            out = Temperature::calcTemperatureFromSpectrumTest(curSettings,
                                                           material,                                                               
                                                           sourceEm,
                                                           mp,
                                                           inputSignalType,
                                                           activeChannels,
                                                           iter,
                                                           startT,
                                                           startC
                                                           );

        }
        else if(method == "copy from signals")
        {
            Signal signal_1 = mp->getSignal(chId_copySignals, inputSignalType);

            out.data = signal_1.data;
            out.start_time = signal_1.start_time;
            out.dt = signal_1.dt;
            out.type = Signal::TEMPERATURE;

        }
        else // "Ratio"
        {
            Signal signal_1 = mp->getSignal(chId1, inputSignalType);
            Signal signal_2 = mp->getSignal(chId2, inputSignalType);

            int sz1 = signal_1.data.size();
            int sz2 = signal_2.data.size();

            int idx = 0;
            while( idx < sz1 && idx < sz2)
            {
                out.data.append(signal_1.data[idx]/signal_2.data[idx]);
                idx++;
            }
           // qDebug() << out.data.size();
           // this caused an error, because of different signal sizes!
           // for (int i=0; i< signal_1.data.size(); i++)
           // {
           //     signal_1.data[i] = signal_1.data[i]/signal_2.data[i];
           // }

            out.start_time = signal_1.start_time;
            out.dt = signal_1.dt;            
            out.type = Signal::TEMPERATURE;
        }


        // if empty signal is returned: error (messages are shown in notification window)
        if(out.data.size() == 0)
        {
            qDebug() << "TemperatureCalculator: Error T-Channel:" << m_tempChannelID;
            mProcessingError = true;
        }

        out.channelID = m_tempChannelID;
        out.fitMaterial = material.name;

    }
    catch(LIISimException e)
    {
        mProcessingError = true;

        // rethrow exception to caller
        throw LIISimException("TemperatureCalculator: " + e.what(), e.type());
    }

    if(!msaSearched)
    {
        msaMode = m_sourcePchain->containsActivePlugin( MultiSignalAverage::pluginName );
        if(msaMode )
            msaTemp = out;
        msaSearched = true;
    }

    updateMRunMetadata();

    if(mProcessingError)
        return false;
    else
        return true;
}


void TemperatureCalculator::reset()
{
    msaMode = false;
    msaSearched = false;

    ProcessingPlugin::reset();
}


QString TemperatureCalculator::getParameterPreview()
{      
    QString str = "";
    if(this->m_tempChannelID == -1)
        str = "Temperature Channel not available ";
    else
    {
        if(method == "Two-Color")
        {
            str = QString("Two-Color (T%1), %2, Source: %3/%4")
                    .arg(this->m_tempChannelID)
                    .arg(inputType)
                    .arg(chId1)
                    .arg(chId2);
        }
        else if(method == "Spectrum")
        {
            QString ch_str = " ";
            for(int i = 0; i < activeChannels->size(); i++)
            {
                if(activeChannels->at(i) == true)
                    ch_str.append(QString("%0 ").arg(QString::number(i+1)));
            }

            str = QString("Spectrum (T%1), %2 (%3), IT:%4, T=%5")
                    .arg(this->m_tempChannelID)
                    .arg(inputType)
                    .arg(ch_str)
                    .arg(iter)
                    .arg(startT);
        }
        else if(method == "Test")
        {
            str = QString("Test (T%1), %2, IT:%3, T=%4,C=%5")
                    .arg(this->m_tempChannelID)
                    .arg(inputType)
                    .arg(iter)
                    .arg(startT)
                    .arg(startC);
        }
        else if(method == "copy from signals")
        {
            str = QString("Copy (T%1), %2, Channel: %3")
                    .arg(this->m_tempChannelID)
                    .arg(inputType)
                    .arg(chId_copySignals);
        }
        else // "Ratio"
        {
            str = QString("Ratio (T%1), %2, Source: %3/%4")
                    .arg(this->m_tempChannelID)
                    .arg(inputType)
                    .arg(chId1)
                    .arg(chId2);
        }
    }
    return str;
}


/**
 * @brief TemperatureCalculator::onMethodChanged This slot is executed when plugin method
 * has changed and toggles visibility of other parameters
 */
void TemperatureCalculator::onMethodChanged()
{
    if(inputs.getValue("cbMethod").toString() == "Spectrum")
    {
        inputs.showGroup(2);
    }
    else if(inputs.getValue("cbMethod").toString() == "Test")
    {
        inputs.showGroup(3);
    }
    else if(inputs.getValue("cbMethod").toString() == "copy from signals")
    {
        inputs.showGroup(4);
    }
    else
    {
        inputs.showGroup(1);
    }

    // signal 7: input fields modified (see also: ProcessingPlugin.h)
    emit dataChanged(7);
}


/**
 * @brief TemperatureCalculator::onMaterialChanged This slot is executed
 * when the material selection of the global modeling settings has changed
 * (also see Core::modelingSettings, ModelingSettings)
 */
void TemperatureCalculator::onMaterialChanged()
{
    setDirty(true);
}


/**
 * @brief TemperatureCalculator::onMRunChanged This slot is executed
 * when the user selects other MRun
 */
void TemperatureCalculator::onMRunChanged()
{
    // default: checked
    bool val = true;

    // setup the signal selection comboboxes
    int numCh = mrun->getNoChannels(inputSignalType);
    LIISettings curSettings = mrun->liiSettings();

    inputs.getPluginInput("inputSelectChannel")->checkboxGroupLabels.clear();
    inputs.getPluginInput("inputSelectChannel")->checkboxGroupValues.clear();

    inputs.getPluginInput("inputSelectChannel_Test")->checkboxGroupLabels.clear();
    inputs.getPluginInput("inputSelectChannel_Test")->checkboxGroupValues.clear();

    if(activeChannels->size() == 0)
    {
        for(int i=0; i < numCh; i++)
            activeChannels->append(val);
    }


    for(int i=0; i < numCh; i++)
    {
        int lambda = curSettings.channels.at(i).wavelength;

        QString label = QString("Channel %0 (%1 nm)").arg(i+1).arg(lambda);

        // add wavelength information
        inputs.getPluginInput("inputSelectChannel")->checkboxGroupLabels.append(label);
        inputs.getPluginInput("inputSelectChannel_Test")->checkboxGroupLabels.append(label);

        // always pre-select channels
        inputs.getPluginInput("inputSelectChannel")->checkboxGroupValues.append(activeChannels->at(i));
        inputs.getPluginInput("inputSelectChannel_Test")->checkboxGroupValues.append(activeChannels->at(i));
    }

    emit dataChanged(3); // notify gui (Parameters changed)
}


/**
 * @brief TemperatureCalculator::onLIISettingsChanged This slot is executed
 * when the liisettings of the parent mrun were changed
 */
void TemperatureCalculator::onLIISettingsChanged()
{
    setDirty(true);

    onMRunChanged();

    emit dataChanged(7); // input fields changed
}


Signal::SType TemperatureCalculator::getInputSignalType()
{
    return inputSignalType;
}
