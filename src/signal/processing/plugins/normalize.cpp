#include "normalize.h"

#include "../processingchain.h"
#include "../../mrun.h"
#include <QDebug>

QString Normalize::descriptionFileName = "nrmalize.html"; // TODO
QString Normalize::iconFileName = "iconfile"; // TODO
QString Normalize::pluginName = "Normalize";

QList<Signal::SType> Normalize::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW
        << Signal::ABS
        << Signal::TEMPERATURE;

//TODO: comboBox Channel for TEMPERATURE Signals

Normalize::Normalize(ProcessingChain *parentChain) :   ProcessingPlugin(parentChain)
{
    shortDescription = "Normalize signal of specific channel to peak or value";

    // standard values
    operation   = "peak";
    chId        = "all";
    value       = 0.0;

    // create input fields;
    ProcessingPluginInput cbOperation;
    cbOperation.type = ProcessingPluginInput::COMBOBOX;
    cbOperation.value = "peak;peak;value";
    cbOperation.labelText = "Operation";
    cbOperation.identifier = "cbOperation";
    cbOperation.tooltip = "Normalize signal trace to peak or value";

    inputs << cbOperation; // add value to input list

    ProcessingPluginInput cbChannel;
    cbChannel.type = ProcessingPluginInput::COMBOBOX;
    cbChannel.labelText = "Channel";
    cbChannel.identifier = "cbChannel";
    cbChannel.tooltip = "Select channel for this operation";

    // fill combobox with values
    int numCh = channelCount();

    QString str,res;

    for(int i=0; i < numCh; i++)
    {
        res.append(str.sprintf(";%d", i+1));
    }
    cbChannel.value = "all;all" + res;

    inputs << cbChannel;

    ProcessingPluginInput inputValue;
    inputValue.type = ProcessingPluginInput::DOUBLE_FIELD;
    inputValue.value = value;
    inputValue.minValue = -1E12;
    inputValue.maxValue = 1E12;
    inputValue.labelText = "Value: ";
    inputValue.identifier = "cValue";
    inputValue.tooltip = "If value operation is selected, the complete signal is normalized to this value";

    inputs << inputValue;
}


/**
 * @brief Normalize::getName implementation of virtual function
 * @return name of plugin
 */
QString Normalize::getName()
{
    return pluginName;
}


/**
 * @brief Normalize::setInputs overrides virtual function
 * @param inputs list of Processing plugin inputs
 */
void Normalize::setFromInputs()
{
    // assign members

    operation   = inputs.getValue("cbOperation").toString();
    chId        = inputs.getValue("cbChannel").toString();
    value       = inputs.getValue("cValue").toDouble();
}

/**
 * @brief Normalize::processSignal implements virtual function
 * @param in  input signal
 * @param out output signal
 * @return true if the signal has passed validation
 */
bool Normalize::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    // process selected channel only
    if(chId == "all" || (chId != "all" && in.channelID == chId.toInt()))
    {
        int noPts = in.data.size();

        // get signal of mpoint mpIdx with channelId= chId_B at previous position in chain
        //ProcessingChain* pchain = mrun->getProcessingChain(in.type);
        //Signal s = pchain->getStepSignalPre(mpIdx, in.channelID, positionInChain-1);
        Signal s = in;
        double maxValue = s.getMaxValue();

        // use normalize operation on every datapoint
        for(int i = 0; i < noPts; i++)
        {
            double v;
            if(operation == "peak")
                v = in.data.at(i) / maxValue;
            else if(operation == "value")
                v = in.data.at(i) / value;
            else
                v = in.data.at(i);

            out.data.append(v);
        }
    }
    else
    {
        // don't change other channels
        out = in;
    }

    return true; // we do not make any validation here
}


/**
 * @brief Arithmetic::getParameterPreview
 * @return
 */
QString Normalize::getParameterPreview()
{
    QString str = "(Ch %1;%2;val=%3)";
    return str.arg(chId).arg(operation).arg(value);
}
