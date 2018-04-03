#include "transfer.h"

#include "../../mrun.h"
#include "../processingchain.h"

QString Transfer::pluginName = "Transfer";

QList<Signal::SType> Transfer::supportedSignalTypes = QList<Signal::SType>() << Signal::ABS;

Transfer::Transfer(ProcessingChain *parentChain) : ProcessingPlugin(parentChain)
{
    shortDescription = "Transfers the PROCESSED signals from raw to absolute or in reverse.\nOverwrites any signal data in the destination processing chain.";

    destinationSType = parentChain->getSignalType();
}


QString Transfer::getName()
{
    return pluginName;
}


bool Transfer::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    ProcessingChain *pchain = mrun->getProcessingChain(Signal::RAW);

    if(pchain && pchain->isValid(mpIdx))
    {
        if(pchain->msaPosition() > -1)
            out = pchain->getStepSignalPre(0, in.channelID, pchain->noPlugs() - 1);
        else
            out = pchain->getStepSignalPre(mpIdx, in.channelID, pchain->noPlugs() - 1);

        return true;
    }

    return false;
}


void Transfer::setFromInputs()
{

}


QString Transfer::getParameterPreview()
{
    if(destinationSType == Signal::ABS)
        return "(source: raw signal)";
    else
        return "(ERROR: Wrong signal type! Please remove the plugin!)";
}
