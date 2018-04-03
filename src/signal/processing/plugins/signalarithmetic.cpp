#include "signalarithmetic.h"

#include "../processingchain.h"
#include "../../mrun.h"
#include <QDebug>

QString SignalArithmetic::descriptionFileName = "signalarithmetic.html"; // TODO
QString SignalArithmetic::iconFileName = "iconfile"; // TODO
QString SignalArithmetic::pluginName = "SignalArithmetic";

QList<Signal::SType> SignalArithmetic::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW << Signal::ABS;


SignalArithmetic::SignalArithmetic(ProcessingChain *parentChain) :   ProcessingPlugin(parentChain)
{
    shortDescription = "Apply arithmetic operations on each datapoint";

    // standard values
    operation   = "multiplication (A*B)";
    chId_A        = "1";
    chId_B        = "2";

    // create input fields;
    ProcessingPluginInput cbOperation;
    cbOperation.type = ProcessingPluginInput::COMBOBOX;
    cbOperation.value = QString("multiplication (A*B);")    //standard value
                      + "multiplication (A*B);"
                      + "division (A/B);"
                      + "division (B/A);"
                      + "addition (A+B);"
                      + "subtraction (A-B);"
                      + "subtraction (B-A)";

    cbOperation.labelText = "Operation";
    cbOperation.identifier = "cbOperation";
    cbOperation.tooltip = "Mathematical operation";

    inputs << cbOperation; // add value to input list

    ProcessingPluginInput cbChannel_A;
    cbChannel_A.type = ProcessingPluginInput::COMBOBOX;
    cbChannel_A.value = chId_A;
    cbChannel_A.labelText = "Channel A (overwritten)";
    cbChannel_A.identifier = "cbChannel_A";
    cbChannel_A.tooltip = "Set channel A for operation";

    ProcessingPluginInput cbChannel_B;
    cbChannel_B.type = ProcessingPluginInput::COMBOBOX;
    cbChannel_B.value = chId_B;
    cbChannel_B.labelText = "Channel B";
    cbChannel_B.identifier = "cbChannel_B";
    cbChannel_B.tooltip = "Set channel B for operation";

    // fill combobox with values
    int numCh = channelCount();

    QString str,res;

    for(int i=0; i < numCh; i++)
    {
        res.append(str.sprintf(";%d", i+1));
    }
    cbChannel_A.value = "1" + res;
    cbChannel_B.value = "2" + res;

    inputs << cbChannel_A;
    inputs << cbChannel_B;
}


/**
 * @brief SignalArithmetic::getName implementation of virtual function
 * @return name of plugin
 */
QString SignalArithmetic::getName()
{
    return pluginName;
}


/**
 * @brief SignalArithmetic::setInputs overrides virtual function
 * @param inputs list of Processing plugin inputs
 */
void SignalArithmetic::setFromInputs()
{
    // assign members

    operation   = inputs.getValue("cbOperation").toString();
    chId_A        = inputs.getValue("cbChannel_A").toString();
    chId_B        = inputs.getValue("cbChannel_B").toString();
}

/**
 * @brief SignalArithmetic::processSignal implements virtual function
 * @param in  input signal
 * @param out output signal
 * @return true if the signal has passed validation
 */
bool SignalArithmetic::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    // process selected channel only
    if(in.channelID == chId_A.toInt())
    {
        int noPts = in.data.size();

        // get signal of mpoint mpIdx with channelId= chId_B at previous position in chain
        ProcessingChain* pchain = mrun->getProcessingChain(in.type);
        Signal s = pchain->getStepSignalPre(mpIdx, chId_B.toInt(), positionInChain-1);

        // use arithmetic operation on every datapoint
        for(int i = 0; i < noPts; i++)
        {
            double v;
            if(operation == "multiplication (A*B)")
                v = in.data.at(i) * s.data.at(i);
            else if(operation == "division (A/B)")
                v = in.data.at(i) / s.data.at(i);
            else if(operation == "division (B/A)")
                v = s.data.at(i) / in.data.at(i);
            else if(operation == "addition (A+B)")
                v = in.data.at(i) + s.data.at(i);
            else if(operation == "subtraction (A-B)")
                v = in.data.at(i) - s.data.at(i);
            else if(operation == "subtraction (B-A)")
                v = s.data.at(i) - in.data.at(i);
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
QString SignalArithmetic::getParameterPreview()
{
    QString str_op;

    if(operation == "multiplication (A*B)")
        str_op = "(A*B)";
    else if(operation == "division (A/B)")
       str_op = "(A/B)";
    else if(operation == "division (B/A)")
       str_op = "(B/A)";
    else if(operation == "addition (A+B)")
       str_op = "(A+B)";
    else if(operation == "subtraction (A-B)")
        str_op = "(A-B)";
    else if(operation == "subtraction (B-A)")
        str_op = "(B-A)";
    else
        str_op = "(A*B)";

    QString str = "(A=%1;B=%2;%3)";
    return str.arg(chId_A).arg(chId_B).arg(str_op);
}
