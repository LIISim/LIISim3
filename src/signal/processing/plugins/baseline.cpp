#include "baseline.h"

#include "../../mrun.h"
#include <QDebug>

QString Baseline::descriptionFileName = "baseline.html"; // TODO
QString Baseline::iconFileName = "iconfile"; // TODO
QString Baseline::pluginName = "Baseline";

QList<Signal::SType> Baseline::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW << Signal::ABS;


/**
 * @brief Baseline::Baseline
 * @param parentChain
 */
Baseline::Baseline(ProcessingChain *parentChain) :   ProcessingPlugin(parentChain)
{
    shortDescription = "Baseline correction\n\
                       Select time range, which is used for calculation of correction factor.\n\
                       The signal is then subtracted by this correction factor.";

    // standard values
    operation   = "all signals";
    startAverage   = 0.0;
    endAverage     = 50.0;

    offsetAvailable = false;
    offset.clear();

    // pass stdev to next processing step
    preserveStdev = true;


    // create input fields;
    ProcessingPluginInput cbOperation;
    cbOperation.type = ProcessingPluginInput::COMBOBOX;
    cbOperation.value = "all signals;each signal;all signals;LIISettings";
    cbOperation.labelText = "Offset from";
    cbOperation.identifier = "cbOperation";
    cbOperation.tooltip = "All signals: one correction value is calculated from all signals\n"
                          "Each signal: correction values are calculated for each signal individually\n"
                          "LIISettings: offset values provided by the LIISettings are subtracted from the signals.";

    inputs << cbOperation; // add value to input list

    // show these only for "all signals" and "each signal"
    ProcessingPluginInput inputStart;
    inputStart.type = ProcessingPluginInput::DOUBLE_FIELD;
    inputStart.groupID = 1;
    inputStart.value = startAverage;
    inputStart.limits = false;
    inputStart.labelText = "Start [ns]";
    inputStart.identifier = "inputStart";
    inputStart.tooltip = "Select start time";

    inputs << inputStart;


    ProcessingPluginInput inputEnd;
    inputEnd.type = ProcessingPluginInput::DOUBLE_FIELD;
    inputEnd.groupID = 1;
    inputEnd.value = endAverage;
    inputEnd.limits = false;
    inputEnd.labelText = "End [ns]";
    inputEnd.identifier = "inputEnd";
    inputEnd.tooltip = "Select end time";

    inputs << inputEnd;   

    // show only parameters for selected operation
    onOperationChanged();
}


/**
 * @brief Baseline::getName implementation of virtual function
 * @return name of plugin
 */
QString Baseline::getName()
{
    return pluginName;
}


/**
 * @brief Baseline::setInputs overrides virtual function
 * @param inputs list of Processing plugin inputs
 */
void Baseline::setFromInputs()
{
    // if operation was changed, toggle visibility of parameter inputs
    if(inputs.getValue("cbOperation").toString() != operation)
        onOperationChanged();

    // assign members
    operation      = inputs.getValue("cbOperation").toString();
    startAverage   = inputs.getValue("inputStart").toDouble();
    endAverage     = inputs.getValue("inputEnd").toDouble();
}


/**
 * @brief Baseline::processSignal implements virtual function
 * @param in  input signal
 * @param out output signal
 * @return true if the signal has passed validation
 */
bool Baseline::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{        
    // make sure to not have parallel processing
    QMutexLocker lock(&variableMutex);

    int noPoints = in.data.size();

    if(noPoints == 0)
        return true;


    if(!offsetAvailable)
    {
        // init offset container
        if(offset.empty())
        {
            for(int i=0; i< mrun->getNoChannels(stype); i++)
                offset.append(0.0);
        }

        if(operation == "LIISettings")
        {            
            for(int i=0; i< mrun->getNoChannels(stype); i++)
                offset[i] =  mrun->liiSettings().channels.at(i).offset;

            offsetAvailable = true;
        }
        else
        {
            // copy input parameters, to avoid changing the user input!
            double avg_start    = startAverage * 1E-9;   // [ns] -> [s]
            double avg_end      = endAverage * 1E-9;     // [ns] -> [s]

            if(!in.hasDataAt(avg_end))
                avg_end = in.maxTime(); //convert [s] to [ns]

            // validate signal start time
            if(!in.hasDataAt(avg_start))
                avg_start = in.start_time;


            //normal processing
            if(operation == "each signal")
            {
                Signal s = in;
                offset[in.channelID-1] =  s.calcRangeAverage(avg_start, avg_end);
            }
            //calculate global offset for all signals of this mrun
            else
            {
                offsetAvailable = true;

                QList<int> chids = mrun->channelIDs(stype);

                // iterate through all channels
                for( int c = 0; c < chids.size(); c++)
                {
                    // set avg counter to zero
                    int counter = 0;

                    // iterate through all available mpoints
                    for( int i = 0; i < mrun->sizeAllMpoints(); i++)
                    {
                        // include only signals which passed validation in previous step
                        if( validAtPreviousStep(i) )
                        {
                            // get signal of mpoint i with channelId c+1 at previous position in chain
                            Signal s = processedSignalPreviousStep(i, chids[c]);

                            // sum avg value for all mpoints
                            offset[c] += s.calcRangeAverage(avg_start, avg_end);
                            counter++;
                        }
                    }
                    offset[c] /= double(counter);
                }
            }
        }
    }

    // subtract offset value from signal
    for(int i = 0; i < noPoints; i++)
    {
        double v = in.data.at(i) - offset.at(in.channelID-1);

        out.data.append(v);
    }

    return true; // we do not make any validation here
}


/**
 * @brief Baseline::getParameterPreview
 * @return
 */
QString Baseline::getParameterPreview()
{
    if(inputs.getValue("cbOperation").toString() == "LIISettings")
    {
        QString str = "(offset from LIISettings)";
        return str;
    }
    else
    {
        QString str = "(%1;avg=%2 to %3 ns)";
        return str.arg(operation).arg(startAverage).arg(endAverage);
    }
}



/**
 * @brief Baseline::reset resets global vars after signals are processed (ill conditioned otherwise)
 */
void Baseline::reset()
{
    QMutexLocker lock(&variableMutex);
    ProcessingPlugin::reset();
    offset.clear();
    offsetAvailable = false;
}


/**
 * @brief Baseline::onOperationChanged change visible inputs when operation is changed
 */
void Baseline::onOperationChanged()
{
    if(inputs.getValue("cbOperation").toString() == "LIISettings")
    {
        inputs.showGroup(2);
    }
    else
    {
        inputs.showGroup(1);
    }

    // signal 7: input fields modified (see also: ProcessingPlugin.h)
    emit dataChanged(7);
}
