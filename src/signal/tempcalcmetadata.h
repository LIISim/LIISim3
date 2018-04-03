#ifndef TEMPCALCMETADATA
#define TEMPCALCMETADATA

#include "signal.h"

/**
 * @brief The TempCalcMetadata class
 * @details Contains metadata about with which settings the temperature
 * signal was calculated.
 */

class TempCalcMetadata
{
public:
    //Info
    int tempChannelID;
    QString method;
    Signal::SType signalSource;

    QString material; // spectroscopic
    QString sourceEm;

    //Two-Color/Ratio A-B
    int channelID1;
    int channelID2;

    //Spectrum
    int iterations;
    double startTemperature;
    double startC;
    bool autoStartC;
    QList<bool> activeChannels;
    bool bandpass;
    bool weighting;
};

#endif // TEMPCALCMETADATA

