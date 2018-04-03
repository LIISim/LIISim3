#include "simpledatareducer.h"



QString SimpleDataReducer::descriptionFileName = "simpleDataReducer.html"; // TODO
QString SimpleDataReducer::iconFileName = "iconfile"; // TODO
QString SimpleDataReducer::pluginName = "Simple Data Reducer";

QList<Signal::SType> SimpleDataReducer::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW << Signal::ABS << Signal::TEMPERATURE;

/**
 * @brief SimpleDataReducer::SimpleDataReducer
 * @param parent parent object (default = 0)
 */
SimpleDataReducer::SimpleDataReducer(ProcessingChain *parentChain) :   ProcessingPlugin(parentChain)
{
    shortDescription = "Reduce the amout of datapoints by increasing the time step between data points";

    dtFactor = 2;

    // create input fields;
    ProcessingPluginInput inputDtFactor;
    inputDtFactor.type = ProcessingPluginInput::INTEGER_FIELD;
    inputDtFactor.value = dtFactor;
    inputDtFactor.identifier = "dtFactor";
    inputDtFactor.minValue = 1;
    inputDtFactor.maxValue = 50;
    inputDtFactor.labelText = "Skip data points: ";
    inputDtFactor.tooltip = "Define number of data points that are skipped";

    inputs << inputDtFactor; // add value to input list
}


/**
 * @brief SimpleDataReducer::getName implementation of virtual function
 * @return name of plugin
 */
QString SimpleDataReducer::getName()
{
    return pluginName;
}


/**
 * @brief SimpleDataReducer::setInputs overrides virtual function
 * @param inputs list of Processing plugin inputs
 */
void SimpleDataReducer::setFromInputs()
{
    // assign members
    dtFactor = inputs.getValue("dtFactor").toInt();

    // avoid reset to one if necessary!
    if(dtFactor < 0) dtFactor = 0;
}


/**
 * @brief SimpleDataReducer::processSignal implements virtual function
 * @param in  input signal
 * @param out output signal
 * @return true if the signal has passed validation
 */
bool SimpleDataReducer::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    int noPts = in.data.size();

    // okay, we could do something more special here than simply skipping
    // data points :D ! TODO !!!
    for(int i = 0; i < noPts-1; i += (dtFactor+1))
    {
        double v = in.data.at(i);
        out.data.append(v);
    }

    out.dt = in.dt * (dtFactor+1);
    return true; // we do not make any validation here
}


QString SimpleDataReducer::getParameterPreview()
{
    QString str ;
    str.sprintf("(skip=%d)",dtFactor);

    return str;
}
