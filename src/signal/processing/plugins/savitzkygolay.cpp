#include "savitzkygolay.h"


QString SavitzkyGolay::descriptionFileName = "savitzkygolay.html"; // TODO
QString SavitzkyGolay::iconFileName = "iconfile"; // TODO
QString SavitzkyGolay::pluginName = "Savitzky-Golay";

QList<Signal::SType> SavitzkyGolay::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW << Signal::ABS << Signal::TEMPERATURE;

SavitzkyGolay::SavitzkyGolay(ProcessingChain *parentChain) :   ProcessingPlugin(parentChain)
{
    //https://en.wikipedia.org/wiki/Savitzky%E2%80%93Golay_filter
    shortDescription = "SavitzkyGolay filter for smoothing data";



}

/**
 * @brief SavitzkyGolay::getName implementation of virtual function
 * @return name of plugin
 */
QString SavitzkyGolay::getName()
{
    return pluginName;
}


/**
 * @brief SavitzkyGolay::setInputs overrides virtual function
 * @param inputs list of Processing plugin inputs
 */
void SavitzkyGolay::setFromInputs()
{
}


/**
 * @brief SavitzkyGolay::getParameterPreview
 * @return
 */
QString SavitzkyGolay::getParameterPreview()
{
    QString str = "()";
    return str;
}

/**
 * @brief SavitzkyGolay::processSignal implements virtual function
 * @param in  input signal
 * @param out output signal
 * @return true if the signal has passed validation
 */
bool SavitzkyGolay::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    out = in;
    return true; // we do not make any validation here
}
