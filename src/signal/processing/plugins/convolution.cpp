#include "convolution.h"

#include "../../mrun.h"
#include <QDebug>

QString Convolution::descriptionFileName = "Convolution.html"; // TODO
QString Convolution::iconFileName = "iconfile"; // TODO
QString Convolution::pluginName = "Convolution";

QList<Signal::SType> Convolution::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW
        << Signal::ABS
        << Signal::TEMPERATURE;


/**
 * @brief Convolution::Convolution
 * @param parentChain
 */
Convolution::Convolution(ProcessingChain *parentChain) :   ProcessingPlugin(parentChain)
{    
    shortDescription = "Apply convolution window on each datapoint";

    // standard values
    chId        = "all";
    //calcvalue   = 0.0;

        ProcessingPluginInput cbChannel;
        cbChannel.type = ProcessingPluginInput::COMBOBOX;
        cbChannel.value = "all";
        cbChannel.labelText = "Channel";
        cbChannel.identifier = "cbChannel";
        cbChannel.tooltip = "Select channels for this operation";

        // fill combobox with values
        int numCh = channelCount();

        QString str,res;

        // add option "all"
        res.append(";all");

        for(int i=0; i < numCh; i++)
        {
            res.append(str.sprintf(";%d", i+1));
        }
        cbChannel.value = res;

    //inputs << cbChannel;

        ProcessingPluginInput inputCalcValue;
        inputCalcValue.type = ProcessingPluginInput::DOUBLE_FIELD;
        inputCalcValue.value = calcvalue;
        inputCalcValue.minValue = -1E12;
        inputCalcValue.maxValue = 1E12;
        inputCalcValue.labelText = "Value: ";
        inputCalcValue.identifier = "cValue";
        inputCalcValue.tooltip = "";

    //inputs << inputCalcValue;
}


/**
 * @brief Convolution::getName implementation of virtual function
 * @return name of plugin
 */
QString Convolution::getName()
{
    return pluginName;
}


/**
 * @brief Convolution::setInputs overrides virtual function
 * @param inputs list of Processing plugin inputs
 */
void Convolution::setFromInputs()
{
    // assign members
    chId        = inputs.getValue("cbChannel").toString();
    //calcvalue   = inputs.getValue("cValue").toDouble();
}


/**
 * @brief Convolution::processSignal implements virtual function
 * @param in  input signal
 * @param out output signal
 * @return true if the signal has passed validation
 */
bool Convolution::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
//    // process selected channel only
//    if(chId == "all" || (chId != "all" && in.channelID == chId.toInt()))
//    {
//        int noPts = in.data.size();

//        // use arithmetic operation on every datapoint
//        for(int i = 0; i < noPts; i++)
//        {
//            double v;
//            if(operation == "multiplication")
//                v = in.data.at(i) * calcvalue;
//            else if(operation == "addition")
//                v = in.data.at(i) + calcvalue;
//            else
//                v = in.data.at(i);

//            out.data.append(v);
//        }
//    }
//    else
//    {
//        // don't change other channels
//        out = in;
//    }


    out = in;
    return true; // we do not make any validation here
}


/**
 * @brief Convolution::getParameterPreview
 * @return
 */
QString Convolution::getParameterPreview()
{
    QString str = "(Ch %1)";
    return str.arg(chId);
}
