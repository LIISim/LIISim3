#ifndef PICOSCOPESETTINGS_H
#define PICOSCOPESETTINGS_H

#include "settingsbase.h"
#include "general/picoscopecommon.h"

#include "../externalLibraries/picoscope6000/include/ps6000Api.h"

enum class PSChannel { A,
                       B,
                       C,
                       D,
                       AUX
                     };


enum class PSSignalSource { DontCare,
                            Driver,
                            UI
                          };

enum class PSStreamingMode { SingleMeasurement,
                             AverageMeasurement,
                             Both
                           };

class PicoScopeSettings : public SettingsBase
{
    Q_OBJECT

public:
    explicit PicoScopeSettings(QObject *parent = 0);
    ~PicoScopeSettings();

    void setLinkSettings(bool linkSettings);
    bool linkSettings();

    void setChannel(PSChannel channel, bool enabled, PSSignalSource source = PSSignalSource::DontCare);
    bool channel(PSChannel channel);

    void setRange(PSChannel channel, PSRange range, PSSignalSource source = PSSignalSource::DontCare);
    PSRange range(PSChannel channel);

    void setCoupling(PSCoupling coupling, PSSignalSource source = PSSignalSource::DontCare);
    PSCoupling coupling();

    void setTimebase(unsigned long timebase, PSSignalSource source = PSSignalSource::DontCare);
    unsigned long timebase();

    void setTimeInterval(float timeInterval, PSSignalSource source = PSSignalSource::DontCare);
    float timeInterval();

    void setSampleInterval(double sampleInterval, PSSignalSource source = PSSignalSource::DontCare);
    double sampleInterval();

    void setOffset(PSChannel channel, float offset, PSSignalSource source = PSSignalSource::DontCare);
    float offset(PSChannel channel);

    void setCollectionTime(double ct, PSSignalSource source = PSSignalSource::DontCare);
    double collectionTime();

    void setCaptures(unsigned int captures, PSSignalSource source = PSSignalSource::DontCare);
    unsigned int captures();

    void setSamples(unsigned long samples);
    unsigned long samples();

    void setPresamplePercentage(unsigned int percentage, PSSignalSource source = PSSignalSource::DontCare);
    unsigned int presamplePercentage();

    QStringList availableRange();
    QStringList availableCoupling();

    PSRange intToPSRange(int range);
    PSCoupling intToPSCoupling(int coupling);

    enum enPS6000TriggerState getChannelTriggerState(PSChannel channel);
    QString getActiveTriggerChannels();

    void signalHelper(PSSignalSource source);

    PSStreamingMode streaming_mode;

    void setTriggerState(PSChannel channel, enPS6000TriggerState state);
    enPS6000TriggerState triggerState(PSChannel channel);

    void setTriggerMode(PSChannel channel, enPS6000ThresholdMode mode);
    enPS6000ThresholdMode triggerMode(PSChannel channel);

    void setTriggerDirection(PSChannel channel, enPS6000ThresholdDirection direction);
    enPS6000ThresholdDirection triggerDirection(PSChannel channel);

    void setUpperThreshold(PSChannel channel, double threshold_volt, double hysteresis_volt);
    double upperThreshold(PSChannel channel);
    double upperHysteresis(PSChannel channel);
    short upperThresholdADC(PSChannel channel);
    short upperHysteresisADC(PSChannel channel);

    void setLowerThreshold(PSChannel channel, double threshold_volt, double hysteresis_volt);
    double lowerThreshold(PSChannel channel);
    double lowerHysteresis(PSChannel channel);
    short lowerThresholdADC(PSChannel channel);
    short lowerHysteresisADC(PSChannel channel);

    QString getTriggerDescription(PSChannel channel);
    bool getTriggerActiv(PSChannel channel);

    double getRangeFactor(PSRange range);

    void setAveragingBufferSize(unsigned int size, PSSignalSource source = PSSignalSource::DontCare);
    unsigned int getAveragingBufferSize();

protected:

private:

    void init();

    unsigned long v_samples;
    float v_timeInterval;
    double v_sampleInterval;

    bool channelSettingsChanged;

    QString key_linkSettings;
    QString key_averagingBufferSize;

    QString key_channelAenabled;
    QString key_channelBenabled;
    QString key_channelCenabled;
    QString key_channelDenabled;
    QString key_range;
    QString key_coupling;
    QString key_timebase;
    QString key_offset;
    QString key_collectionTime;
    QString key_dt;
    QString key_captures;
    QString key_presamplepercentage;

    QString group_channelA;
    QString group_channelB;
    QString group_channelC;
    QString group_channelD;
    QString group_channelAUX;

    //Trigger settings keys
    QString key_triggerState;
    QString key_triggerMode;
    QString key_triggerDirection;

    QString key_thresholdUpper;
    QString key_upperHysteresis;
    QString key_thresholdLower;
    QString key_lowerHysteresis;

    QString key_autoTriggerMs;

    enum enPS6000TriggerState channelATrigger;
    enum enPS6000TriggerState channelBTrigger;
    enum enPS6000TriggerState channelCTrigger;
    enum enPS6000TriggerState channelDTrigger;
    enum enPS6000TriggerState channelAuxTrigger;

signals:
    void settingsChangedDriver();
    void settingsChangedUI();

    void rangeChanged(PSChannel channel);

public slots:

};

#endif // PICOSCOPESETTINGS_H
