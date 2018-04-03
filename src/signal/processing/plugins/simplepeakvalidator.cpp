#include "simplepeakvalidator.h"

#include <QDebug>
#include "../processingchain.h"

QString SimplePeakValidator::descriptionFileName = "simplePeakValidator.html"; // TODO
QString SimplePeakValidator::iconFileName = "iconfile"; // TODO
QString SimplePeakValidator::pluginName = "Peak Validator";

QList<Signal::SType> SimplePeakValidator::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW << Signal::ABS << Signal::TEMPERATURE;


SimplePeakValidator::SimplePeakValidator(ProcessingChain *parentChain)  :   ProcessingPlugin(parentChain)
{
    // determine default threshold by signal type
    switch(parentChain->getSignalType())
    {
    case Signal::RAW:
        thresh = 1.0;
        break;
    case Signal::ABS:
        thresh = 2e8;
        break;
    case Signal::TEMPERATURE:
        thresh = 2500;
    }

    shortDescription = "Validator, sorts out all signals with peak values is below the threshold parameter.";

    // create input fields;
    ProcessingPluginInput inputMinValue;
    inputMinValue.type = ProcessingPluginInput::DOUBLE_FIELD;
    inputMinValue.value = thresh;
    inputMinValue.identifier = "thresh";
    inputMinValue.minValue = 0.0;
    inputMinValue.maxValue = 1e20;
    inputMinValue.labelText = "Threshold: ";
    inputMinValue.tooltip = "Threshold for validation";

    inputs << inputMinValue; // add value to input list

    // example code for creation of integer/checkbox inputfields

 /*   ProcessingPluginInput test;
    test.type = ProcessingPluginInput::INTEGER_FIELD;
    test.value = 5;
    test.identifier = "integer";
    test.minValue = 1;
    test.maxValue = 20;
    test.labelText = "Integer test: ";

    inputs << test; // add value to input list

    ProcessingPluginInput test2;
    test2.type = ProcessingPluginInput::CHECKBOX;
    test2.value = true;
    test2.identifier = "checkbox";
    test2.labelText = "Checkbox Test: ";

    inputs << test2;*/

}


/**
 * @brief SimplePeakValidator::getName implementation of virtual function
 * @return  name of plugin
 */
QString SimplePeakValidator::getName()
{
    return pluginName;
}


void SimplePeakValidator::setFromInputs()
{
    // assign members
    thresh = inputs.getValue("thresh").toDouble();
}


/**
 * @brief SimplePeakValidator::processSignal implements virtual function
 * @param in input signal
 * @param out output signal
 * @return true if the signal has passed validation
 */
bool SimplePeakValidator::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    int noPts = in.data.size();

    for(int i = 0; i < noPts-1; i ++)
    {
        if(in.data[i] > thresh )
        {
            out = in;
            return true;
        }
    }
    return false;
}


QString SimplePeakValidator::getParameterPreview()
{
    QString str;
    str.sprintf("(thresh=%.2e)",thresh);
    return str;
}

