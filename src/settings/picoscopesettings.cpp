#include "picoscopesettings.h"
#include <QDebug>
#include "general/LIISimException.h"

PicoScopeSettings::PicoScopeSettings(QObject *parent) : SettingsBase(parent)
{
    groupName = "PicoScopeSettings";

    key_averagingBufferSize = "averagingBufferSize";
    key_linkSettings = "linkSettings";

    key_channelAenabled = "channelA";
    key_channelBenabled = "channelB";
    key_channelCenabled = "channelC";
    key_channelDenabled = "channelD";
    key_range = "range";
    key_coupling = "coupling";
    key_timebase = "timebase";
    key_offset = "offset";
    key_collectionTime = "collectionTime";
    //key_dt = "dt";
    key_captures = "captures";

    key_presamplepercentage = "presamplePercentage";

    v_samples        = 1;
    v_timeInterval   = 0.0;
    v_sampleInterval = 0.0;

    channelATrigger = PS6000_CONDITION_DONT_CARE;
    channelBTrigger = PS6000_CONDITION_DONT_CARE;
    channelCTrigger = PS6000_CONDITION_DONT_CARE;
    channelDTrigger = PS6000_CONDITION_DONT_CARE;
    channelAuxTrigger = PS6000_CONDITION_TRUE;

    group_channelA = "chA/";
    group_channelB = "chB/";
    group_channelC = "chC/";
    group_channelD = "chD/";
    group_channelAUX = "chAUX/";

    key_triggerState = "triggerState";
    key_triggerMode = "triggerMode";
    key_triggerDirection = "triggerDirection";

    key_thresholdUpper = "upperThreshold";
    key_upperHysteresis = "upperHysteresis";
    key_thresholdLower = "lowerThreshold";
    key_lowerHysteresis = "lowerHysteresis";

    key_autoTriggerMs = "autoTriggerMs";

    /***
     * PSStreamingMode::SingleMeasurement,
     * PSStreamingMode::AverageMeasurement
     * PSStreamingMode::Both
     */
    streaming_mode = PSStreamingMode::Both;

    init();
}


PicoScopeSettings::~PicoScopeSettings()
{

}


void PicoScopeSettings::init()
{
    if(!settings.contains(key_averagingBufferSize))
        settings.insert(key_averagingBufferSize, 1);

    if(!settings.contains(key_linkSettings))
        settings.insert(key_linkSettings, true);

    if(!settings.contains(key_channelAenabled))
        settings.insert(key_channelAenabled, true);

    if(!settings.contains(key_channelBenabled))
        settings.insert(key_channelBenabled, true);

    if(!settings.contains(key_channelCenabled))
        settings.insert(key_channelCenabled, true);

    if(!settings.contains(key_channelDenabled))
        settings.insert(key_channelDenabled, true);

    if(!settings.contains(group_channelA + key_range))
        setRange(PSChannel::A, PSRange::R5V);
    if(!settings.contains(group_channelB + key_range))
        setRange(PSChannel::B, PSRange::R5V);
    if(!settings.contains(group_channelC + key_range))
        setRange(PSChannel::C, PSRange::R5V);
    if(!settings.contains(group_channelD + key_range))
        setRange(PSChannel::D, PSRange::R5V);

    if(!settings.contains(key_coupling))
        setCoupling(PSCoupling::DC50R);

    if(!settings.contains(key_timebase))
        settings.insert(key_timebase, 0);

    if(!settings.contains(key_offset))
        settings.insert(key_offset, 0);

    if(!settings.contains(group_channelA + key_offset))
        setOffset(PSChannel::A, 0.1f);
    if(!settings.contains(group_channelB + key_offset))
        setOffset(PSChannel::B, 0.2f);
    if(!settings.contains(group_channelC + key_offset))
        setOffset(PSChannel::C, 0.0);
    if(!settings.contains(group_channelD + key_offset))
        setOffset(PSChannel::D, 0.0);

    if(!settings.contains(key_collectionTime))
        settings.insert(key_collectionTime, 0.000001);

    //if(!settings.contains(key_dt))
    //    settings.insert(key_dt, 0.000001);

    if(!settings.contains(key_captures))
        settings.insert(key_captures, 50);

    if(!settings.contains(key_presamplepercentage))
        settings.insert(key_presamplepercentage, 20);

    //Trigger settings channel A
    if(!settings.contains(group_channelA + key_triggerState))
        settings.insert(group_channelA + key_triggerState, PS6000_CONDITION_DONT_CARE);

    if(!settings.contains(group_channelA + key_triggerMode))
        settings.insert(group_channelA + key_triggerMode, PS6000_LEVEL);

    if(!settings.contains(group_channelA + key_triggerDirection))
        settings.insert(group_channelA + key_triggerDirection, PS6000_ABOVE);

    if(!settings.contains(group_channelA + key_thresholdUpper))
        settings.insert(group_channelA + key_thresholdUpper, 0);

    if(!settings.contains(group_channelA + key_upperHysteresis))
        settings.insert(group_channelA + key_upperHysteresis, 0);

    if(!settings.contains(group_channelA + key_thresholdLower))
        settings.insert(group_channelA + key_thresholdLower, 0);

    if(!settings.contains(group_channelA + key_lowerHysteresis))
        settings.insert(group_channelA + key_lowerHysteresis, 0);

    //Trigger settings channel B
    if(!settings.contains(group_channelB + key_triggerState))
        settings.insert(group_channelB + key_triggerState, PS6000_CONDITION_DONT_CARE);

    if(!settings.contains(group_channelB + key_triggerMode))
        settings.insert(group_channelB + key_triggerMode, PS6000_LEVEL);

    if(!settings.contains(group_channelB + key_triggerDirection))
        settings.insert(group_channelB + key_triggerDirection, PS6000_ABOVE);

    if(!settings.contains(group_channelB + key_thresholdUpper))
        settings.insert(group_channelB + key_thresholdUpper, 0);

    if(!settings.contains(group_channelB + key_upperHysteresis))
        settings.insert(group_channelB + key_upperHysteresis, 0);

    if(!settings.contains(group_channelB + key_thresholdLower))
        settings.insert(group_channelB + key_thresholdLower, 0);

    if(!settings.contains(group_channelB + key_lowerHysteresis))
        settings.insert(group_channelB + key_lowerHysteresis, 0);

    //Trigger settings channel C
    if(!settings.contains(group_channelC + key_triggerState))
        settings.insert(group_channelC + key_triggerState, PS6000_CONDITION_DONT_CARE);

    if(!settings.contains(group_channelC + key_triggerMode))
        settings.insert(group_channelC + key_triggerMode, PS6000_LEVEL);

    if(!settings.contains(group_channelC + key_triggerDirection))
        settings.insert(group_channelC + key_triggerDirection, PS6000_ABOVE);

    if(!settings.contains(group_channelC + key_thresholdUpper))
        settings.insert(group_channelC + key_thresholdUpper, 0);

    if(!settings.contains(group_channelC + key_upperHysteresis))
        settings.insert(group_channelC + key_upperHysteresis, 0);

    if(!settings.contains(group_channelC + key_thresholdLower))
        settings.insert(group_channelC + key_thresholdLower, 0);

    if(!settings.contains(group_channelC + key_lowerHysteresis))
        settings.insert(group_channelC + key_lowerHysteresis, 0);

    //Trigger settings channel D
    if(!settings.contains(group_channelD + key_triggerState))
        settings.insert(group_channelD + key_triggerState, PS6000_CONDITION_DONT_CARE);

    if(!settings.contains(group_channelD + key_triggerMode))
        settings.insert(group_channelD + key_triggerMode, PS6000_LEVEL);

    if(!settings.contains(group_channelD + key_triggerDirection))
        settings.insert(group_channelD + key_triggerDirection, PS6000_ABOVE);

    if(!settings.contains(group_channelD + key_thresholdUpper))
        settings.insert(group_channelD + key_thresholdUpper, 0);

    if(!settings.contains(group_channelD + key_upperHysteresis))
        settings.insert(group_channelD + key_upperHysteresis, 0);

    if(!settings.contains(group_channelD + key_thresholdLower))
        settings.insert(group_channelD + key_thresholdLower, 0);

    if(!settings.contains(group_channelD + key_lowerHysteresis))
        settings.insert(group_channelD + key_lowerHysteresis, 0);

    //Trigger settings channel AUX
    if(!settings.contains(group_channelAUX + key_triggerState))
        settings.insert(group_channelAUX + key_triggerState, PS6000_CONDITION_DONT_CARE);

    if(!settings.contains(group_channelAUX + key_triggerMode))
        settings.insert(group_channelAUX + key_triggerMode, PS6000_LEVEL);

    if(!settings.contains(group_channelAUX + key_triggerDirection))
        settings.insert(group_channelAUX + key_triggerDirection, PS6000_ABOVE);

    if(!settings.contains(group_channelAUX + key_thresholdUpper))
        settings.insert(group_channelAUX + key_thresholdUpper, 0);

    if(!settings.contains(group_channelAUX + key_upperHysteresis))
        settings.insert(group_channelAUX + key_upperHysteresis, 0);

    if(!settings.contains(group_channelAUX + key_thresholdLower))
        settings.insert(group_channelAUX + key_thresholdLower, 0);

    if(!settings.contains(group_channelAUX + key_lowerHysteresis))
        settings.insert(group_channelAUX + key_lowerHysteresis, 0);


    //Autotrigger
    if(!settings.contains(key_autoTriggerMs))
        settings.insert(key_autoTriggerMs, 0);
}


void PicoScopeSettings::setLinkSettings(bool linkSettings)
{
    settings.insert(key_linkSettings, linkSettings);
}


bool PicoScopeSettings::linkSettings()
{
    return settings.value(key_linkSettings).toBool();
}


void PicoScopeSettings::setChannel(PSChannel channel, bool enabled, PSSignalSource source)
{
    switch(channel)
    {
        case PSChannel::A:  settings.insert(key_channelAenabled, enabled);        break;
        case PSChannel::B:  settings.insert(key_channelBenabled, enabled);        break;
        case PSChannel::C:  settings.insert(key_channelCenabled, enabled);        break;
        case PSChannel::D:  settings.insert(key_channelDenabled, enabled);        break;
    }
    channelSettingsChanged = true;
    signalHelper(source);
}


bool PicoScopeSettings::channel(PSChannel channel)
{
    switch(channel)
    {
        case PSChannel::A:  return settings.value(key_channelAenabled).toBool();    break;
        case PSChannel::B:  return settings.value(key_channelBenabled).toBool();    break;
        case PSChannel::C:  return settings.value(key_channelCenabled).toBool();    break;
        case PSChannel::D:  return settings.value(key_channelDenabled).toBool();    break;
        default:            return false;        break;
    }
}


void PicoScopeSettings::setRange(PSChannel channel, PSRange range, PSSignalSource source)
{
    QString key = QString("");
    switch(channel)
    {
        case PSChannel::A: key.append(group_channelA); break;
        case PSChannel::B: key.append(group_channelB); break;
        case PSChannel::C: key.append(group_channelC); break;
        case PSChannel::D: key.append(group_channelD); break;
    }
    key.append(key_range);

    switch(range)
    {
        case PSRange::R50mV:  settings.insert(key, (unsigned int)0); break;
        case PSRange::R100mV: settings.insert(key, (unsigned int)1); break;
        case PSRange::R200mV: settings.insert(key, (unsigned int)2); break;
        case PSRange::R500mV: settings.insert(key, (unsigned int)3); break;
        case PSRange::R1V:    settings.insert(key, (unsigned int)4); break;
        case PSRange::R2V:    settings.insert(key, (unsigned int)5); break;
        case PSRange::R5V:    settings.insert(key, (unsigned int)6); break;
        case PSRange::R10V:   settings.insert(key, (unsigned int)7); break;
        case PSRange::R20V:   settings.insert(key, (unsigned int)8); break;
    }
    emit rangeChanged(channel);
    signalHelper(source);
}


PSRange PicoScopeSettings::range(PSChannel channel)
{
    QString key = QString("");
    switch(channel)
    {
        case PSChannel::A: key.append(group_channelA); break;
        case PSChannel::B: key.append(group_channelB); break;
        case PSChannel::C: key.append(group_channelC); break;
        case PSChannel::D: key.append(group_channelD); break;
    }
    key.append(key_range);
    bool ok;
    int range = settings.value(key).toInt(&ok);
    if(ok)
    {
        switch(range)
        {
            case 0:     return PSRange::R50mV;      break;
            case 1:     return PSRange::R100mV;     break;
            case 2:     return PSRange::R200mV;     break;
            case 3:     return PSRange::R500mV;     break;
            case 4:     return PSRange::R1V;        break;
            case 5:     return PSRange::R2V;        break;
            case 6:     return PSRange::R5V;        break;
            case 7:     return PSRange::R10V;       break;
            case 8:     return PSRange::R20V;       break;
        }
    }
    else
    {
        qDebug() << "Error parsing range";
    }
}


void PicoScopeSettings::setCoupling(PSCoupling coupling, PSSignalSource source)
{
    switch(coupling)
    {
        case PSCoupling::AC:    settings.insert(key_coupling, (unsigned int)0); break;
        case PSCoupling::DC1M:  settings.insert(key_coupling, (unsigned int)1); break;
        case PSCoupling::DC50R: settings.insert(key_coupling, (unsigned int)2); break;
    }
    channelSettingsChanged = true;
    signalHelper(source);
}


PSCoupling PicoScopeSettings::coupling()
{
    bool ok;
    int coupling = settings.value(key_coupling).toInt(&ok);
    if(ok)
    {
        switch(coupling)
        {
            case 0: return PSCoupling::AC;      break;
            case 1: return PSCoupling::DC1M;    break;
            case 2: return PSCoupling::DC50R;   break;
        }
    }
    else
    {
        qDebug() << "Error parsing coupling";
    }
}


void PicoScopeSettings::setTimebase(unsigned long timebase, PSSignalSource source)
{
    settings.insert(key_timebase, (qulonglong)timebase);
    qDebug() << "psettings tb =" << timebase;
    signalHelper(source);
}


unsigned long PicoScopeSettings::timebase()
{
    return settings.value(key_timebase, 4).toULongLong();
}


void PicoScopeSettings::setTimeInterval(float timeInterval, PSSignalSource source)
{
    v_timeInterval = timeInterval;
    signalHelper(source);
}


float PicoScopeSettings::timeInterval()
{
    return v_timeInterval;
}

void PicoScopeSettings::setSampleInterval(double sampleInterval, PSSignalSource source)
{
    v_sampleInterval = sampleInterval;
    signalHelper(source);
}


double PicoScopeSettings::sampleInterval()
{
    return v_sampleInterval;
}

void PicoScopeSettings::setOffset(PSChannel channel, float offset, PSSignalSource source)
{
    switch(channel)
    {
    case PSChannel::A: settings.insert(group_channelA + key_offset, offset); break;
    case PSChannel::B: settings.insert(group_channelB + key_offset, offset); break;
    case PSChannel::C: settings.insert(group_channelC + key_offset, offset); break;
    case PSChannel::D: settings.insert(group_channelD + key_offset, offset); break;
    }
    signalHelper(source);
}


float PicoScopeSettings::offset(PSChannel channel)
{
    switch(channel)
    {
    case PSChannel::A: return settings.value(group_channelA + key_offset).toFloat(); break;
    case PSChannel::B: return settings.value(group_channelB + key_offset).toFloat(); break;
    case PSChannel::C: return settings.value(group_channelC + key_offset).toFloat(); break;
    case PSChannel::D: return settings.value(group_channelD + key_offset).toFloat(); break;
    }
}


void PicoScopeSettings::setCollectionTime(double ct, PSSignalSource source)
{
    settings.insert(key_collectionTime, ct);
    signalHelper(source);
}


double PicoScopeSettings::collectionTime()
{
    return settings.value(key_collectionTime, 1.0).toDouble();
}


void PicoScopeSettings::setCaptures(unsigned int captures, PSSignalSource source)
{
    settings.insert(key_captures, captures);
    signalHelper(source);
}


unsigned int PicoScopeSettings::captures()
{
    return settings.value(key_captures, 1).toUInt();
}


void PicoScopeSettings::setSamples(unsigned long samples)
{
    v_samples = samples;
}


unsigned long PicoScopeSettings::samples()
{
    return v_samples;
}


void PicoScopeSettings::setPresamplePercentage(unsigned int percentage, PSSignalSource source)
{
    settings.insert(key_presamplepercentage, percentage);
    signalHelper(source);
}


unsigned int PicoScopeSettings::presamplePercentage()
{
    return settings.value(key_presamplepercentage, 0).toUInt();
}


QStringList PicoScopeSettings::availableRange()
{
    QStringList rangeList;
    rangeList << "± 50 mV" << "± 100 mV" << "± 200 mV" << "± 500 mV" << "± 1 V" << "± 2 V" << "± 5 V";

    if(!(coupling() == PSCoupling::DC50R))
        rangeList << "± 10 V" << "± 20 V";

    return rangeList;
}


QStringList PicoScopeSettings::availableCoupling()
{
    QStringList couplingList;
    couplingList << "AC" << "DC [1 MOhm]";

    PSRange range = PSRange::R50mV;

    for(int i = 1; i < 5; i++)
    {
        PSChannel channel;
        switch(i)
        {
        case 1: channel = PSChannel::A; break;
        case 2: channel = PSChannel::B; break;
        case 3: channel = PSChannel::C; break;
        case 4: channel = PSChannel::D; break;
        }
        if(this->channel(channel))
            if(this->range(channel) > range)
                range = this->range(channel);
    }

    if(!(range == PSRange::R10V) && !(range == PSRange::R20V))
        couplingList << "DC [50 Ohm]";

    return couplingList;
}


PSRange PicoScopeSettings::intToPSRange(int range)
{
    switch(range)
    {
        case 0:     return PSRange::R50mV;  break;
        case 1:     return PSRange::R100mV; break;
        case 2:     return PSRange::R200mV; break;
        case 3:     return PSRange::R500mV; break;
        case 4:     return PSRange::R1V;    break;
        case 5:     return PSRange::R2V;    break;
        case 6:     return PSRange::R5V;    break;
        case 7:     return PSRange::R10V;   break;
        case 8:     return PSRange::R20V;   break;
        default:    return PSRange::R2V;    break;
    }
}


PSCoupling PicoScopeSettings::intToPSCoupling(int coupling)
{
    switch(coupling)
    {
        case 0: return PSCoupling::AC;      break;
        case 1: return PSCoupling::DC1M;    break;
        case 2: return PSCoupling::DC50R;   break;
    }
}


enum enPS6000TriggerState PicoScopeSettings::getChannelTriggerState(PSChannel channel)
{
    switch(channel)
    {
        case PSChannel::A:      return channelATrigger;     break;
        case PSChannel::B:      return channelBTrigger;     break;
        case PSChannel::C:      return channelCTrigger;     break;
        case PSChannel::D:      return channelDTrigger;     break;
        case PSChannel::AUX:    return channelAuxTrigger;   break;
    }
}


void PicoScopeSettings::signalHelper(PSSignalSource source)
{
    switch(source)
    {
        case PSSignalSource::UI:        emit settingsChangedUI();       break;
        case PSSignalSource::Driver:    emit settingsChangedDriver();   break;
        case PSSignalSource::DontCare:  break;
    }
}

void PicoScopeSettings::setTriggerState(PSChannel channel, enPS6000TriggerState state)
{
    switch(channel)
    {
    case PSChannel::A:  settings.insert(group_channelA + key_triggerState, state);  break;
    case PSChannel::B:  settings.insert(group_channelB + key_triggerState, state);  break;
    case PSChannel::C:  settings.insert(group_channelC + key_triggerState, state);  break;
    case PSChannel::D:  settings.insert(group_channelD + key_triggerState, state);  break;
    case PSChannel::AUX:  settings.insert(group_channelAUX + key_triggerState, state);  break;
    }
    emit settingsChanged();
}


enPS6000TriggerState PicoScopeSettings::triggerState(PSChannel channel)
{
    switch(channel)
    {
    case PSChannel::A: switch(settings.value(group_channelA + key_triggerState, "0").toInt())
        {
        case 0: return PS6000_CONDITION_DONT_CARE; break;
        case 1: return PS6000_CONDITION_TRUE; break;
        case 2: return PS6000_CONDITION_FALSE; break;
        }
        break;
    case PSChannel::B: switch(settings.value(group_channelB + key_triggerState, "0").toInt())
        {
        case 0: return PS6000_CONDITION_DONT_CARE; break;
        case 1: return PS6000_CONDITION_TRUE; break;
        case 2: return PS6000_CONDITION_FALSE; break;
        }
        break;
    case PSChannel::C: switch(settings.value(group_channelC + key_triggerState, "0").toInt())
        {
        case 0: return PS6000_CONDITION_DONT_CARE; break;
        case 1: return PS6000_CONDITION_TRUE; break;
        case 2: return PS6000_CONDITION_FALSE; break;
        }
        break;
    case PSChannel::D: switch(settings.value(group_channelD + key_triggerState, "0").toInt())
        {
        case 0: return PS6000_CONDITION_DONT_CARE; break;
        case 1: return PS6000_CONDITION_TRUE; break;
        case 2: return PS6000_CONDITION_FALSE; break;
        }
        break;
    case PSChannel::AUX: switch(settings.value(group_channelAUX + key_triggerState, "0").toInt())
        {
        case 0: return PS6000_CONDITION_DONT_CARE; break;
        case 1: return PS6000_CONDITION_TRUE; break;
        case 2: return PS6000_CONDITION_FALSE; break;
        }
        break;
    }
}


void PicoScopeSettings::setTriggerMode(PSChannel channel, enPS6000ThresholdMode mode)
{
    switch(channel)
    {
    case PSChannel::A:  settings.insert(group_channelA + key_triggerMode, mode);  break;
    case PSChannel::B:  settings.insert(group_channelB + key_triggerMode, mode);  break;
    case PSChannel::C:  settings.insert(group_channelC + key_triggerMode, mode);  break;
    case PSChannel::D:  settings.insert(group_channelD + key_triggerMode, mode);  break;
    case PSChannel::AUX:  settings.insert(group_channelAUX + key_triggerMode, mode);  break;
    }
    emit settingsChanged();
}


enPS6000ThresholdMode PicoScopeSettings::triggerMode(PSChannel channel)
{
    switch(channel)
    {
    case PSChannel::A: switch(settings.value(group_channelA + key_triggerMode, "0").toInt())
        {
        case 0: return PS6000_LEVEL; break;
        case 1: return PS6000_WINDOW; break;
        }
        break;
    case PSChannel::B: switch(settings.value(group_channelB + key_triggerMode, "0").toInt())
        {
        case 0: return PS6000_LEVEL; break;
        case 1: return PS6000_WINDOW; break;
        }
        break;
    case PSChannel::C: switch(settings.value(group_channelC + key_triggerMode, "0").toInt())
        {
        case 0: return PS6000_LEVEL; break;
        case 1: return PS6000_WINDOW; break;
        }
        break;
    case PSChannel::D: switch(settings.value(group_channelD + key_triggerMode, "0").toInt())
        {
        case 0: return PS6000_LEVEL; break;
        case 1: return PS6000_WINDOW; break;
        }
        break;
    case PSChannel::AUX: switch(settings.value(group_channelAUX + key_triggerMode, "0").toInt())
        {
        case 0: return PS6000_LEVEL; break;
        case 1: return PS6000_WINDOW; break;
        }
        break;
    }
}

void PicoScopeSettings::setTriggerDirection(PSChannel channel, enPS6000ThresholdDirection direction)
{
    switch(channel)
    {
    case PSChannel::A:  settings.insert(group_channelA + key_triggerDirection, direction);  break;
    case PSChannel::B:  settings.insert(group_channelB + key_triggerDirection, direction);  break;
    case PSChannel::C:  settings.insert(group_channelC + key_triggerDirection, direction);  break;
    case PSChannel::D:  settings.insert(group_channelD + key_triggerDirection, direction);  break;
    case PSChannel::AUX:  settings.insert(group_channelAUX + key_triggerDirection, direction);  break;
    }
    emit settingsChanged();
}


enPS6000ThresholdDirection PicoScopeSettings::triggerDirection(PSChannel channel)
{
    switch(channel)
    {
        case PSChannel::A:
        {
            if(triggerMode(PSChannel::A) == PS6000_LEVEL)
            {
                switch(settings.value(group_channelA + key_triggerDirection, "0").toInt())
                {
                case 0: return PS6000_ABOVE; break;
                case 1: return PS6000_BELOW; break;
                case 2: return PS6000_RISING; break;
                case 3: return PS6000_FALLING; break;
                case 4: return PS6000_RISING_OR_FALLING; break;
                case 5: return PS6000_ABOVE_LOWER; break;
                case 6: return PS6000_BELOW_LOWER; break;
                case 7: return PS6000_RISING_LOWER; break;
                case 8: return PS6000_FALLING_LOWER; break;
                }
            }
            else
            {
                switch(settings.value(group_channelA + key_triggerDirection, "0").toInt())
                {
                case 0: return PS6000_INSIDE; break;
                case 1: return PS6000_OUTSIDE; break;
                case 2: return PS6000_ENTER; break;
                case 3: return PS6000_EXIT; break;
                case 4: return PS6000_ENTER_OR_EXIT; break;
                case 9: return PS6000_POSITIVE_RUNT; break;
                case 10: return PS6000_NEGATIVE_RUNT; break;
                }
            }
        }

        case PSChannel::B:
        {
            if(triggerMode(PSChannel::B) == PS6000_LEVEL)
            {
                switch(settings.value(group_channelB + key_triggerDirection, "0").toInt())
                {
                case 0: return PS6000_ABOVE; break;
                case 1: return PS6000_BELOW; break;
                case 2: return PS6000_RISING; break;
                case 3: return PS6000_FALLING; break;
                case 4: return PS6000_RISING_OR_FALLING; break;
                case 5: return PS6000_ABOVE_LOWER; break;
                case 6: return PS6000_BELOW_LOWER; break;
                case 7: return PS6000_RISING_LOWER; break;
                case 8: return PS6000_FALLING_LOWER; break;
                }
            }
            else
            {
                switch(settings.value(group_channelB + key_triggerDirection, "0").toInt())
                {
                case 0: return PS6000_INSIDE; break;
                case 1: return PS6000_OUTSIDE; break;
                case 2: return PS6000_ENTER; break;
                case 3: return PS6000_EXIT; break;
                case 4: return PS6000_ENTER_OR_EXIT; break;
                case 9: return PS6000_POSITIVE_RUNT; break;
                case 10: return PS6000_NEGATIVE_RUNT; break;
                }
            }
        }

        case PSChannel::C:
        {
            if(triggerMode(PSChannel::C) == PS6000_LEVEL)
            {
                switch(settings.value(group_channelC + key_triggerDirection, "0").toInt())
                {
                case 0: return PS6000_ABOVE; break;
                case 1: return PS6000_BELOW; break;
                case 2: return PS6000_RISING; break;
                case 3: return PS6000_FALLING; break;
                case 4: return PS6000_RISING_OR_FALLING; break;
                case 5: return PS6000_ABOVE_LOWER; break;
                case 6: return PS6000_BELOW_LOWER; break;
                case 7: return PS6000_RISING_LOWER; break;
                case 8: return PS6000_FALLING_LOWER; break;
                }
            }
            else
            {
                switch(settings.value(group_channelC + key_triggerDirection, "0").toInt())
                {
                case 0: return PS6000_INSIDE; break;
                case 1: return PS6000_OUTSIDE; break;
                case 2: return PS6000_ENTER; break;
                case 3: return PS6000_EXIT; break;
                case 4: return PS6000_ENTER_OR_EXIT; break;
                case 9: return PS6000_POSITIVE_RUNT; break;
                case 10: return PS6000_NEGATIVE_RUNT; break;
                }
            }
        }

        case PSChannel::D:
        {
            if(triggerMode(PSChannel::D) == PS6000_LEVEL)
            {
                switch(settings.value(group_channelD + key_triggerDirection, "0").toInt())
                {
                case 0: return PS6000_ABOVE; break;
                case 1: return PS6000_BELOW; break;
                case 2: return PS6000_RISING; break;
                case 3: return PS6000_FALLING; break;
                case 4: return PS6000_RISING_OR_FALLING; break;
                case 5: return PS6000_ABOVE_LOWER; break;
                case 6: return PS6000_BELOW_LOWER; break;
                case 7: return PS6000_RISING_LOWER; break;
                case 8: return PS6000_FALLING_LOWER; break;
                }
            }
            else
            {
                switch(settings.value(group_channelD + key_triggerDirection, "0").toInt())
                {
                case 0: return PS6000_INSIDE; break;
                case 1: return PS6000_OUTSIDE; break;
                case 2: return PS6000_ENTER; break;
                case 3: return PS6000_EXIT; break;
                case 4: return PS6000_ENTER_OR_EXIT; break;
                case 9: return PS6000_POSITIVE_RUNT; break;
                case 10: return PS6000_NEGATIVE_RUNT; break;
                }
            }
        }

        case PSChannel::AUX:
        {
            if(triggerMode(PSChannel::AUX) == PS6000_LEVEL)
            {
                switch(settings.value(group_channelAUX + key_triggerDirection, "0").toInt())
                {
                case 0: return PS6000_ABOVE; break;
                case 1: return PS6000_BELOW; break;
                case 2: return PS6000_RISING; break;
                case 3: return PS6000_FALLING; break;
                case 4: return PS6000_RISING_OR_FALLING; break;
                case 5: return PS6000_ABOVE_LOWER; break;
                case 6: return PS6000_BELOW_LOWER; break;
                case 7: return PS6000_RISING_LOWER; break;
                case 8: return PS6000_FALLING_LOWER; break;
                }
            }
            else
            {
                switch(settings.value(group_channelAUX + key_triggerDirection, "0").toInt())
                {
                case 0: return PS6000_INSIDE; break;
                case 1: return PS6000_OUTSIDE; break;
                case 2: return PS6000_ENTER; break;
                case 3: return PS6000_EXIT; break;
                case 4: return PS6000_ENTER_OR_EXIT; break;
                case 9: return PS6000_POSITIVE_RUNT; break;
                case 10: return PS6000_NEGATIVE_RUNT; break;
                }
            }
        }
    }
}


void PicoScopeSettings::setUpperThreshold(PSChannel channel, double threshold_volt, double hysteresis_volt)
{
    switch(channel)
    {
        case PSChannel::A:
        {
            settings.insert(group_channelA + key_thresholdUpper, threshold_volt);
            settings.insert(group_channelA + key_upperHysteresis, hysteresis_volt);
        }
        case PSChannel::B:
        {
            settings.insert(group_channelB + key_thresholdUpper, threshold_volt);
            settings.insert(group_channelB + key_upperHysteresis, hysteresis_volt);
        }
        case PSChannel::C:
        {
            settings.insert(group_channelC + key_thresholdUpper, threshold_volt);
            settings.insert(group_channelC + key_upperHysteresis, hysteresis_volt);
        }
        case PSChannel::D:
        {
            settings.insert(group_channelD + key_thresholdUpper, threshold_volt);
            settings.insert(group_channelD + key_upperHysteresis, hysteresis_volt);
        }
        case PSChannel::AUX:
        {
            settings.insert(group_channelAUX + key_thresholdUpper, threshold_volt);
            settings.insert(group_channelAUX + key_upperHysteresis, hysteresis_volt);
        }
    }
    emit settingsChanged();
}


double PicoScopeSettings::upperThreshold(PSChannel channel)
{
    switch(channel)
    {
    case PSChannel::A:
        return settings.value(group_channelA + key_thresholdUpper, "0").toDouble();
        break;

    case PSChannel::B:
        return settings.value(group_channelB + key_thresholdUpper, "0").toDouble();
        break;

    case PSChannel::C:
        return settings.value(group_channelC + key_thresholdUpper, "0").toDouble();
        break;

    case PSChannel::D:
        return settings.value(group_channelD + key_thresholdUpper, "0").toDouble();
        break;

    case PSChannel::AUX:
        return settings.value(group_channelAUX + key_thresholdUpper, "0").toDouble();
        break;
    }
}


double PicoScopeSettings::upperHysteresis(PSChannel channel)
{
    switch(channel)
    {
    case PSChannel::A:
        return settings.value(group_channelA + key_upperHysteresis, "0").toDouble();
        break;

    case PSChannel::B:
        return settings.value(group_channelB + key_upperHysteresis, "0").toDouble();
        break;

    case PSChannel::C:
        return settings.value(group_channelC + key_upperHysteresis, "0").toDouble();
        break;

    case PSChannel::D:
        return settings.value(group_channelD + key_upperHysteresis, "0").toDouble();
        break;

    case PSChannel::AUX:
        return settings.value(group_channelAUX + key_upperHysteresis, "0").toDouble();
        break;
    }
}


short PicoScopeSettings::upperThresholdADC(PSChannel channel)
{
    switch(channel)
    {
    case PSChannel::A:
        return (short)(settings.value(group_channelA + key_thresholdUpper, "0").toDouble() * getRangeFactor(range(PSChannel::A)));
        break;

    case PSChannel::B:
        return (short)(settings.value(group_channelB + key_thresholdUpper, "0").toDouble() * getRangeFactor(range(PSChannel::B)));
        break;

    case PSChannel::C:
        return (short)(settings.value(group_channelC + key_thresholdUpper, "0").toDouble() * getRangeFactor(range(PSChannel::C)));
        break;

    case PSChannel::D:
        return (short)(settings.value(group_channelD + key_thresholdUpper, "0").toDouble() * getRangeFactor(range(PSChannel::D)));
        break;

    case PSChannel::AUX:
        return (short)(settings.value(group_channelAUX + key_thresholdUpper, "0").toDouble() * 32512.0);
        break;

    default:
        throw LIISimException("PicoScopeSettings::upperThresholdADC: wrong channel");
        break;
    }
}


short PicoScopeSettings::upperHysteresisADC(PSChannel channel)
{
    switch(channel)
    {
    case PSChannel::A:
        return (short)(settings.value(group_channelA + key_upperHysteresis, "0").toDouble() * getRangeFactor(range(PSChannel::A)));
        break;

    case PSChannel::B:
        return (short)(settings.value(group_channelB + key_upperHysteresis, "0").toDouble() * getRangeFactor(range(PSChannel::B)));
        break;

    case PSChannel::C:
        return (short)(settings.value(group_channelC + key_upperHysteresis, "0").toDouble() * getRangeFactor(range(PSChannel::C)));
        break;

    case PSChannel::D:
        return (short)(settings.value(group_channelD + key_upperHysteresis, "0").toDouble() * getRangeFactor(range(PSChannel::D)));
        break;

    case PSChannel::AUX:
        return (short)(settings.value(group_channelAUX + key_upperHysteresis, "0").toDouble() * 32512.0);
        break;

    default:
        throw LIISimException("PicoScopeSettings::upperThresholdADC: wrong channel");
        break;
    }
}


void PicoScopeSettings::setLowerThreshold(PSChannel channel, double threshold_volt, double hysteresis_volt)
{
    switch(channel)
    {
        case PSChannel::A:
        {
            settings.insert(group_channelA + key_thresholdLower, threshold_volt);
            settings.insert(group_channelA + key_lowerHysteresis, hysteresis_volt);
        }
        case PSChannel::B:
        {
            settings.insert(group_channelB + key_thresholdLower, threshold_volt);
            settings.insert(group_channelB + key_lowerHysteresis, hysteresis_volt);
        }
        case PSChannel::C:
        {
            settings.insert(group_channelC + key_thresholdLower, threshold_volt);
            settings.insert(group_channelC + key_lowerHysteresis, hysteresis_volt);
        }
        case PSChannel::D:
        {
            settings.insert(group_channelD + key_thresholdLower, threshold_volt);
            settings.insert(group_channelD + key_lowerHysteresis, hysteresis_volt);
        }
        case PSChannel::AUX:
        {
            settings.insert(group_channelAUX + key_thresholdLower, threshold_volt);
            settings.insert(group_channelAUX + key_lowerHysteresis, hysteresis_volt);
        }
    }
    emit settingsChanged();
}

double PicoScopeSettings::lowerThreshold(PSChannel channel)
{
    switch(channel)
    {
    case PSChannel::A:
        return settings.value(group_channelA + key_thresholdLower, "0").toDouble();
        break;

    case PSChannel::B:
        return settings.value(group_channelB + key_thresholdLower, "0").toDouble();
        break;

    case PSChannel::C:
        return settings.value(group_channelC + key_thresholdLower, "0").toDouble();
        break;

    case PSChannel::D:
        return settings.value(group_channelD + key_thresholdLower, "0").toDouble();
        break;

    case PSChannel::AUX:
        return settings.value(group_channelAUX + key_thresholdLower, "0").toDouble();
        break;
    }
}

double PicoScopeSettings::lowerHysteresis(PSChannel channel)
{
    switch(channel)
    {
    case PSChannel::A:
        return settings.value(group_channelA + key_lowerHysteresis, "0").toDouble();
        break;

    case PSChannel::B:
        return settings.value(group_channelB + key_lowerHysteresis, "0").toDouble();
        break;

    case PSChannel::C:
        return settings.value(group_channelC + key_lowerHysteresis, "0").toDouble();
        break;

    case PSChannel::D:
        return settings.value(group_channelD + key_lowerHysteresis, "0").toDouble();
        break;

    case PSChannel::AUX:
        return settings.value(group_channelAUX + key_lowerHysteresis, "0").toDouble();
        break;
    }
}


short PicoScopeSettings::lowerThresholdADC(PSChannel channel)
{
    switch(channel)
    {
    case PSChannel::A:
        return (short)(settings.value(group_channelA + key_thresholdLower, "0").toDouble() * getRangeFactor(range(PSChannel::A)));
        break;

    case PSChannel::B:
        return (short)(settings.value(group_channelB + key_thresholdLower, "0").toDouble() * getRangeFactor(range(PSChannel::B)));
        break;

    case PSChannel::C:
        return (short)(settings.value(group_channelC + key_thresholdLower, "0").toDouble() * getRangeFactor(range(PSChannel::C)));
        break;

    case PSChannel::D:
        return (short)(settings.value(group_channelD + key_thresholdLower, "0").toDouble() * getRangeFactor(range(PSChannel::D)));
        break;

    case PSChannel::AUX:
        return (short)(settings.value(group_channelAUX + key_thresholdLower, "0").toDouble() * 32512.0);
        break;
    }
}


short PicoScopeSettings::lowerHysteresisADC(PSChannel channel)
{
    switch(channel)
    {
    case PSChannel::A:
        return (short)(settings.value(group_channelA + key_lowerHysteresis, "0").toDouble() * getRangeFactor(range(PSChannel::A)));
        break;

    case PSChannel::B:
        return (short)(settings.value(group_channelB + key_lowerHysteresis, "0").toDouble() * getRangeFactor(range(PSChannel::B)));
        break;

    case PSChannel::C:
        return (short)(settings.value(group_channelC + key_lowerHysteresis, "0").toDouble() * getRangeFactor(range(PSChannel::C)));
        break;

    case PSChannel::D:
        return (short)(settings.value(group_channelD + key_lowerHysteresis, "0").toDouble() * getRangeFactor(range(PSChannel::D)));
        break;

    case PSChannel::AUX:
        return (short)(settings.value(group_channelAUX + key_lowerHysteresis, "0").toDouble() * 32512.0);
        break;
    }
}


QString PicoScopeSettings::getTriggerDescription(PSChannel channel)
{
    QString ret("");

    switch(triggerState(channel))
    {
    case PS6000_CONDITION_DONT_CARE: ret.append("-"); return ret; break;
    case PS6000_CONDITION_TRUE: ret.append("True: "); break;
    case PS6000_CONDITION_FALSE: ret.append("False: "); break;
    }

    switch(triggerMode(channel))
    {
    case PS6000_LEVEL: ret.append("Level | ");
        switch(triggerDirection(channel))
        {
        case PS6000_ABOVE: ret.append("Above (upper) | "); break;
        case PS6000_BELOW: ret.append("Below (upper) | "); break;
        case PS6000_RISING: ret.append("Rising edge (upper) | "); break;
        case PS6000_FALLING: ret.append("Falling edge (upper) | "); break;
        case PS6000_RISING_OR_FALLING: ret.append("Rising or falling edge | "); break;
        case PS6000_ABOVE_LOWER: ret.append("Above (lower) | "); break;
        case PS6000_BELOW_LOWER: ret.append("Below (lower) | "); break;
        case PS6000_RISING_LOWER: ret.append("Rising edge (lower) | "); break;
        case PS6000_FALLING_LOWER: ret.append("Falling edge (lower) | "); break;
        }
        break;
    case PS6000_WINDOW: ret.append("Window | ");
        switch(triggerDirection(channel))
        {
        case PS6000_INSIDE: ret.append("Inside | "); break;
        case PS6000_OUTSIDE: ret.append("Outside | "); break;
        case PS6000_ENTER: ret.append("Enter | "); break;
        case PS6000_EXIT: ret.append("Exit | "); break;
        case PS6000_ENTER_OR_EXIT: ret.append("Enter or exit | "); break;
        case PS6000_POSITIVE_RUNT: ret.append("Positive runt | "); break;
        case PS6000_NEGATIVE_RUNT: ret.append("Negative runt | "); break;
        }
    }

    switch(triggerMode(channel))
    {
    case PS6000_LEVEL:
        switch(triggerDirection(channel))
        {
        case PS6000_ABOVE:
        case PS6000_BELOW:
        case PS6000_RISING:
        case PS6000_FALLING: ret.append(QString::number(upperThreshold(channel))).append("V (");
                             ret.append(QString::number(upperHysteresis(channel))).append("V)"); break;

        case PS6000_ABOVE_LOWER:
        case PS6000_BELOW_LOWER:
        case PS6000_RISING_LOWER:
        case PS6000_FALLING_LOWER: ret.append(QString::number(lowerThreshold(channel))).append("V (");
                                   ret.append(QString::number(lowerHysteresis(channel))).append("V)"); break;

        case PS6000_RISING_OR_FALLING: ret.append(QString::number(upperThreshold(channel))).append("V (");
                                       ret.append(QString::number(upperHysteresis(channel))).append("V) | ");
                                       ret.append(QString::number(lowerThreshold(channel))).append("V (");
                                       ret.append(QString::number(lowerHysteresis(channel))).append("V)"); break;
        }
        break;
    case PS6000_WINDOW: ret.append(QString::number(upperThreshold(channel))).append("V (");
                        ret.append(QString::number(upperHysteresis(channel))).append("V) | ");
                        ret.append(QString::number(lowerThreshold(channel))).append("V (");
                        ret.append(QString::number(lowerHysteresis(channel))).append("V)"); break;
    }

    return ret;
}


bool PicoScopeSettings::getTriggerActiv(PSChannel channel)
{
    switch(triggerState(channel))
    {
    case PS6000_CONDITION_DONT_CARE:    return false;   break;
    case PS6000_CONDITION_TRUE:         return true;    break;
    case PS6000_CONDITION_FALSE:        return true;    break;
    }
}


QString PicoScopeSettings::getActiveTriggerChannels()
{
    QString ret;

    if(channelATrigger == PS6000_CONDITION_TRUE)
        ret.append(" A ");
    if(channelBTrigger == PS6000_CONDITION_TRUE)
        ret.append(" B ");
    if(channelCTrigger == PS6000_CONDITION_TRUE)
        ret.append(" C ");
    if(channelDTrigger == PS6000_CONDITION_TRUE)
        ret.append(" D ");
    if(channelAuxTrigger == PS6000_CONDITION_TRUE)
    {
        ret.append(" AUX ");
    }
    return ret;
}


double PicoScopeSettings::getRangeFactor(PSRange range)
{
    switch(range)
    {
        case PSRange::R50mV:    return 650240.0;     break;
        case PSRange::R100mV:   return 325120.0;     break;
        case PSRange::R200mV:   return 162560.0;     break;
        case PSRange::R500mV:   return 65024.0;      break;
        case PSRange::R1V:      return 32512.0;      break;
        case PSRange::R2V:      return 16256.0;      break;
        case PSRange::R5V:      return 6502.4;       break;
        case PSRange::R10V:     return 3251.2;       break;
        case PSRange::R20V:     return 1625.6;       break;
    }
}


void PicoScopeSettings::setAveragingBufferSize(unsigned int size, PSSignalSource source)
{
    settings.insert(key_averagingBufferSize, size);
}


unsigned int PicoScopeSettings::getAveragingBufferSize()
{
    return settings.value(key_averagingBufferSize, 1).toUInt();
}
