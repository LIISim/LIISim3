#include "swapchannels.h"

#include "../../mrun.h"
#include "../processingchain.h"
#include <QDebug>

QString SwapChannels::descriptionFileName = "swapchannels.html"; // TODO
QString SwapChannels::iconFileName = "iconfile"; // TODO
QString SwapChannels::pluginName = "Swap channels";

QList<Signal::SType> SwapChannels::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW
        << Signal::ABS;

SwapChannels::SwapChannels(ProcessingChain *parentChain) :   ProcessingPlugin(parentChain)
{
    shortDescription = "Swap channel A and channel B";

    // standard values
    chId_A   = "1";
    chId_B   = "2";

    // pass stdev to next processing step
    preserveStdev = true;

    // create input fields;
    ProcessingPluginInput cbChannel_A;
    cbChannel_A.type = ProcessingPluginInput::COMBOBOX;
    cbChannel_A.value = chId_A;
    cbChannel_A.labelText = "Channel A";
    cbChannel_A.identifier = "cbChannel_A";
    cbChannel_A.tooltip = "Channel A";

    ProcessingPluginInput cbChannel_B;
    cbChannel_B.type = ProcessingPluginInput::COMBOBOX;
    cbChannel_B.value = chId_B;
    cbChannel_B.labelText = "Channel B";
    cbChannel_B.identifier = "cbChannel_B";
    cbChannel_B.tooltip = "Channel B";

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
 * @brief SwapChannels::getName implementation of virtual function
 * @return name of plugin
 */
QString SwapChannels::getName()
{
    return pluginName;
}


/**
 * @brief SwapChannels::setInputs overrides virtual function
 * @param inputs list of Processing plugin inputs
 */
void SwapChannels::setFromInputs()
{
    // assign members
    chId_A        = inputs.getValue("cbChannel_A").toString();
    chId_B        = inputs.getValue("cbChannel_B").toString();
}


/**
 * @brief SwapChannels::processSignal implements virtual function
 * @param in  input signal
 * @param out output signal
 * @return true if the signal has passed validation
 */
bool SwapChannels::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    Signal s;

    // process selected channel A only
    if(in.channelID == chId_A.toInt())
    {
        ProcessingChain* pchain = mrun->getProcessingChain(in.type);
        s = pchain->getStepSignalPre(mpIdx, chId_B.toInt(), positionInChain-1);
        out = s;
    }
    else if(in.channelID == chId_B.toInt())
    {
        ProcessingChain* pchain = mrun->getProcessingChain(in.type);
        s = pchain->getStepSignalPre(mpIdx, chId_A.toInt(), positionInChain-1);
        out = s;
    }
    else
    {
        // don't change other channels
        out = in;
    }
    return true; // we do not make any validation here
}


/**
 * @brief SwapChannels::getParameterPreview
 * @return
 */
QString SwapChannels::getParameterPreview()
{
    QString str = "(A=%1;B=%2)";
    return str.arg(chId_A).arg(chId_B);
}
