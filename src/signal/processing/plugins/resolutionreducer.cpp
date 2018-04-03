#include "resolutionreducer.h"

QString ResolutionReducer::descriptionFileName = "resolutionreducer.html"; // TODO
QString ResolutionReducer::iconFileName = "iconfile"; // TODO
QString ResolutionReducer::pluginName = "Resolution Reducer";

QList<Signal::SType> ResolutionReducer::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW
        << Signal::ABS
        << Signal::TEMPERATURE;

ResolutionReducer::ResolutionReducer(ProcessingChain *parentChain) :   ProcessingPlugin(parentChain)
{
    shortDescription = "Reduces vertical resolution of signal (for simulation of various instrument resolutions)";

    range = 0.1;
    resolution = "8";


    ProcessingPluginInput cbResolution;
    cbResolution.type = ProcessingPluginInput::COMBOBOX;
    cbResolution.value = resolution + ";4;5;6;7;8;9;10;11;12;13;14;15;16";
    cbResolution.labelText = "Resolution";
    cbResolution.identifier = "cbResolution";
    cbResolution.tooltip = "Select desired resolution in bit";

    inputs << cbResolution;

    ProcessingPluginInput inputRangeValue;
    inputRangeValue.type = ProcessingPluginInput::DOUBLE_FIELD;
    inputRangeValue.value = range;
    inputRangeValue.minValue = 0.0;
    inputRangeValue.maxValue = 1E12;
    inputRangeValue.labelText = "Range: ";
    inputRangeValue.identifier = "cRange";
    inputRangeValue.tooltip = "Set vertical resolution range, this range is then mapped to the selected resolution";

    inputs << inputRangeValue;

}

/**
 * @brief ResolutionReducer::getName implementation of virtual function
 * @return name of plugin
 */
QString ResolutionReducer::getName()
{
    return pluginName;
}


/**
 * @brief ResolutionReducer::getParameterPreview
 * @return
 */
QString ResolutionReducer::getParameterPreview()
{
    QString str = "(Res: %1 bit; Range: %2)";
    return str.arg(resolution).arg(range);
}



/**
 * @brief ResolutionReducer::setInputs overrides virtual function
 * @param inputs list of Processing plugin inputs
 */
void ResolutionReducer::setFromInputs()
{
    // assign members
    resolution = inputs.getValue("cbResolution").toString();
    range      = inputs.getValue("cRange").toDouble();
}


/**
 * @brief ResolutionReducer::processSignal implements virtual function
 * @param in  input signal
 * @param out output signal
 * @return true if the signal has passed validation
 */
bool ResolutionReducer::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    int noPts = in.data.size();

    // use arithmetic operation on every datapoint
    for(int i = 0; i < noPts; i++)
    {
        double y, v;
        y = in.data.at(i);

        double stepSize = (range) / pow(2.0, resolution.toDouble());

        v = round(y / stepSize) * stepSize;

        out.data.append(v);
    }

    return true;
}
