#include "overwrite.h"

#include "../../mrun.h"
#include "../processingchain.h"
#include <QDebug>

QString Overwrite::descriptionFileName = "overwrite.html"; // TODO
QString Overwrite::iconFileName = "iconfile"; // TODO
QString Overwrite::pluginName = "Overwrite";

QList<Signal::SType> Overwrite::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW
        << Signal::ABS;
        //TODO << Signal::TEMPERATURE;


Overwrite::Overwrite(ProcessingChain *parentChain) :   ProcessingPlugin(parentChain)
{
    shortDescription = "Overwrites channel A with channel B";

    // standard values
    chId_A   = "1";
    chId_B   = "1";

    // create input fields;
    ProcessingPluginInput cbSource;
    cbSource.type = ProcessingPluginInput::COMBOBOX;

    // determine default values by signal type
    switch(parentChain->getSignalType())
    {
        case Signal::RAW:

            source   = "raw";
            cbSource.value = "raw;raw";
            break;

        case Signal::ABS:

            source   = "absolute";
            cbSource.value = "absolute;raw;absolute";
            break;
        default:
            source = "raw";
            cbSource.value = "raw;raw";
    }

    cbSource.labelText = "Source";
    cbSource.identifier = "cbSource";
    cbSource.tooltip = "Select signal type for source signal, channel B is then retrieved from this signal type";

    inputs << cbSource; // add value to input list

    ProcessingPluginInput cbChannel_A;
    cbChannel_A.type = ProcessingPluginInput::COMBOBOX;
    cbChannel_A.value = chId_A;
    cbChannel_A.labelText = "Channel A (overwritten)";
    cbChannel_A.identifier = "cbChannel_A";
    cbChannel_A.tooltip = "This channel will be overwritten by channel B from source (raw or absolute)";

    ProcessingPluginInput cbChannel_B;
    cbChannel_B.type = ProcessingPluginInput::COMBOBOX;
    cbChannel_B.value = chId_B;
    cbChannel_B.labelText = "Channel B";
    cbChannel_B.identifier = "cbChannel_B";
    cbChannel_B.tooltip = "This channel is retrieved from source (raw or absolute)";

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
 * @brief Overwrite::getName implementation of virtual function
 * @return name of plugin
 */
QString Overwrite::getName()
{
    return pluginName;
}


/**
 * @brief Overwrite::setInputs overrides virtual function
 * @param inputs list of Processing plugin inputs
 */
void Overwrite::setFromInputs()
{
    // assign members
    source   = inputs.getValue("cbSource").toString();
    chId_A        = inputs.getValue("cbChannel_A").toString();
    chId_B        = inputs.getValue("cbChannel_B").toString();
}


/**
 * @brief Overwrite::processSignal implements virtual function
 * @param in  input signal
 * @param out output signal
 * @return true if the signal has passed validation
 */
bool Overwrite::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    // process selected channel A only
    if(in.channelID == chId_A.toInt())
    {
        Signal s;

        // get channel B data
        if(source == "raw" && in.type == Signal::RAW)
        {
            s = in;
        }
        else if(source == "raw" && in.type == Signal::ABS)
        {
            ProcessingChain* pchain = mrun->getProcessingChain(Signal::RAW);
            if(pchain->msaPosition() > -1)
                s = pchain->getStepSignalPre(0, chId_B.toInt(), pchain->noPlugs()-1);
            else
                s = pchain->getStepSignalPre(mpIdx, chId_B.toInt(), pchain->noPlugs()-1);
        }
        else if(source == "absolute" && in.type == Signal::RAW)
        {
            // we cannot provide correct output data for this case
            // ouput signals of absolute chain have not been calculated yet
            s = in;
        }
        else // abs -> abs
        {
            s = in;
        }
        out = s;
    }
    else
    {
        // don't change other channels
        out = in;
    }

    if(out.data.isEmpty())
        return false;

    return true; // we do not make any validation here
}


/**
 * @brief Overwrite::getParameterPreview
 * @return
 */
QString Overwrite::getParameterPreview()
{
    QString str = "(A=%1;B=%2;source=%3)";
    return str.arg(chId_A).arg(chId_B).arg(source);
}
