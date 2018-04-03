#include "getsignalsection.h"

#include <QDebug>

QString GetSignalSection::descriptionFileName = "getSignalSection.html"; // TODO
QString GetSignalSection::iconFileName = "iconfile"; // TODO
QString GetSignalSection::pluginName = "Get Section";

QList<Signal::SType> GetSignalSection::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW << Signal::ABS << Signal::TEMPERATURE;

GetSignalSection::GetSignalSection(ProcessingChain *parentChain) :   ProcessingPlugin(parentChain)
{
    shortDescription = "Get section of signal";

    // pass stdev to next processing step
    preserveStdev = true;

    // if mrun contains signal data, setup some appropriate default parameters
    if(mrun->sizeAllMpoints() > 0 )
    {
        // get first MPoint (id=0) -> first channel (chID=0) // !!! NOT NICE
        int size            = mrun->getPre(0)->getSignal(1, stype).data.size();
        double dt           = mrun->getPre(0)->getSignal(1, stype).dt * 1E9; // [s] -> [ns]
        double start_time   = mrun->getPre(0)->getSignal(1, stype).start_time * 1E9; // [s] -> [ns

        // set default values
        get_decay    = false;
        reset_time   = false;
        signal_start = start_time;  // [ns]
        signal_end   = start_time + int((size - 1) * dt); // [ns]
    }
    else // set default defaults :D
    {
        // set default values
        get_decay    = false;
        reset_time   = false;
        signal_start = 0.0;  // [ns]
        signal_end   = 100000.0; // [ns]
    }


    // create input fields;

    ProcessingPluginInput inputGetDecay;
    inputGetDecay.type = ProcessingPluginInput::CHECKBOX;
    inputGetDecay.value = get_decay;
    inputGetDecay.identifier = "GetDecay";
    inputGetDecay.labelText = "Start from peak";
    inputGetDecay.tooltip   = "Automatically finds peak and sets starting point to peak time";

    ProcessingPluginInput inputResetTime;
    inputResetTime.type = ProcessingPluginInput::CHECKBOX;
    inputResetTime.value = reset_time;
    inputResetTime.identifier = "ResetTime";
    inputResetTime.labelText = "Reset time offset";
    inputResetTime.tooltip   = "Signal start time is set to zero";

    ProcessingPluginInput inputSigStart;
    inputSigStart.type = ProcessingPluginInput::DOUBLE_FIELD;
    inputSigStart.value = signal_start;
    inputSigStart.identifier = "SigStart";
    inputSigStart.limits = false;
    inputSigStart.labelText = "Start [ns]: ";
    inputSigStart.tooltip = "Set start time";

    ProcessingPluginInput inputSigEnd;
    inputSigEnd.type = ProcessingPluginInput::DOUBLE_FIELD;
    inputSigEnd.value = signal_end;
    inputSigEnd.identifier = "SigEnd";
    inputSigEnd.limits = false;
    inputSigEnd.labelText = "End [ns]: ";
    inputSigEnd.tooltip = "Set end time";

    // add values to input list
    inputs << inputGetDecay;
    inputs << inputResetTime;
    inputs << inputSigStart;
    inputs << inputSigEnd;

    executeSyncronized = false;
}


/**
 * @brief GetSignalSection::getName implementation of virtual function
 * @return name of plugin
 */
QString GetSignalSection::getName()
{
    return pluginName;
}


void GetSignalSection::setFromInputs()
{
    // assign members
    get_decay       = inputs.getValue("GetDecay").toBool();
    reset_time      = inputs.getValue("ResetTime").toBool();
    signal_start    = inputs.getValue("SigStart").toDouble();
    signal_end      = inputs.getValue("SigEnd").toDouble();
}


bool GetSignalSection::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    // copy input parameters, to avoid changing the user input!
    double signal_start = this->signal_start;   // [ns]
    double signal_end = this->signal_end;       // [ns]

    int noPoints = in.data.size();

    // skip processing if mpoint is empty
    if(noPoints == 0) return true;

    int idx_start;
    int idx_end;

    // validate signal_end time
    if(!in.hasDataAt(signal_end * 1e-9))
        signal_end = in.maxTime() * 1e9; //convert [s] to [ns]

    // convert user input to idx
    if(get_decay == true)
    {
        idx_start = in.getMaxIndex() ;

        // overwrite signal_start
        signal_start = in.start_time * 1E9 + idx_start * in.dt * 1E9;
    }
    else
    {

        // validate signal start time
        if(!in.hasDataAt(signal_start * 1e-9))
            signal_start = in.start_time * 1e9;

        idx_start = int((signal_start - (in.start_time*1E9) ) / (in.dt*1E9));
    }

    idx_end     = int((signal_end - (in.start_time*1E9) ) / (in.dt*1E9));

    double v, nstdev;

    // validate indices
    if(idx_start < 0)           idx_start = 0;
    if(idx_start >= noPoints)   idx_start = noPoints - 1;
    if(idx_end >= noPoints)     idx_end = noPoints - 1;
    if(idx_end < idx_start)     idx_end = idx_start = 0;
    if(idx_end < 0)             idx_end = 0;

    for(int i = idx_start; i <= idx_end; i++)
    {
        v = in.data.at(i);
        out.data.append(v);
    }

    if(!in.stdev.isEmpty())
    {
        noPoints = in.stdev.size();
        if(idx_start < 0)           idx_start = 0;
        if(idx_start >= noPoints)   idx_start = noPoints - 1;
        if(idx_end >= noPoints)     idx_end = noPoints - 1;
        if(idx_end < idx_start)     idx_end = idx_start = 0;
        if(idx_end < 0)             idx_end = 0;

        for(int i = idx_start; i <= idx_end; i++)
        {
            nstdev = in.stdev.at(i);
            out.stdev.append(nstdev);
        }
    }

    if(reset_time == true)
        out.start_time = 0.0;
    else
        out.start_time = in.start_time + idx_start * in.dt;

    return true; // we do not make any validation here
}


QString GetSignalSection::getParameterPreview()
{
    QString str ;
    str.sprintf("(start=%g;end=%g)",signal_start, signal_end);

    return str;
}
