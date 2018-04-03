#include "xshiftsignals.h"
#include "../processingchain.h"

#include <QMutexLocker>

QString XShiftSignals::descriptionFileName = "xshiftsignals.html"; // TODO
QString XShiftSignals::iconFileName = "iconfile"; // TODO
QString XShiftSignals::pluginName = "x-shift";

QList<Signal::SType> XShiftSignals::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW << Signal::ABS;

XShiftSignals::XShiftSignals(ProcessingChain *parentChain) :   ProcessingPlugin(parentChain)
{
    shortDescription = "X-shift signals";
    executeSyncronized = false;
}


/**
 * @brief XShiftSignals::getName implementation of virtual function
 * @return name of plugin
 */
QString XShiftSignals::getName()
{
    return pluginName;
}


void XShiftSignals::setFromInputs()
{

}


QString XShiftSignals::getParameterPreview()
{
    QString str = "";
    return str;
}


bool XShiftSignals::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    QMutexLocker lock(&variableMutex);

    //do calculations only for first channel
    if(in.channelID == 1)
    {

        shiftSignals[mpIdx].resize(channelCount());

        ProcessingChain* pchain = mrun->getProcessingChain(this->stype);

        std::vector<int> xpos;
        int lowest_xpos;
        int lowest_xpos_ch;

        xpos.resize(channelCount());

        // iterate through all channels
        for( int c = 0; c < channelCount(); c++)
        {
            // get signal of mpoint mpIdx with channelId c+1 at previous position in chain
            Signal s = pchain->getStepSignalPre(mpIdx, c+1, positionInChain-1);

            shiftSignals[mpIdx][c] = s;

            // find and save peak position
            xpos[c] = s.getMaxIndex();

            if(c == 0)
            {
                lowest_xpos = xpos[0];
                lowest_xpos_ch = 0;
            }
            else if (xpos[c] < xpos[c-1])
            {
                lowest_xpos = xpos[c];
                lowest_xpos_ch = c;
            }
        }

        // iterate again through all channels and save shifted signals
        for( int c = 0; c < channelCount(); c++)
        {
            // calculate new xpos
            if(c != lowest_xpos_ch)
            {
                xpos[c] = xpos[c] - lowest_xpos;
            }
            else
            {
                continue;
            }
            Signal s = pchain->getStepSignalPre(mpIdx, c+1, positionInChain-1);

            // overwrite signal
            for(int i = 0; i < (shiftSignals[mpIdx][c].data.size() - xpos[c]); i++)
            {
                shiftSignals[mpIdx][c].data[i] = s.data[i+xpos[c]];
            }
        }
    }

    out = shiftSignals[mpIdx][in.channelID-1];

    return true; // we do not make any validation here
}


/**
 * @brief XShiftSignals::reset resets plugin state
 * @details clear results
 */
void XShiftSignals::reset()
{
    QMutexLocker lock(&variableMutex);
    ProcessingPlugin::reset();
    //shiftSignals.clear();
}

