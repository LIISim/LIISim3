#include "calibration.h"

#include "../../../core.h"
#include "../../mrun.h"
#include <QDebug>

QString Calibration::descriptionFileName = "calibration.html"; // TODO
QString Calibration::iconFileName = "iconfile"; // TODO
QString Calibration::pluginName = "Calibration";


QList<Signal::SType> Calibration::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW << Signal::ABS;

/**
 * @brief Calibration::Calibration
 * @param parentChain
 */
Calibration::Calibration(ProcessingChain *parentChain) :   ProcessingPlugin(parentChain)
{
    shortDescription = "Apply Calibration operations on each datapoint\n\
    1) Channel sensitvity: corrects for sensitivity of each PMT, values are stored in LIISettings file\n\
    2) Gain (exp): uses y = exp(-A*ln(x/x_ref)) to calculate gain correction factor\n\
    3) Gain (log10): uses y = 10^(-A*log_10(x/x_ref)) to calculate gain correction factor\n\
    x = gain control voltage,\n\
    A and x_ref from LIISettings";

    // standard values
    operation   = "gain (exp)";
    gainValuesFromRun = true;
    for(int i=1; i <= channelCount(); i++)
        calcvalue.append(mrun->pmtGainVoltage(i));

    // pass stdev to next processing step
    // new stdev is calculated within processSignalImplementation()
    preserveStdev = true;

    // create input list
    onMRunLIISettingsChanged();

    // show only parameters for selected operation
    onOperationChanged();

    connect(mrun,
            SIGNAL(LIISettingsChanged()),
            SLOT(onMRunLIISettingsChanged()));

     connect(mrun,
            SIGNAL(MRunDetailsChanged()),
            SLOT(onMRunLIISettingsChanged()));
}


/**
 * @brief Calibration::onMRunLIISettingsChanged This slot
 * is executed when the LIISettings of mrun have been modified.
 */
void Calibration::onMRunLIISettingsChanged()
{
    //qDebug() << mrun->getName() << " Calibration::onMRunLIISettingsChanged()";

    int numCh = channelCount();

    // update channel gain vector,
    // remember last channel gain values
    QList<double> calcvalue_old = calcvalue;
    calcvalue.clear();

    // check if gain values should be copied from run or from input fields
    if(gainValuesFromRun)
        for(int i=0; i < channelCount(); i++)
            calcvalue << mrun->pmtGainVoltage(i+1);
    else
        for(int i=0; i < numCh; i++)
            if(i < calcvalue_old.size())
                calcvalue << calcvalue_old[i];
            else
                calcvalue << 0.0;

    // remember old operation selection
    QString cbOperation_oldValue = operation;
    QString cbGainSource_oldValue = inputs.getValue("gainValueSource").toString();
    // reset input fields
    inputs.clear();


    ProcessingPluginInput cbOperation;
    cbOperation.type = ProcessingPluginInput::COMBOBOX;
    cbOperation.value = cbOperation_oldValue + ";gain (exp);gain (log10);channel sensitivity";
    cbOperation.labelText = "Operation";
    cbOperation.identifier = "cbOperation";
    cbOperation.tooltip = shortDescription;
    inputs << cbOperation; // add value to input list


    // gain calibration only

    ProcessingPluginInput cbGainSource;
    cbGainSource.type = ProcessingPluginInput::COMBOBOX;
    cbGainSource.groupID = 1;
    cbGainSource.value = cbGainSource_oldValue + ";from MRun settings file;manual";
    cbGainSource.labelText = "Source of gain values";
    cbGainSource.identifier = "gainValueSource";
    cbGainSource.tooltip = "Gain control voltages from individual runsettings files are used or can be set manually";
    inputs << cbGainSource;

    for(int i=0; i < numCh; i++)
    {
        ProcessingPluginInput inputCalcValue;
        inputCalcValue.type = ProcessingPluginInput::DOUBLE_FIELD;
        inputCalcValue.groupID = 1;
        inputCalcValue.value = calcvalue[i];
        inputCalcValue.minValue = -1E14;
        inputCalcValue.maxValue = 1E14;
        inputCalcValue.labelText = "Channel: " + QString::number(i+1);
        inputCalcValue.identifier = "cValue_" + QString::number(i+1);
        inputCalcValue.enabled = !gainValuesFromRun;
        inputCalcValue.tooltip = "Gain control voltages for each channel";
        inputs << inputCalcValue;
    }

    // call this manually on initialization
    onOperationChanged();

    setFromInputs();

    emit dataChanged(7); // notify gui
}


/**
 * @brief Calibration::getName implementation of virtual function
 * @return name of plugin
 */
QString Calibration::getName()
{
    return pluginName;
}


/**
 * @brief Calibration::setInputs overrides virtual function
 * @param inputs list of Processing plugin inputs
 */
void Calibration::setFromInputs()
{
    // if operation was changed, toggle visibility of parameter inputs
    if(inputs.getValue("cbOperation").toString() != operation)
        onOperationChanged();

    // assign members
    operation   = inputs.getValue("cbOperation").toString();

    QString gainValueSourceStr = inputs.getValue("gainValueSource").toString();
    bool new_gainValueState = true;
    if(gainValueSourceStr == "manual")
        new_gainValueState = false;
    else
        new_gainValueState = true;

    // check if 'gain value source' changed
    if(gainValuesFromRun != new_gainValueState)
    {
        // check if gain values should be copied from run
        if(new_gainValueState)
            for(int i=0; i < channelCount(); i++)
            {
                calcvalue[i] = mrun->pmtGainVoltage(i+1);
                QString identifier = "cValue_" + QString::number(i+1);
                inputs.setValue(identifier,calcvalue[i]);

                // disable input fields
                inputs.setEnabled(identifier,false);
            }
        else
            // for manual gain values -> enable input fields
            for(int i=0; i < channelCount(); i++)
                inputs.setEnabled("cValue_" + QString::number(i+1),true);

        // notify gui
        emit dataChanged(7);
    }

    gainValuesFromRun = new_gainValueState;
    for(int i=0; i < channelCount(); i++)
        calcvalue[i] = inputs.getValue("cValue_" + QString::number(i+1)).toDouble();

}


/**
 * @brief Calibration::processSignal implements virtual function
 * @param in  input signal
 * @param out output signal
 * @return true if the signal has passed validation
 */
bool Calibration::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    double rel_gain_factor;

    LIISettings curSettings = mrun->liiSettings();

    int idx = in.channelID-1;

    if(idx < 0)
        throw LIISimException("Calibration: invalid channel ID: " + QString::number(idx+1));

    if( curSettings.channels.size() <= idx)
    {
        throw LIISimException("Calibration: Not enough channels defined in LIISettings '"
                              + curSettings.name  + "' (defined: "
                              + QString::number(curSettings.channels.size())+ " needed: "
                              + QString::number( idx+1)+")");
    }
    Channel curChannel = curSettings.channels.at(idx);

    double cval = calcvalue.at(idx);

    int noPts = in.data.size();

    // y = 10^(A*log_10(x/xref))
    if(operation == "gain (log10)")
    {
        rel_gain_factor = pow(10.0, (
                              log10(cval / curChannel.pmt_gain)
                              * curChannel.pmt_gain_formula_A
                              ));
    }
    else if(operation == "gain (exp)")
    {              
        // y = exp(A*ln(x/xref))
        rel_gain_factor = exp(
                              log(cval / curChannel.pmt_gain)
                              * curChannel.pmt_gain_formula_A);
    }

    if(rel_gain_factor == 0.0)
        rel_gain_factor = 1.0;

    // use Calibration operation on every datapoint
    for(int i = 0; i < noPts; i++)
    {
        double v;
        if(operation == "gain (log10)" || operation == "gain (exp)")
            v = in.data.at(i) / rel_gain_factor;
        else if(operation == "channel sensitivity")
            v = in.data.at(i) * curChannel.calibration;
        else
            v = in.data.at(i);

        out.data.append(v);
    }
    //calculate stdev only if available
    if(!in.stdev.isEmpty())
    {
        noPts = in.stdev.size();
        double v;

        for(int i = 0; i < noPts; i++)
        {
            if(operation == "gain (log10)" || operation == "gain (exp)")
                v = in.stdev.at(i) / rel_gain_factor;
            else if(operation == "channel sensitivity")
                v = in.stdev.at(i) * curChannel.calibration;
            else
                v = in.stdev.at(i);

            out.stdev.append(v);
        }
    }
    //qDebug() << QString::number(curChannel.wavelength) << " - " << QString::number(curChannel.calibration);

    // TODO: check if calibration exists for specific channel
    return true; // we do not make any validation here
}


/**
 * @brief Calibration::getParameterPreview
 * @return
 */
QString Calibration::getParameterPreview()
{
    if(operation == "channel sensitivity")
        return "(channel sensitivity)";

    QString str = "(" + operation + ": ";
    for(int i=0; i < channelCount(); i++)
    {
        str = str + QString::number(calcvalue.at(i));
        if(i+1 != channelCount())
            str = str + "; ";
    }

    str = str + ")";
    return str;

}


/**
 * @brief Calibration::onOperationChanged change visible inputs when operation is changed
 */
void Calibration::onOperationChanged()
{
    if(inputs.getValue("cbOperation").toString() == "gain (log10)")
    {
        inputs.showGroup(1);
    }
    else if(inputs.getValue("cbOperation").toString() == "gain (exp)")
    {
        inputs.showGroup(1);
    }
    else
    {
        inputs.showGroup(2);
    }

    // signal 7: input fields modified (see also: ProcessingPlugin.h)
    emit dataChanged(7);
}
