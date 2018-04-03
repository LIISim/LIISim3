#include "arithmetic.h"

#include "../../mrun.h"
#include <QDebug>

QString Arithmetic::descriptionFileName = "arithmetic.html"; // TODO
QString Arithmetic::iconFileName = "iconfile"; // TODO
QString Arithmetic::pluginName = "Arithmetic";

QList<Signal::SType> Arithmetic::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW << Signal::ABS << Signal::TEMPERATURE;


/**
 * @brief Arithmetic::Arithmetic
 * @param parentChain
 */
Arithmetic::Arithmetic(ProcessingChain *parentChain) :   ProcessingPlugin(parentChain)
{
    shortDescription = "Apply arithmetic operations on each datapoint";

    // standard values
    operation   = "multiplication";
    chId        = "all";
    calcvalue   = -1.0;

    // create input fields;
    ProcessingPluginInput cbOperation;
    cbOperation.type = ProcessingPluginInput::COMBOBOX;
    cbOperation.value = "multiplication;multiplication;division;addition;subtraction";
    cbOperation.labelText = "Operation";
    cbOperation.identifier = "cbOperation";
    cbOperation.tooltip = "Select mathematical operation";

    inputs << cbOperation; // add value to input list

    ProcessingPluginInput cbChannel;
    cbChannel.type = ProcessingPluginInput::COMBOBOX;
    cbChannel.value = "all";
    cbChannel.labelText = "Channel";
    cbChannel.identifier = "cbChannel";
    cbChannel.tooltip = "Select channel to which the operation is applied";

    // fill combobox with values
    int numCh = channelCount();

    QString str,res;

    // add option "all"
    res.append("all;all");

    for(int i=0; i < numCh; i++)
    {
        res.append(str.sprintf(";%d", i+1));
    }
    cbChannel.value = res;

    inputs << cbChannel;

    ProcessingPluginInput inputCalcValue;
    inputCalcValue.type = ProcessingPluginInput::DOUBLE_FIELD;
    inputCalcValue.value = calcvalue;
    inputCalcValue.minValue = -1E25;
    inputCalcValue.maxValue = 1E25;
    inputCalcValue.labelText = "Value: ";
    inputCalcValue.identifier = "cValue";
    inputCalcValue.tooltip = "Value used for operation";

    inputs << inputCalcValue;
}


/**
 * @brief Arithmetic::getName implementation of virtual function
 * @return name of plugin
 */
QString Arithmetic::getName()
{
    return pluginName;
}


/**
 * @brief Arithmetic::setInputs overrides virtual function
 * @param inputs list of Processing plugin inputs
 */
void Arithmetic::setFromInputs()
{
    // assign members

    operation   = inputs.getValue("cbOperation").toString();
    chId        = inputs.getValue("cbChannel").toString();
    calcvalue   = inputs.getValue("cValue").toDouble();

    if(operation == "division" && calcvalue == 0)
        calcvalue = 1.0;
    // TODO: throw error
}


/**
 * @brief Arithmetic::processSignal implements virtual function
 * @param in  input signal
 * @param out output signal
 * @return true if the signal has passed validation
 */
bool Arithmetic::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    // process selected channel only    
    if(chId == "all" || (chId != "all" && in.channelID == chId.toInt()))
    {
        int noPts = in.data.size();
        out.data.resize(noPts);

        // use arithmetic operation on every datapoint
        for(int i = 0; i < noPts; i++)
        {
            double v;
            if(operation == "multiplication")
                v = in.data.at(i) * calcvalue;
            else if(operation == "division")
                v = in.data.at(i) / calcvalue;
            else if(operation == "addition")
                v = in.data.at(i) + calcvalue;
            else if(operation == "subtraction")
                v = in.data.at(i) - calcvalue;
            else
                v = in.data.at(i);

            out.data[i] = v;
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
QString Arithmetic::getParameterPreview()
{
    QString str = "(Ch %1;val=%2;%3)";
    return str.arg(chId).arg(calcvalue).arg(operation);
}
