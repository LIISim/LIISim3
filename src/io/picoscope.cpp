#include "picoscope.h"
#include "./core.h"
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QMutexLocker>
#include "../../calculations/standarddeviation.h"

#define MAX_STREAMING_UPDATE_RATE_MS    100
#define RESTART_STREAMING_WAIT_MS       250

PicoScope::PicoScope(PicoScopeSettings *settings, QObject *parent) : QObject(parent), streamingLock(1)
{
    bufferSize = 0;
    bufferCaptures = 0;

    this->settings = settings;
    connect(settings, SIGNAL(settingsChangedUI()), this, SLOT(updateSettings()));

    lastError = "";

    isOpen(false);
    isStreaming(false);
    isBlockrun(false);
    isProcessing(false);
    isTriggered(false);
    isError(false);

    streamingDataProcessed = true;
    restartStreaming = false;

    qRegisterMetaType<Signal>("Signal");
    qRegisterMetaType<StreamPoint>("StreamPoint");

    calculationThreads[0] = new AverageCalculator();
    calculationThreads[1] = new AverageCalculator();
    calculationThreads[2] = new AverageCalculator();
    calculationThreads[3] = new AverageCalculator();

    lastOverflowCounter[0] = 0;
    lastOverflowCounter[1] = 0;
    lastOverflowCounter[2] = 0;
    lastOverflowCounter[3] = 0;

    lastOverflow[0] = false;
    lastOverflow[1] = false;
    lastOverflow[2] = false;
    lastOverflow[3] = false;
}


PicoScope::~PicoScope()
{
    streaming = false;
    close();
}


bool PicoScope::open()
{
    if(driverOpen)
    {
        emit infoMsg("PicoScope already opened (close before reopen)");
        return driverOpen;
    }
    else
    {
        emit infoMsg("Opening...");
        PICO_STATUS status = ps6000OpenUnit(&handle, NULL);

        if(status == PICO_OK)
        {
            emit infoMsg(picoStatusAsString(status));
            isOpen(true);
            isError(false);
            return driverOpen;
        }
        else
        {
            setLastError(picoStatusAsString(status));
            isOpen(false);
            return driverOpen;
        }
    }
}


void PicoScope::close()
{
    ps6000CloseUnit(handle);
    emit infoMsg("PicoScope closed");
    isOpen(false);
    isStreaming(false);
    isBlockrun(false);
    isProcessing(false);
    isError(false);
}

bool PicoScope::internalOpen()
{
    if(driverOpen)
        return driverOpen;
    else
    {
        emit infoMsg("Opening PicoScope...");
        PICO_STATUS status = ps6000OpenUnit(&handle, NULL);
        if(status == PICO_OK)
        {
            emit infoMsg("PicoScope open");
            isError(false);
            isOpen(true);
            return isOpen();
        }
        else
        {
            setLastError(picoStatusAsString(status));
            isOpen(false);
            return isOpen();
        }
    }
}


void PicoScope::getRun(MRun *run, bool averageCaptures, bool calculateStdev)
{
    QFuture<void> future = QtConcurrent::run(this, &PicoScope::blockWorker, run, averageCaptures, calculateStdev);
}


void PicoScope::offsetBounds(PSChannel channel, float *max, float *min)
{
    //lock mutex to ensure access to picoscope
    if(!open())
    {
        *max = 0;
        *min = 0;
    }
    else
    {
        PS6000_COUPLING coupling = getCoupling(settings->coupling());

        PS6000_RANGE range = getRange(settings->range(channel));

        PICO_STATUS status = ps6000GetAnalogueOffset(handle, range, coupling, max, min);
        if(status != PICO_OK)
        {
            setLastError(picoStatusAsString(status));
            *max = 0;
            *min = 0;
        }
    }
}


Signal PicoScope::getAverageStreamSignal(PSChannel channel)
{
    switch(channel)
    {
        case PSChannel::A:  return averageStreamSignalA;    break;
        case PSChannel::B:  return averageStreamSignalB;    break;
        case PSChannel::C:  return averageStreamSignalC;    break;
        case PSChannel::D:  return averageStreamSignalD;    break;
        default:
            throw LIISimException("PicoScope::getAverageStreamSignal(): invalid channel" );
            return 0;
    }
}


/**
 * @brief PicoScope::clearStreamingAvg clears circular buffer and thus resets averaging
 */
void PicoScope::clearStreamingAvg()
{
    clearCircularBuffer = true;
}


void PicoScope::updateSettings()
{
    if(streaming)
    {
        stopStreaming();
        QThread::msleep(200);
        startStreaming();
    }
    else //If not streaming, configure for blockmode
        setup(true);
}


bool PicoScope::setup(bool blockmode)
{
    PICO_STATUS status;

    if(!internalOpen())
        return false;

    QMutexLocker locker(&picoscopeAccess);

    // stop picoscope
    status = ps6000Stop(handle);
    if(status != PICO_OK)
    {
        setLastError(picoStatusAsString(status));
        return false;
    }


    /**********************************
     *  TRIGGER SETUP FUNCTIONS
     **/

#pragma pack(push, 2)
    PS6000_TRIGGER_CONDITIONS conditions;
    conditions.channelA = settings->triggerState(PSChannel::A);
    conditions.channelB = settings->triggerState(PSChannel::B);
    conditions.channelC = settings->triggerState(PSChannel::C);
    conditions.channelD = settings->triggerState(PSChannel::D);
    conditions.aux      = settings->triggerState(PSChannel::AUX);
    conditions.external = PS6000_CONDITION_DONT_CARE;
    conditions.pulseWidthQualifier = PS6000_CONDITION_DONT_CARE;
#pragma pack(pop)

    status = ps6000SetTriggerChannelConditions(handle, &conditions, 1);
    if(status != PICO_OK)
    {
        setLastError(picoStatusAsString(status));
        return false;
    }

    PS6000_THRESHOLD_DIRECTION channelAdirection;
    if(settings->triggerState(PSChannel::A) == PS6000_CONDITION_DONT_CARE)
        channelAdirection = PS6000_NONE;
    else
        channelAdirection = settings->triggerDirection(PSChannel::A);

    PS6000_THRESHOLD_DIRECTION channelBdirection;
    if(settings->triggerState(PSChannel::B) == PS6000_CONDITION_DONT_CARE)
        channelBdirection = PS6000_NONE;
    else
        channelBdirection = settings->triggerDirection(PSChannel::B);

    PS6000_THRESHOLD_DIRECTION channelCdirection;
    if(settings->triggerState(PSChannel::C) == PS6000_CONDITION_DONT_CARE)
        channelCdirection = PS6000_NONE;
    else
        channelCdirection = settings->triggerDirection(PSChannel::C);

    PS6000_THRESHOLD_DIRECTION channelDdirection;
    if(settings->triggerState(PSChannel::D) == PS6000_CONDITION_DONT_CARE)
        channelDdirection = PS6000_NONE;
    else
        channelDdirection = settings->triggerDirection(PSChannel::D);

    PS6000_THRESHOLD_DIRECTION channelAUXdirection;
    if(settings->triggerState(PSChannel::AUX) == PS6000_CONDITION_DONT_CARE)
        channelAUXdirection = PS6000_NONE;
    else
        channelAUXdirection = settings->triggerDirection(PSChannel::AUX);

    status = ps6000SetTriggerChannelDirections(handle, channelAdirection, channelBdirection,
        channelCdirection, channelDdirection, PS6000_NONE, channelAUXdirection);

    PS6000_TRIGGER_CHANNEL_PROPERTIES properties[5];

    for(int i = 0; i < 5; i++)
    {
        PSChannel channel;
        PS6000_CHANNEL trigger_channel;

        switch(i)
        {
            case 0: channel = PSChannel::A;     trigger_channel = PS6000_CHANNEL_A; break;
            case 1: channel = PSChannel::B;     trigger_channel = PS6000_CHANNEL_B; break;
            case 2: channel = PSChannel::C;     trigger_channel = PS6000_CHANNEL_C; break;
            case 3: channel = PSChannel::D;     trigger_channel = PS6000_CHANNEL_D; break;
            case 4: channel = PSChannel::AUX;   trigger_channel = PS6000_TRIGGER_AUX; break;
        }

        properties[i].thresholdUpper    = settings->upperThresholdADC(channel);
        properties[i].hysteresisUpper   = settings->upperHysteresisADC(channel);
        properties[i].thresholdLower    = settings->lowerThresholdADC(channel);
        properties[i].hysteresisLower   = settings->lowerHysteresisADC(channel);
        properties[i].channel           = trigger_channel;
        properties[i].thresholdMode     = settings->triggerMode(channel);
    }

    status = ps6000SetTriggerChannelProperties(handle, properties, 5, NULL, 0);
    if(status != PICO_OK)
    {
        setLastError(picoStatusAsString(status));
        return false;
    }

    /**********************************
     *  CHANNEL SETUP FUNCTIONS
     **/

    PS6000_COUPLING coupling = getCoupling(settings->coupling());

    status = ps6000SetChannel(handle, PS6000_CHANNEL_A, settings->channel(PSChannel::A), coupling, getRange(settings->range(PSChannel::A)), settings->offset(PSChannel::A), PS6000_BW_FULL);
    if(status != PICO_OK)
    {
        setLastError(picoStatusAsString(status));
        return false;
    }

    status = ps6000SetChannel(handle, PS6000_CHANNEL_B, settings->channel(PSChannel::B), coupling, getRange(settings->range(PSChannel::B)), settings->offset(PSChannel::B), PS6000_BW_FULL);
    if(status != PICO_OK)
    {
        setLastError(picoStatusAsString(status));
        return false;
    }

    status = ps6000SetChannel(handle, PS6000_CHANNEL_C, settings->channel(PSChannel::C), coupling, getRange(settings->range(PSChannel::C)), settings->offset(PSChannel::C), PS6000_BW_FULL);
    if(status != PICO_OK)
    {
        setLastError(picoStatusAsString(status));
        return false;
    }

    status = ps6000SetChannel(handle, PS6000_CHANNEL_D, settings->channel(PSChannel::D), coupling, getRange(settings->range(PSChannel::D)), settings->offset(PSChannel::D), PS6000_BW_FULL);
    if(status != PICO_OK)
    {
        setLastError(picoStatusAsString(status));
        return false;
    }

    /* Calculate timebase according to dt, see ps6000pg section 3.7:
     * _____________________________________________________________________
     * timebase     sample interval formula         sample interval examples
     * _____________________________________________________________________
     * 0 to 4       2^timebase / 5,000,000,000      0 => 200 ps
     *                                              1 => 400 ps
     *                                              2 => 800 ps
     *                                              3 => 1.6 ns
     *                                              4 => 3.2 ns
     * _____________________________________________________________________
     * 5 to 2^32-1  (timebase - 4) / 156,250,000    5 => 6.4 ns
     *                                              ...
     *                                              232-1 => ~ 6.87 s
     * _____________________________________________________________________
     */

    double sampleInterval;

    if(settings->timebase() >= 5)
        sampleInterval = (double)(settings->timebase() - 4) / 156250000;
    else
        sampleInterval = (double)(pow(2, settings->timebase())) / 5E9;

    /* Calculate approximately needed samples:
     * (capturingTime/div * 10) / sampling interval     */
    uint32_t samples = (settings->collectionTime() * 10) / sampleInterval;

    float timeIntervalNanoseconds;
    uint32_t maxSamples;


    /* SETUP RAPID BLOCKMODE */

    /* In this mode signals as long a trigger state is true signals are collected sequentially with a minimum gap
     * Avoid this by using "Rising Edge" as trigger type or shorter trigger signals <= measurement interval
     *
     *  Example: Trigger signal (above upper threshold) lasts 100 µs -> Measurement interval is only 10 µs, in this case several waveforms are collected for this trigger signal
     */

    if(blockmode)
    {        
        status = ps6000SetNoOfCaptures(handle, settings->captures());
        if(status != PICO_OK)
        {
            setLastError(QString("Error setting number of captures: ").append(picoStatusAsString(status)));
            return false;
        }

        /***
         * Divide the PicoScope memory according to how much captures are requested
         * This function allows you to divide the memory into a number
         * of segments so that the scope can store several waveforms sequentially.
         **/
        uint32_t maxSamplesPerSegment = 0;
        status = ps6000MemorySegments(handle, settings->captures(), &maxSamplesPerSegment);
        if(status != PICO_OK)
        {
            setLastError(QString("Error setting memory segments: ").append(picoStatusAsString(status)));
            return false;
        }
        else
        {
            if(maxSamplesPerSegment < samples)
            {
                setLastError("Samples per segment too small to accomodate sample count. Try to reduce sample interval.");
                return false;
            }
            else
            {
                settings->setSamples(samples);
            }
        }


        for(int i; i < settings->captures(); i++)
        {
            status = ps6000GetTimebase2(handle,
                                        settings->timebase(),
                                        samples,
                                        &timeIntervalNanoseconds,
                                        0,
                                        &maxSamples,
                                        i // segmentIndex
                                        );
            if(status != PICO_OK)
            {
                setLastError(QString("Error getting timebase (SegIdx=%0): ").arg(i).append(picoStatusAsString(status)));
                return false;
            }
            else
            {
                settings->setTimeInterval(timeIntervalNanoseconds);
                if(samples <= maxSamples)
                    settings->setSamples(samples);
                else
                {
                    setLastError(QString("Error setting samples (SegIdx=%0): samples > maxSamples").arg(i));
                    return false;
                }
            }
        }

        qDebug() << "Timebase: " << settings->timebase();
        qDebug() << "Samples: " << settings->samples();
        qDebug() << "Max Samples: " << maxSamples;
        qDebug() << "Max Samples per Segment: " << maxSamplesPerSegment;
        qDebug() << "Time Interval: " << settings->timeInterval();
    }

    /* SETUP STREAMING */

    else
    {
        //Set captures per run to 1, because in streaming we only need 1 capture per run
        status = ps6000SetNoOfCaptures(handle, 1);
        if(status != PICO_OK)
        {
            setLastError(QString("Error setting capture count (Streaming): ").append(picoStatusAsString(status)));
            return false;
        }

        //Set memory segments to 1
        uint32_t dontcare;
        ps6000MemorySegments(handle, 1, &dontcare);
        if(status != PICO_OK)
        {
            setLastError(QString("Error setting memory segments (Streaming): ").append(picoStatusAsString(status)));
            return false;
        }

        // set timebase
        status = ps6000GetTimebase2(handle,
                                    settings->timebase(),
                                    samples,
                                    &timeIntervalNanoseconds,
                                    0,
                                    &maxSamples,
                                    0 // segmentIndex
                                    );
        if(status != PICO_OK)
        {
            setLastError(QString("Error getting timebase (Streaming): ").append(picoStatusAsString(status)));
            return false;
        }
        else
        {
            settings->setTimeInterval(timeIntervalNanoseconds);
            if(samples <= maxSamples)
                settings->setSamples(samples);
            else
            {
                setLastError(QString("Error setting samples (Streaming): samples > maxSamples"));
                return false;
            }
        }

        //Set and clear the buffers for averaging
        circularBufferChannelA.set_capacity(settings->captures());
        circularBufferChannelA.clear();
        circularBufferChannelB.set_capacity(settings->captures());
        circularBufferChannelB.clear();
        circularBufferChannelC.set_capacity(settings->captures());
        circularBufferChannelC.clear();
        circularBufferChannelD.set_capacity(settings->captures());
        circularBufferChannelD.clear();
    }

    return true;
}


bool PicoScope::run()
{
    // check if error occurs during setup
    if(!setup(true))
        return false;
    PICO_STATUS status;

    QMutexLocker locker(&picoscopeAccess);

    int32_t timeIndisposed;
    uint32_t samplesPreTrigger  = ((double)settings->samples() / 100) * settings->presamplePercentage();
    uint32_t samplesPostTrigger = ((double)settings->samples() / 100) * (100 - settings->presamplePercentage());

    //qDebug() << "Samples " << settings->samples();
    //qDebug() << "Samples pre " << samplesPreTrigger;
    //qDebug() << "Samples post " << samplesPostTrigger;

    status = ps6000RunBlock(handle,
                                        samplesPreTrigger,
                                        samplesPostTrigger,
                                        settings->timebase(),
                                        1, //the oversampling factor, a number in the range 1 to 256.
                                        &timeIndisposed, //on exit, the time in milliseconds that the scope will spend collecting samples.
                                        0,  //segmentIndex, zero-based, specifies which memory segment to use.
                                        (ps6000BlockReady)callbackBlockReady, //lpReady, a pointer to the ps6000BlockReady callback function that the driver will call when the data has been collected
                                        this);

    if(status != PICO_OK)
    {
        emit errorMsg(QString("Error run block: ").append(picoStatusAsString(status)));
        isBlockrun(false);
        return false;
    }
    else
    {
        emit errorMsg(QString("Running block, time indisposed: ").append(QString::number(timeIndisposed)).append(" ms"));
        isBlockrun(true);
        return true;
    }

}


bool PicoScope::isOpen()
{
    return driverOpen;
}


void PicoScope::isOpen(bool driverOpen)
{
    this->driverOpen = driverOpen;
    emit statusOpenChanged(driverOpen);
}


bool PicoScope::isStreaming()
{
    return streaming;
}


void PicoScope::isStreaming(bool streaming)
{
    this->streaming = streaming;
    emit statusStreamingChanged(streaming);
}


bool PicoScope::isTriggered()
{
    return triggered;
}


void PicoScope::isTriggered(bool triggered)
{
    this->triggered = triggered;
    emit statusTriggeredChanged(triggered);
}


bool PicoScope::isBlockrun()
{
    return blockrun;
}


void PicoScope::isBlockrun(bool blockrun)
{
    this->blockrun = blockrun;
    emit statusBlockrunChanged(blockrun);
}


bool PicoScope::isProcessing()
{
    return processing;
}


void PicoScope::isProcessing(bool processing)
{
    this->processing = processing;
    emit statusProcessingChanged(processing);
}


bool PicoScope::isError()
{
    return error;
}


void PicoScope::isError(bool error)
{
    this->error = error;
    emit statusErrorChanged(error);
}


void PicoScope::setLastError(QString error)
{
    lastError = error;
    isError(true);
    emit errorMsg(lastError);
}


const QString PicoScope::getLastError()
{
    return lastError;
}


void PicoScope::startStreaming()
{
#ifdef PICOSCOPE_TEST_MODE
    streamingTest();
#else
    if(!setup(false))
        return;
    isStreaming(true);
    isError(false);
    QFuture<void> future = QtConcurrent::run(this, &PicoScope::streamingWorker);
#endif
}


void PicoScope::stopStreaming()
{
#ifdef PICOSCOPE_TEST_MODE
    streamingTest(false);
#else
    ps6000Stop(handle);
    isStreaming(false);

    streamingDataProcessed = true;

    isTriggered(false);
    isError(false);

    if(streamingLock.available() < 1)
        streamingLock.release();
#endif
}


#ifdef __MINGW32__
__stdcall void PicoScope::callbackStreamingReady(int16_t handle, PICO_STATUS status, void *pParameter)
#else
#ifdef _MSC_VER
void PicoScope::callbackStreamingReady(int16_t handle, PICO_STATUS status, void *pParameter)
#endif
#endif
{
    static_cast<PicoScope *>(pParameter)->handleStreamingReady(status);
}


void PicoScope::handleStreamingReady(PICO_STATUS status)
{
    isTriggered(true);
    if(status != PICO_OK)
    {
        setLastError(QString("Error fetching streaming run: ").append(picoStatusAsString(status)));
        streamingDataProcessed = true;
        isTriggered(false);
        return;
    }

    QMutexLocker locker(&picoscopeAccess);

    resizeBuffer(settings->samples(), settings->captures());

    QThread::msleep(5);

//    int64_t time;
//    PS6000_TIME_UNITS unit;
//    status = ps6000GetTriggerTimeOffset64(handle, &time, &unit, 0);
//    if(status != PICO_OK)
//    {
//        setLastError(picoStatusAsString(status));
//        isBlockrun(false);
//        isProcessing(false);
//        return;
//    }
//    double trigger_time = 0;

//    switch(unit)
//    {
//        case PS6000_FS: trigger_time = (double)time * pow(10, -15); break;
//        case PS6000_PS: trigger_time = (double)time * pow(10, -12); break;
//        case PS6000_NS: trigger_time = (double)time * pow(10, -9);  break;
//        case PS6000_US: trigger_time = (double)time * pow(10, -6);  break;
//        case PS6000_MS: trigger_time = (double)time * pow(10, -3);  break;
//        case PS6000_S:  trigger_time = (double)time;                break;
//    }


//    streamSignalA.start_time = -trigger_time;
//    streamSignalB.start_time = -trigger_time;
//    streamSignalC.start_time = -trigger_time;
//    streamSignalD.start_time = -trigger_time;

    streamSignalA.dt = (double)settings->timeInterval() * pow(10, -9);
    streamSignalB.dt = (double)settings->timeInterval() * pow(10, -9);
    streamSignalC.dt = (double)settings->timeInterval() * pow(10, -9);
    streamSignalD.dt = (double)settings->timeInterval() * pow(10, -9);

    streamSignalA.start_time = 0;
    streamSignalB.start_time = 0;
    streamSignalC.start_time = 0;
    streamSignalD.start_time = 0;

    //FIXME: test if this is needed
    QThread::msleep(5);

    if(settings->channel(PSChannel::A))
    {
        status = ps6000SetDataBuffer(handle, PS6000_CHANNEL_A, bufferA[0], settings->samples(), PS6000_RATIO_MODE_NONE);
        if(status != PICO_OK)
        {
            if(status == PICO_DRIVER_FUNCTION)
                restartStreaming = true;
            else
                setLastError(QString("CH A: Error setting data buffer for block streaming: ").append(picoStatusAsString(status)));
            streamingDataProcessed = true;
            return;
        }
    }

    if(settings->channel(PSChannel::B))
    {
        status = ps6000SetDataBuffer(handle, PS6000_CHANNEL_B, bufferB[0], settings->samples(), PS6000_RATIO_MODE_NONE);
        if(status != PICO_OK)
        {
            if(status == PICO_DRIVER_FUNCTION)
                restartStreaming = true;
            else
                setLastError(QString("CH B: Error setting data buffer for block streaming: ").append(picoStatusAsString(status)));
            streamingDataProcessed = true;
            return;
        }
    }

    if(settings->channel(PSChannel::C))
    {
        status = ps6000SetDataBuffer(handle, PS6000_CHANNEL_C, bufferC[0], settings->samples(), PS6000_RATIO_MODE_NONE);
        if(status != PICO_OK)
        {
            if(status == PICO_DRIVER_FUNCTION)
                restartStreaming = true;
            else
                setLastError(QString("CH C: Error setting data buffer for block streaming: ").append(picoStatusAsString(status)));
            streamingDataProcessed = true;
            return;
        }
    }

    if(settings->channel(PSChannel::D))
    {
        status = ps6000SetDataBuffer(handle, PS6000_CHANNEL_D, bufferD[0], settings->samples(), PS6000_RATIO_MODE_NONE);
        if(status != PICO_OK)
        {
            if(status == PICO_DRIVER_FUNCTION)
                restartStreaming = true;
            else
                setLastError(QString("CH D: Error setting data buffer for block streaming: ").append(picoStatusAsString(status)));
            streamingDataProcessed = true;
            return;
        }
    }

    uint32_t samplesInOut = settings->samples();
    short overflow;

    status = ps6000GetValues(handle, 0, &samplesInOut, 1, PS6000_RATIO_MODE_NONE, 0, &overflow);
    if(status != PICO_OK)
    {
        if(status == PICO_DRIVER_FUNCTION)
            restartStreaming = true;
        else
            setLastError(QString("Error getting block streaming values: ").append(picoStatusAsString(status)));

        streamingDataProcessed = true;
        return;
    }

    if(circularBufferChannelA.capacity() != settings->getAveragingBufferSize())
    {
        circularBufferChannelA.set_capacity(settings->getAveragingBufferSize());
        circularBufferChannelB.set_capacity(settings->getAveragingBufferSize());
        circularBufferChannelC.set_capacity(settings->getAveragingBufferSize());
        circularBufferChannelD.set_capacity(settings->getAveragingBufferSize());
    }

    if(clearCircularBuffer)
    {        
        circularBufferChannelA.clear();
        circularBufferChannelB.clear();
        circularBufferChannelC.clear();
        circularBufferChannelD.clear();

        clearCircularBuffer = false;

        lastOverflow[0] = false;
        lastOverflow[1] = false;
        lastOverflow[2] = false;
        lastOverflow[3] = false;

        lastOverflowCounter[0] = 0;
        lastOverflowCounter[1] = 0;
        lastOverflowCounter[2] = 0;
        lastOverflowCounter[3] = 0;
    }

    double rangeFactor;

    // CHANNEL A
    rangeFactor = getRangeFactor(settings->range(PSChannel::A));
    streamSignalA.data.clear();
    for(unsigned int i = 0; i < samplesInOut; i++)
        streamSignalA.data.append(bufferA[0][i] / rangeFactor);

    circularBufferChannelA.push_back(streamSignalA);

    // CHANNEL B
    rangeFactor = getRangeFactor(settings->range(PSChannel::B));
    streamSignalB.data.clear();
    for(unsigned int i = 0; i < samplesInOut; i++)
        streamSignalB.data.append(bufferB[0][i] / rangeFactor);

    circularBufferChannelB.push_back(streamSignalB);

    // CHANNEL C
    rangeFactor = getRangeFactor(settings->range(PSChannel::C));
    streamSignalC.data.clear();
    for(unsigned int i = 0; i < samplesInOut; i++)
        streamSignalC.data.append(bufferC[0][i] / rangeFactor);

    circularBufferChannelC.push_back(streamSignalC);

    // CHANNEL D
    rangeFactor = getRangeFactor(settings->range(PSChannel::D));
    streamSignalD.data.clear();
    for(unsigned int i = 0; i < samplesInOut; i++)
        streamSignalD.data.append(bufferD[0][i] / rangeFactor);

    circularBufferChannelD.push_back(streamSignalD);

    // emit current size of circular buffer for GUI
    // DataAcquisitionWindow: SLOT(updateSignalCounter(int)
    //emit updateSignalCounter(circularBufferChannelA.size());


    /*if(settings->streaming_mode == PSStreamingMode::AverageMeasurement || settings->streaming_mode == PSStreamingMode::Both)
        calculateAverage();*/

    /*switch(settings->streaming_mode)
    {
    case PSStreamingMode::SingleMeasurement:
        emit streamingData(streamSignalA, streamSignalB, streamSignalC, streamSignalD, false);
        break;

    case PSStreamingMode::AverageMeasurement:
        emit streamingData(averageStreamSignalA, averageStreamSignalB, averageStreamSignalC, averageStreamSignalD, true);
        break;

    case PSStreamingMode::Both:
                emit streamingData(streamSignalA, streamSignalB, streamSignalC, streamSignalD, averageStreamSignalA, averageStreamSignalB,
                           averageStreamSignalC, averageStreamSignalD);
        break;
    }*/

    calculateAverage();

    StreamPoint point;
    point.averageBufferFilling = circularBufferChannelA.size();

    /*if(overflow & 1)
        point.overflowA = true;
    if(overflow & 2)
        point.overflowB = true;
    if(overflow & 3)
        point.overflowC = true;
    if(overflow & 4)
        point.overflowD = true;*/

    if(overflow & 1)
    {
        lastOverflow[0] = true;
        lastOverflowCounter[0] = 0;
    }
    else if(lastOverflow[0])
    {
        if(lastOverflowCounter[0] > circularBufferChannelA.size())
            lastOverflow[0] = false;
        else
            lastOverflowCounter[0]++;
    }

    if(overflow & 2)
    {
        lastOverflow[1] = true;
        lastOverflowCounter[1] = 0;
    }
    else if(lastOverflow[1])
    {
        if(lastOverflowCounter[1] > circularBufferChannelB.size())
            lastOverflow[1] = false;
        else
            lastOverflowCounter[1]++;
    }

    if(overflow & 3)
    {
        lastOverflow[2] = true;
        lastOverflowCounter[2] = 0;
    }
    else if(lastOverflow[2])
    {
        if(lastOverflowCounter[2] > circularBufferChannelC.size())
            lastOverflow[2] = false;
        else
            lastOverflowCounter[2]++;
    }

    if(overflow & 4)
    {
        lastOverflow[3] = true;
        lastOverflowCounter[3] = 0;
    }
    else if(lastOverflow[3])
    {
        if(lastOverflowCounter[3] > circularBufferChannelD.size())
            lastOverflow[3] = false;
        else
            lastOverflowCounter[3]++;
    }

    point.overflowA = lastOverflow[0];
    point.overflowB = lastOverflow[1];
    point.overflowC = lastOverflow[2];
    point.overflowD = lastOverflow[3];

    switch(settings->streaming_mode)
    {
    case PSStreamingMode::SingleMeasurement:
    {
        //StreamPoint point;
        //point.averageBufferFilling = circularBufferChannelA.size();

        if(settings->channel(PSChannel::A))
            point.single.insert(1, streamSignalA);
        if(settings->channel(PSChannel::B))
            point.single.insert(2, streamSignalB);
        if(settings->channel(PSChannel::C))
            point.single.insert(3, streamSignalC);
        if(settings->channel(PSChannel::D))
            point.single.insert(4, streamSignalD);

        emit streamingData(point);
    }
    break;

    case PSStreamingMode::AverageMeasurement:
    {
        //StreamPoint point;
        //point.averageBufferFilling = circularBufferChannelA.size();

        if(settings->channel(PSChannel::A))
            point.average.insert(1, averageStreamSignalA);
        if(settings->channel(PSChannel::B))
            point.average.insert(2, averageStreamSignalB);
        if(settings->channel(PSChannel::C))
            point.average.insert(3, averageStreamSignalC);
        if(settings->channel(PSChannel::D))
            point.average.insert(4, averageStreamSignalD);

        emit streamingData(point);
    }
    break;

    case PSStreamingMode::Both:
    {
        //StreamPoint point;
        //point.averageBufferFilling = circularBufferChannelA.size();

        if(settings->channel(PSChannel::A))
            point.single.insert(1, streamSignalA);
        if(settings->channel(PSChannel::B))
            point.single.insert(2, streamSignalB);
        if(settings->channel(PSChannel::C))
            point.single.insert(3, streamSignalC);
        if(settings->channel(PSChannel::D))
            point.single.insert(4, streamSignalD);

        if(settings->channel(PSChannel::A))
            point.average.insert(1, averageStreamSignalA);
        if(settings->channel(PSChannel::B))
            point.average.insert(2, averageStreamSignalB);
        if(settings->channel(PSChannel::C))
            point.average.insert(3, averageStreamSignalC);
        if(settings->channel(PSChannel::D))
            point.average.insert(4, averageStreamSignalD);

        emit streamingData(point);
    }
    break;
    }
    streamingDataProcessed = true;
    isTriggered(false);

    streamingLock.release();
}


void PicoScope::streamingWorker()
{
    uint32_t samplesPreTrigger  = ((double)settings->samples() / 100) * settings->presamplePercentage();
    uint32_t samplesPostTrigger = ((double)settings->samples() / 100) * (100 - settings->presamplePercentage());

    if(streamingLock.available() < 1)
        streamingLock.release();

    while(streaming)
    {
        streamingLock.acquire();
        if(streaming)
        {
            picoscopeAccess.lock();

            if(restartStreaming)
            {
                qDebug() << "[PS] restarting streaming";
                ps6000Stop(handle);
                restartStreaming = false;
                QThread::msleep(RESTART_STREAMING_WAIT_MS);
            }

            if(streamingDataProcessed)
            {
                int32_t timeIndisposed;

                PICO_STATUS status = ps6000RunBlock(handle, samplesPreTrigger, samplesPostTrigger, settings->timebase(), 1, &timeIndisposed, 0, (ps6000BlockReady)callbackStreamingReady, this);
                if(status != PICO_OK)
                {
                    setLastError(QString("Could not start new streaming run: ").append(picoStatusAsString(status)));
                    stopStreaming();
                }
                else
                {
                    streamingDataProcessed = false;
                }
            }

            picoscopeAccess.unlock();
        }
        //QThread::msleep(MAX_STREAMING_UPDATE_RATE_MS);
    }
}


void PicoScope::blockWorker(MRun *run, bool averageCaptures, bool calculateStdev)
{
    isProcessing(true);
    PICO_STATUS status;

    resizeBuffer(settings->samples(), settings->captures());

    int16_t *overflow = new int16_t[settings->captures()];

    // set data buffers for all captures
    for(unsigned int i = 0; i < settings->captures(); i++)
    {
        if(settings->channel(PSChannel::A))
        {
            status = ps6000SetDataBufferBulk(handle, PS6000_CHANNEL_A, bufferA[i], settings->samples(), i, PS6000_RATIO_MODE_NONE);
            if(status != PICO_OK)
            {
                setLastError(picoStatusAsString(status));
                isBlockrun(false);
                isProcessing(false);
                return;
            }
        }

        if(settings->channel(PSChannel::B))
        {
            status = ps6000SetDataBufferBulk(handle, PS6000_CHANNEL_B, bufferB[i], settings->samples(), i, PS6000_RATIO_MODE_NONE);
            if(status != PICO_OK)
            {
                setLastError(picoStatusAsString(status));
                isBlockrun(false);
                isProcessing(false);
                return;
            }
        }

        if(settings->channel(PSChannel::C))
        {
            status = ps6000SetDataBufferBulk(handle, PS6000_CHANNEL_C, bufferC[i], settings->samples(), i, PS6000_RATIO_MODE_NONE);
            if(status != PICO_OK)
            {
                setLastError(picoStatusAsString(status));
                isBlockrun(false);
                isProcessing(false);
                return;
            }
        }

        if(settings->channel(PSChannel::D))
        {
            status = ps6000SetDataBufferBulk(handle, PS6000_CHANNEL_D, bufferD[i], settings->samples(), i, PS6000_RATIO_MODE_NONE);
            if(status != PICO_OK)
            {
                setLastError(picoStatusAsString(status));
                isBlockrun(false);
                isProcessing(false);
                return;
            }
        }
    }   // for captures


    uint32_t samplesInOut = settings->samples();

    status = ps6000GetValuesBulk(handle,
                                 &samplesInOut,         // on entry, the number of samples required;
                                 0,                     // first segment from which the waveform should be retrieved
                                 settings->captures()-1,  // last segment from which the waveform should be retrieved
                                 1,                     //downSampleRatio
                                 PS6000_RATIO_MODE_NONE,
                                 &overflow[0]);

    if(status != PICO_OK)
    {
        setLastError(picoStatusAsString(status));
        isBlockrun(false);
        return;
    }

    //calculate standard deviation
    QVector<double> stdevA;
    QVector<double> stdevB;
    QVector<double> stdevC;
    QVector<double> stdevD;

    if(calculateStdev)
    {
        for(int i = 0; i < samplesInOut; i++)
        {
            //channel A
            if(settings->channel(PSChannel::A))
            {
                double rangeFactor = getRangeFactor(settings->range(PSChannel::A));

                StandardDeviation stdev;

                for(int j = 0; j < settings->captures(); j++)
                    stdev.addValue(bufferA[j][i] / rangeFactor);

                stdevA.push_back(stdev.getStandardDeviation());
            }

            if(settings->channel(PSChannel::B))
            {
                double rangeFactor = getRangeFactor(settings->range(PSChannel::B));

                StandardDeviation stdev;

                for(int j = 0; j < settings->captures(); j++)
                    stdev.addValue(bufferB[j][i] / rangeFactor);

                stdevB.push_back(stdev.getStandardDeviation());
            }

            if(settings->channel(PSChannel::C))
            {
                double rangeFactor = getRangeFactor(settings->range(PSChannel::C));

                StandardDeviation stdev;

                for(int j = 0; j < settings->captures(); j++)
                    stdev.addValue(bufferC[j][i] / rangeFactor);

                stdevC.push_back(stdev.getStandardDeviation());
            }

            if(settings->channel(PSChannel::D))
            {
                double rangeFactor = getRangeFactor(settings->range(PSChannel::D));

                StandardDeviation stdev;

                for(int j = 0; j < settings->captures(); j++)
                    stdev.addValue(bufferD[j][i] / rangeFactor);

                stdevD.push_back(stdev.getStandardDeviation());
            }
        }
    }

    /***************
     *  AVERAGE MODE
     **/
    if(averageCaptures)
    {
        Signal signalChannelA(0, (double)settings->timeInterval() * pow(10, -9), 0);
        Signal signalChannelB(0, (double)settings->timeInterval() * pow(10, -9), 0);
        Signal signalChannelC(0, (double)settings->timeInterval() * pow(10, -9), 0);
        Signal signalChannelD(0, (double)settings->timeInterval() * pow(10, -9), 0);

        for(unsigned int i = 0; i < settings->captures(); i++)
        {
            if(settings->channel(PSChannel::A))
            {
                double rangeFactor = getRangeFactor(settings->range(PSChannel::A));
                bool vectorIsEmpty = signalChannelA.data.isEmpty();

                for(unsigned int j = 0; j < samplesInOut; j++)
                {
                    if(vectorIsEmpty)
                        signalChannelA.data.append((bufferA[i][j] / rangeFactor) / settings->captures());
                    else if(j < signalChannelA.data.size())
                        signalChannelA.data.replace(j, signalChannelA.data[j] + ((bufferA[i][j] / rangeFactor) / settings->captures()));
                }

                signalChannelA.stdev = stdevA;
            }

            if(settings->channel(PSChannel::B))
            {
                double rangeFactor = getRangeFactor(settings->range(PSChannel::B));
                bool vectorIsEmpty = signalChannelB.data.isEmpty();

                for(unsigned int j = 0; j < samplesInOut; j++)
                {
                    if(vectorIsEmpty)
                        signalChannelB.data.append((bufferB[i][j] / rangeFactor) / settings->captures());
                    else if(j < signalChannelB.data.size())
                        signalChannelB.data.replace(j, signalChannelB.data[j] + ((bufferB[i][j] / rangeFactor) / settings->captures()));
                }

                signalChannelB.stdev = stdevB;
            }

            if(settings->channel(PSChannel::C))
            {
                double rangeFactor = getRangeFactor(settings->range(PSChannel::C));
                bool vectorIsEmpty = signalChannelC.data.isEmpty();

                for(unsigned int j = 0; j < samplesInOut; j++)
                {
                    if(vectorIsEmpty)
                        signalChannelC.data.append((bufferC[i][j] / rangeFactor) / settings->captures());
                    else if(j < signalChannelC.data.size())
                        signalChannelC.data.replace(j, signalChannelC.data[j] + ((bufferC[i][j] / rangeFactor) / settings->captures()));
                }

                signalChannelC.stdev = stdevC;
            }

            if(settings->channel(PSChannel::D))
            {
                double rangeFactor = getRangeFactor(settings->range(PSChannel::D));
                bool vectorIsEmpty = signalChannelD.data.isEmpty();

                for(unsigned int j = 0; j < samplesInOut; j++)
                {
                    if(vectorIsEmpty)
                        signalChannelD.data.append((bufferD[i][j] / rangeFactor) / settings->captures());
                    else if(j < signalChannelD.data.size())
                        signalChannelD.data.replace(j, signalChannelD.data[j] + ((bufferD[i][j] / rangeFactor) / settings->captures()));
                }

                signalChannelD.stdev = stdevD;
            }
        }

        MPoint *mp = run->getCreatePre(0);
        int channelCounter = 1;

        if(settings->channel(PSChannel::A))
        {
            signalChannelA.type = Signal::RAW;
            signalChannelA.channelID = channelCounter;
            mp->setSignal(signalChannelA, channelCounter, Signal::RAW);

            signalChannelA.type = Signal::ABS;
            mp->setSignal(signalChannelA, channelCounter, Signal::ABS);

            channelCounter++;
        }

        if(settings->channel(PSChannel::B))
        {
            signalChannelB.type = Signal::RAW;
            signalChannelB.channelID = channelCounter;
            mp->setSignal(signalChannelB, channelCounter, Signal::RAW);
            signalChannelB.type = Signal::ABS;
            mp->setSignal(signalChannelB, channelCounter, Signal::ABS);
            channelCounter++;
        }

        if(settings->channel(PSChannel::C))
        {
            signalChannelC.type = Signal::RAW;
            signalChannelC.channelID = channelCounter;
            mp->setSignal(signalChannelC, channelCounter, Signal::RAW);
            signalChannelC.type = Signal::ABS;
            mp->setSignal(signalChannelC, channelCounter, Signal::ABS);
            channelCounter++;
        }

        if(settings->channel(PSChannel::D))
        {
            signalChannelD.type = Signal::RAW;
            signalChannelD.channelID = channelCounter;
            mp->setSignal(signalChannelD, channelCounter, Signal::RAW);
            signalChannelD.type = Signal::ABS;
            mp->setSignal(signalChannelD, channelCounter, Signal::ABS);
        }
    }    
    /***************
     *  NORMAL MODE
     **/
    else
    {       
        for(unsigned int i = 0; i < settings->captures(); i++)
        {
            int64_t time;
            PS6000_TIME_UNITS unit;
            double trigger_time = 0.0;

            status = ps6000GetTriggerTimeOffset64(handle, &time, &unit, i);
            if(status != PICO_OK)
            {
                setLastError(picoStatusAsString(status));
                isBlockrun(false);
                isProcessing(false);
                return;
            }

            switch(unit)
            {
                case PS6000_FS: trigger_time = (double)time * pow(10, -15); break;
                case PS6000_PS: trigger_time = (double)time * pow(10, -12); break;
                case PS6000_NS: trigger_time = (double)time * pow(10, -9);  break;
                case PS6000_US: trigger_time = (double)time * pow(10, -6);  break;
                case PS6000_MS: trigger_time = (double)time * pow(10, -3);  break;
                case PS6000_S:  trigger_time = (double)time;                break;
            }
            //qDebug() << "Trigger point = " << trigger_time;


            MPoint *mp = run->getCreatePre(i);


            int channelCounter = 1;

            if(settings->channel(PSChannel::A))
            {
                Signal signal(0, (double)settings->timeInterval() * pow(10, -9), -trigger_time);

                if(!signal.data.isEmpty())
                    signal.data.clear();

                signal.stdev = stdevA;

                signal.channelID = channelCounter;
                signal.type = Signal::RAW;

                qDebug() << "fetching " << samplesInOut << " samples for channel A";

                double rangeFactor = getRangeFactor(settings->range(PSChannel::A));

                for(unsigned int j = 0; j < samplesInOut; j++)
                {
                    signal.data.append(bufferA[i][j] / rangeFactor);
                }
                mp->setSignal(signal, channelCounter, Signal::RAW);

                signal.type = Signal::ABS;
                mp->setSignal(signal, channelCounter, Signal::ABS);

                channelCounter++;
            }

            if(settings->channel(PSChannel::B))
            {
                Signal signal(0, (double)settings->timeInterval() * pow(10, -9), -trigger_time);

                if(!signal.data.isEmpty())
                    signal.data.clear();

                signal.stdev = stdevB;

                signal.channelID = channelCounter;
                signal.type = Signal::RAW;

                //qDebug() << "fetching " << samplesInOut << " samples for channel B";

                double rangeFactor = getRangeFactor(settings->range(PSChannel::B));

                for(unsigned int j = 0; j < samplesInOut; j++)
                {
                    signal.data.append(bufferB[i][j] / rangeFactor);
                }
                mp->setSignal(signal, channelCounter, Signal::RAW);

                signal.type = Signal::ABS;
                mp->setSignal(signal, channelCounter, Signal::ABS);

                channelCounter++;
            }

            if(settings->channel(PSChannel::C))
            {
                Signal signal(0, (double)settings->timeInterval() * pow(10, -9), -trigger_time);

                if(!signal.data.isEmpty())
                    signal.data.clear();

                signal.stdev = stdevC;

                signal.channelID = channelCounter;
                signal.type = Signal::RAW;

                //qDebug() << "fetching " << samplesInOut << " samples for channel C";

                double rangeFactor = getRangeFactor(settings->range(PSChannel::C));

                for(unsigned int j = 0; j < samplesInOut; j++)
                {
                    signal.data.append(bufferC[i][j] / rangeFactor);
                }
                mp->setSignal(signal, channelCounter, Signal::RAW);

                signal.type = Signal::ABS;
                mp->setSignal(signal, channelCounter, Signal::ABS);

                channelCounter++;
            }

            if(settings->channel(PSChannel::D))
            {
                Signal signal(0, (double)settings->timeInterval() * pow(10, -9), -trigger_time);

                if(!signal.data.isEmpty())
                    signal.data.clear();

                signal.stdev = stdevD;

                signal.channelID = channelCounter;
                signal.type = Signal::RAW;

                //qDebug() << "fetching " << samplesInOut << " samples for channel D";

                double rangeFactor = getRangeFactor(settings->range(PSChannel::D));

                for(unsigned int j = 0; j < samplesInOut; j++)
                {
                    signal.data.append(bufferD[i][j] / rangeFactor);
                }
                mp->setSignal(signal, channelCounter, Signal::RAW);

                signal.type = Signal::ABS;
                mp->setSignal(signal, channelCounter, Signal::ABS);
            }
        }


    }

    isBlockrun(false);
    isProcessing(false);
    emit processingFinished(run);
}


#ifdef __MINGW32__
__stdcall void PicoScope::callbackBlockReady(int16_t handle, PICO_STATUS status, void *pParameter)
#else
 #ifdef _MSC_VER
void PicoScope::callbackBlockReady(int16_t handle, PICO_STATUS status, void *pParameter)
 #endif
#endif
{
    static_cast<PicoScope *>(pParameter)->handleBlockReady(status);
}


void PicoScope::handleBlockReady(PICO_STATUS status)
{
    if(status != PICO_OK)
    {
        setLastError(QString("Callback for Blockready returned: ").append(picoStatusAsString(status)));
    }
    else
    {
        emit runReady();
    }
}


double PicoScope::getRangeFactor(PSRange range)
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


PS6000_COUPLING PicoScope::getCoupling(PSCoupling coupling)
{
    switch(coupling)
    {
        case PSCoupling::AC:    return PS6000_AC;       break;
        case PSCoupling::DC1M:  return PS6000_DC_1M;    break;
        case PSCoupling::DC50R: return PS6000_DC_50R;   break;
    }
}


PS6000_RANGE PicoScope::getRange(PSRange range)
{
    switch(range)
    {
        case PSRange::R50mV:    return PS6000_50MV;    break;
        case PSRange::R100mV:   return PS6000_100MV;   break;
        case PSRange::R200mV:   return PS6000_200MV;   break;
        case PSRange::R500mV:   return PS6000_500MV;   break;
        case PSRange::R1V:      return PS6000_1V;      break;
        case PSRange::R2V:      return PS6000_2V;      break;
        case PSRange::R5V:      return PS6000_5V;      break;
        case PSRange::R10V:     return PS6000_10V;     break;
        case PSRange::R20V:     return PS6000_20V;     break;
    }
}

/**
 * @brief PicoScope::resizeBuffer Resizes the buffer for the captured data
 * @param size Size of the buffer
 * @return True if resizing was successfull, otherwise false
 */
bool PicoScope::resizeBuffer(uint32_t size, uint32_t captures)
{
    /*if(bufferSize < size)
    {
        try
        {
            if(bufferSize > 0)
            {
                for (int i = 0; i < captures; ++i)
                {
                    delete[] bufferA[i];
                    delete[] bufferB[i];
                    delete[] bufferC[i];
                    delete[] bufferD[i];
                }
                delete[] bufferA;
                delete[] bufferB;
                delete[] bufferC;
                delete[] bufferD;
            }

            bufferA = new int16_t*[captures];
            bufferB = new int16_t*[captures];
            bufferC = new int16_t*[captures];
            bufferD = new int16_t*[captures];

            for (int i = 0; i < captures; ++i)
            {
                bufferA[i] = new int16_t[size];
                bufferB[i] = new int16_t[size];
                bufferC[i] = new int16_t[size];
                bufferD[i] = new int16_t[size];
            }
            bufferSize = size;
        }
        catch(std::bad_alloc)
        {
            setLastError("Could not allocate memory to resize buffer");
            return false;
        }
        return true;
    }
    return true;*/


    if(bufferSize < size || bufferCaptures < captures)
    {
        try
        {
            if(bufferSize > 0 && bufferCaptures > 0)
            {
                for (int i = 0; i < bufferCaptures; ++i)
                {
                    delete[] bufferA[i];
                    delete[] bufferB[i];
                    delete[] bufferC[i];
                    delete[] bufferD[i];
                }
                delete[] bufferA;
                delete[] bufferB;
                delete[] bufferC;
                delete[] bufferD;
            }

            bufferA = new int16_t*[captures];
            bufferB = new int16_t*[captures];
            bufferC = new int16_t*[captures];
            bufferD = new int16_t*[captures];

            for (int i = 0; i < captures; ++i)
            {
                bufferA[i] = new int16_t[size];
                bufferB[i] = new int16_t[size];
                bufferC[i] = new int16_t[size];
                bufferD[i] = new int16_t[size];
            }
            bufferSize = size;
            bufferCaptures = captures;
        }
        catch(std::bad_alloc)
        {
            setLastError("Could not allocate memory to resize buffer");
            return false;
        }
    }
    return true;
}


void PicoScope::calculateAverage()
{
    if(calculationThreads[0]->newAverageAvailable())
        averageStreamSignalA = calculationThreads[0]->getAverage();

    if(calculationThreads[1]->newAverageAvailable())
        averageStreamSignalB = calculationThreads[1]->getAverage();

    if(calculationThreads[2]->newAverageAvailable())
        averageStreamSignalC = calculationThreads[2]->getAverage();

    if(calculationThreads[3]->newAverageAvailable())
        averageStreamSignalD = calculationThreads[3]->getAverage();

    if(!calculationThreads[0]->isRunning() && !calculationThreads[1]->isRunning()
            && !calculationThreads[2]->isRunning() && !calculationThreads[3]->isRunning())
    {
        calculationThreads[0]->setBufferData(circularBufferChannelA);
        calculationThreads[1]->setBufferData(circularBufferChannelB);
        calculationThreads[2]->setBufferData(circularBufferChannelC);
        calculationThreads[3]->setBufferData(circularBufferChannelD);

        calculationThreads[0]->start();
        calculationThreads[1]->start();
        calculationThreads[2]->start();
        calculationThreads[3]->start();
    }
}


/**
 * @brief PicoScope::getLastAverage returns the last calculated average signal
 * @return StreamPoint
 */
StreamPoint PicoScope::getLastAverage()
{
    QMutexLocker locker(&averageLock);

    StreamPoint point;

    if(settings->channel(PSChannel::A))
        point.average.insert(1, averageStreamSignalA);
    if(settings->channel(PSChannel::B))
        point.average.insert(2, averageStreamSignalB);
    if(settings->channel(PSChannel::C))
        point.average.insert(3, averageStreamSignalC);
    if(settings->channel(PSChannel::D))
        point.average.insert(4, averageStreamSignalD);

    return point;
}


void PicoScope::getStreamBufferContent(MRun *run, bool asAverage)
{
    if(circularBufferChannelA.size() == 0)
        throw LIISimException("Can not save empty streaming buffer", LIISimMessageType::ERR);

    if(asAverage)
    {
        int channelIDCount = 1;
        MPoint *mp = run->getCreatePre(0);

        for(int i = 0; i < 4; i++)
            calculationThreads[i]->wait();

        for(int i = 0; i < 4; i++)
            calculationThreads[i]->calcStdevNext();

        calculationThreads[0]->setBufferData(circularBufferChannelA);
        calculationThreads[1]->setBufferData(circularBufferChannelB);
        calculationThreads[2]->setBufferData(circularBufferChannelC);
        calculationThreads[3]->setBufferData(circularBufferChannelD);

        for(int i = 0; i < 4; i++)
            calculationThreads[i]->start();

        for(int i = 0; i < 4; i++)
            calculationThreads[i]->wait();

        QMutexLocker locker(&averageLock);

        if(settings->channel(PSChannel::A))
        {
            Signal signal = calculationThreads[0]->getAverage();
            signal.channelID = channelIDCount;

            signal.type = Signal::RAW;
            mp->setSignal(signal, channelIDCount, Signal::RAW);

            signal.type = Signal::ABS;
            mp->setSignal(signal, channelIDCount, Signal::ABS);

            channelIDCount++;
        }

        if(settings->channel(PSChannel::B))
        {
            Signal signal = calculationThreads[1]->getAverage();
            signal.channelID = channelIDCount;

            signal.type = Signal::RAW;
            mp->setSignal(signal, channelIDCount, Signal::RAW);

            signal.type = Signal::ABS;
            mp->setSignal(signal, channelIDCount, Signal::ABS);

            channelIDCount++;
        }

        if(settings->channel(PSChannel::C))
        {
            Signal signal = calculationThreads[2]->getAverage();
            signal.channelID = channelIDCount;

            signal.type = Signal::RAW;
            mp->setSignal(signal, channelIDCount, Signal::RAW);

            signal.type = Signal::ABS;
            mp->setSignal(signal, channelIDCount, Signal::ABS);

            channelIDCount++;
        }

        if(settings->channel(PSChannel::D))
        {
            Signal signal = calculationThreads[3]->getAverage();
            signal.channelID = channelIDCount;

            signal.type = Signal::RAW;
            mp->setSignal(signal, channelIDCount, Signal::RAW);

            signal.type = Signal::ABS;
            mp->setSignal(signal, channelIDCount, Signal::ABS);
        }

    }
    else
    {
        for(int i = 0; i < circularBufferChannelA.size(); i++)
        {
            int channelIDCount = 1;
            MPoint *mp = run->getCreatePre(i);

            if(settings->channel(PSChannel::A))
            {
                Signal signal = circularBufferChannelA.at(i);
                signal.channelID = channelIDCount;

                signal.type = Signal::RAW;
                mp->setSignal(signal, channelIDCount, Signal::RAW);

                signal.type = Signal::ABS;
                mp->setSignal(signal, channelIDCount, Signal::ABS);

                channelIDCount++;
            }

            if(settings->channel(PSChannel::B))
            {
                Signal signal = circularBufferChannelB.at(i);
                signal.channelID = channelIDCount;

                signal.type = Signal::RAW;
                mp->setSignal(signal, channelIDCount, Signal::RAW);

                signal.type = Signal::ABS;
                mp->setSignal(signal, channelIDCount, Signal::ABS);

                channelIDCount++;
            }

            if(settings->channel(PSChannel::C))
            {
                Signal signal = circularBufferChannelC.at(i);
                signal.channelID = channelIDCount;

                signal.type = Signal::RAW;
                mp->setSignal(signal, channelIDCount, Signal::RAW);

                signal.type = Signal::ABS;
                mp->setSignal(signal, channelIDCount, Signal::ABS);

                channelIDCount++;
            }

            if(settings->channel(PSChannel::D))
            {
                Signal signal = circularBufferChannelD.at(i);
                signal.channelID = channelIDCount;

                signal.type = Signal::RAW;
                mp->setSignal(signal, channelIDCount, Signal::RAW);

                signal.type = Signal::ABS;
                mp->setSignal(signal, channelIDCount, Signal::ABS);
            }
        }
    }
}


QString PicoScope::picoStatusAsString(PICO_STATUS status)
{
    switch(status)
    {
    case PICO_OK: return "OK"; break;
    case PICO_MEMORY_FAIL: return "Not enough memory could be allocated on the host machine"; break;
    case PICO_NOT_FOUND: return "No PicoScope could be found"; break;
    case PICO_OPEN_OPERATION_IN_PROGRESS: return "Operation in progress"; break;
    case PICO_OPERATION_FAILED: return "Operation failed"; break;
    case PICO_NOT_RESPONDING: return "PicoScope is not responding"; break;
    case PICO_CONFIG_FAIL: return "The configuration information in the PicoScope has become corrupt or is missing"; break;
    case PICO_KERNEL_DRIVER_TOO_OLD: return "The picopp.sys file is too old to be used with the device driver"; break;
    case PICO_EEPROM_CORRUPT: return "The EEPROM has become corrupt, so the device will use a default setting"; break;
    case PICO_OS_NOT_SUPPORTED: return "The operating system on the PC is not supported by this driver"; break;
    case PICO_INVALID_HANDLE: return "There is no device with the handle value passed"; break;
    case PICO_INVALID_PARAMETER: return "A parameter value is not valid"; break;
    case PICO_INVALID_TIMEBASE: return "The timebase is not supported or is invalid"; break;
    case PICO_INVALID_VOLTAGE_RANGE: return "The voltage range is not supported or is invalid"; break;
    case PICO_INVALID_CHANNEL: return "The channel number is not valid on this device or no channels have been set"; break;
    case PICO_INVALID_TRIGGER_CHANNEL: return "The channel set for a trigger is not available on this device"; break;
    case PICO_INVALID_CONDITION_CHANNEL: return "The channel set for a condition is not available on this device"; break;
    case PICO_NO_SIGNAL_GENERATOR: return "The device does not have a signal generator"; break;
    case PICO_STREAMING_FAILED: return "Streaming has failed to start or has stopped without user request"; break;
    case PICO_BLOCK_MODE_FAILED: return "Block failed to start - a parameter may have been set wrongly"; break;
    case PICO_NULL_PARAMETER: return "A parameter that was required is NULL"; break;
    case PICO_DATA_NOT_AVAILABLE: return "No data is available from a run block call"; break;
    case PICO_ETS_NOT_SUPPORTED: return "ETS is not supported on this device"; break;
    case PICO_BUFFER_STALL: return "The collection of data has stalled as unread data would be overwritten"; break;
    case PICO_TOO_MANY_SAMPLES: return "Number of samples requested is more than available in the current memory segment"; break;
    case PICO_TOO_MANY_SEGMENTS: return "Not possible to create number of segments requested"; break;
    case PICO_PULSE_WIDTH_QUALIFIER: return "A null pointer has been passed in the trigger function or one of the parameters is out of range"; break;
    case PICO_DELAY: return "One or more of the hold-off parameters are out of range"; break;
    case PICO_SOURCE_DETAILS: return "One or more of the source details are incorrect"; break;
    case PICO_CONDITIONS: return "One or more of the conditions are incorrect"; break;
    case PICO_USER_CALLBACK: return "The driver's thread is currently in the ps6000 BlockReady callback function and therefore the action cannot be carried out"; break;
    case PICO_DEVICE_SAMPLING: return "An attempt is being made to get stored data while streaming. Either stop streaming by calling ps6000Stop, or use ps6000GetStreamingLatestValues"; break;
    case PICO_NO_SAMPLES_AVAILABLE: return "No samples available because a run has not been completed"; break;
    case PICO_SEGMENT_OUT_OF_RANGE: return "The memory index is out of range"; break;
    case PICO_BUSY: return "Data cannot be returned yet"; break;
    case PICO_STARTINDEX_INVALID: return "The start time to get stored data is out of range"; break;
    case PICO_INVALID_INFO: return "The information number requested is not a valid number"; break;
    case PICO_INFO_UNAVAILABLE: return "The handle is invalid so no information is available about the device. Only PICO_DRIVER_VERSION is available"; break;
    case PICO_INVALID_SAMPLE_INTERVAL: return "The sample interval selected for streaming is out of range"; break;
    case PICO_MEMORY: return "Driver cannot allocate memory"; break;
    case PICO_SIG_GEN_PARAM: return "Incorrect parameter passed to signal generator"; break;
    case PICO_WARNING_AUX_OUTPUT_CONFLICT: return "AUX cannot be used as input and output at the same time"; break;
    case PICO_SIGGEN_OUTPUT_OVER_VOLTAGE: return "The combined peak to peak voltage and the analog offset voltage exceed the allowable voltage the signal generator can produce"; break;
    case PICO_DELAY_NULL: return "NULL pointer passed as delay parameter"; break;
    case PICO_INVALID_BUFFER: return "The buffers for overview data have not been set while streaming"; break;
    case PICO_SIGGEN_OFFSET_VOLTAGE: return "The analog offset voltage is out of range"; break;
    case PICO_SIGGEN_PK_TO_PK: return "The analog peak to peak voltage is out of range"; break;
    case PICO_CANCELLED: return "A block collection has been cancelled"; break;
    case PICO_SEGMENT_NOT_USED: return "The segment index is not currently being used"; break;
    case PICO_INVALID_CALL: return "The wrong GetValues function has been called for the collection mode in use"; break;
    case PICO_NOT_USED: return "The function is not available"; break;
    case PICO_INVALID_SAMPLERATIO: return "The aggregation ratio requested is out of range"; break;
    case PICO_INVALID_STATE: return "Device is in an invalid state"; break;
    case PICO_NOT_ENOUGH_SEGMENTS: return "The number of segments allocated is fewer than the number of captures requested"; break;
    case PICO_DRIVER_FUNCTION: return "You called a driver function while another driver function was still being processed"; break;
    case PICO_INVALID_COUPLING: return "An invalid coupling type was specified in ps6000SetChannel"; break;
    case PICO_BUFFERS_NOT_SET: return "An attempt was made to get data before a data buffer was defined"; break;
    case PICO_RATIO_MODE_NOT_SUPPORTED: return "The selected downsampling mode (used for data reduction) is not allowed"; break;
    case PICO_INVALID_TRIGGER_PROPERTY: return "An invalid parameter was passed to ps6000SetTriggerChannelProperties"; break;
    case PICO_INTERFACE_NOT_CONNECTED: return "The driver was unable to contact the oscilloscope"; break;
    case PICO_SIGGEN_WAVEFORM_SETUP_FAILED: return "A problem occurred in ps6000SetSigGenBuiltIn or ps6000SetSigGenArbitrary"; break;
    case PICO_FPGA_FAIL: return "FPGA fail"; break;
    case PICO_POWER_MANAGER: return "Power manager"; break;
    case PICO_INVALID_ANALOGUE_OFFSET: return "An impossible analogue offset value was specified in ps6000SetChannel"; break;
    case PICO_PLL_LOCK_FAILED: return "Unable to configure the PicoScope 6000. PLL lock failed"; break;
    case PICO_ANALOG_BOARD: return "The oscilloscope's analog board is not detected, or is not connected to the digital board"; break;
    case PICO_CONFIG_FAIL_AWG: return "Unable to configure the signal generator"; break;
    case PICO_INITIALISE_FPGA: return "The FPGA cannot be initialized, so unit cannot be opened"; break;
    case PICO_EXTERNAL_FREQUENCY_INVALID: return "The frequency for the external clock is not within ±5% of the stated value"; break;
    case PICO_CLOCK_CHANGE_ERROR: return "The FPGA could not lock the clock signal"; break;
    case PICO_TRIGGER_AND_EXTERNAL_CLOCK_CLASH: return "You are trying to configure the AUX input as both a trigger and a reference clock"; break;
    case PICO_PWQ_AND_EXTERNAL_CLOCK_CLASH: return "You are trying to configure the AUX input as both a pulse width qualifier and a reference clock"; break;
    case PICO_UNABLE_TO_OPEN_SCALING_FILE: return "The scaling file set can not be opened"; break;
    case PICO_MEMORY_CLOCK_FREQUENCY: return "The frequency of the memory is reporting incorrectly"; break;
    case PICO_I2C_NOT_RESPONDING: return "The I2C that is being actioned is not responding to requests"; break;
    case PICO_NO_CAPTURES_AVAILABLE: return "There are no captures available and therefore no data can be returned"; break;
    case PICO_NOT_USED_IN_THIS_CAPTURE_MODE: return "The capture mode the device is currently running in does not support the current request"; break;
    case PICO_GET_DATA_ACTIVE: return "Reserved (get data active)"; break;
    case PICO_IP_NETWORKED: return "The device is currently connected via the IP Network socket and thus the call made is not supported"; break;
    case PICO_INVALID_IP_ADDRESS: return "An IP address that is not correct has been passed to the driver"; break;
    case PICO_IPSOCKET_FAILED: return "The IP socket has failed"; break;
    case PICO_IPSOCKET_TIMEDOUT: return "The IP socket has timed out"; break;
    case PICO_SETTINGS_FAILED: return "The settings requested have failed to be set"; break;
    case PICO_NETWORK_FAILED: return "The network connection has failed"; break;
    case PICO_WS2_32_DLL_NOT_LOADED: return "Unable to load the WS2 dll"; break;
    case PICO_INVALID_IP_PORT: return "The IP port is invalid"; break;
    case PICO_COUPLING_NOT_SUPPORTED: return "The type of coupling requested is not supported on the opened device"; break;
    case PICO_BANDWIDTH_NOT_SUPPORTED: return "Bandwidth limit is not supported on the opened device"; break;
    case PICO_INVALID_BANDWIDTH: return "The value requested for the bandwidth limit is out of range"; break;
    case PICO_AWG_NOT_SUPPORTED: return "The device does not have an arbitrary waveform generator"; break;
    case PICO_ETS_NOT_RUNNING: return "Data has been requested with ETS mode set but run block has not been called, or stop has been called"; break;
    case PICO_SIG_GEN_WHITENOISE_NOT_SUPPORTED: return "White noise is not supported on the opened device"; break;
    case PICO_SIG_GEN_WAVETYPE_NOT_SUPPORTED: return "The wave type requested is not supported by the opened device"; break;
    case PICO_SIG_GEN_PRBS_NOT_SUPPORTED: return "Siggen does not generate pseudorandom bit stream"; break;
    case PICO_ETS_NOT_AVAILABLE_WITH_LOGIC_CHANNELS: return "When a digital port is enabled, ETS sample mode is not available for use"; break;
    case PICO_WARNING_REPEAT_VALUE: return "Not applicable to this device"; break;
    case PICO_POWER_SUPPLY_CONNECTED: return "The DC power supply is connected"; break;
    case PICO_POWER_SUPPLY_NOT_CONNECTED: return "The DC power supply isn’t connected"; break;
    case PICO_POWER_SUPPLY_REQUEST_INVALID: return "Incorrect power mode passed for current power source"; break;
    case PICO_POWER_SUPPLY_UNDERVOLTAGE: return "The supply voltage from the USB source is too low"; break;
    case PICO_CAPTURING_DATA: return "The device is currently busy capturing data"; break;
    case PICO_NOT_SUPPORTED_BY_THIS_DEVICE: return "A function has been called that is not supported by the current device variant"; break;
    case PICO_INVALID_DEVICE_RESOLUTION: return "The device resolution is invalid (out of range)"; break;
    case PICO_INVALID_NUMBER_CHANNELS_FOR_RESOLUTION: return "The number of channels which can be enabled is limited in 15 and 16-bit modes"; break;
    case PICO_CHANNEL_DISABLED_DUE_TO_USB_POWERED: return "USB Power not sufficient to power all channels"; break;
    }
    return "Not defined: " + status;
}



/* for testing purpose */
#ifdef PICOSCOPE_TEST_MODE

void PicoScope::streamingTest(bool start)
{
    streamingTestShouldStop = !start;

    if(start)
    {
        isStreaming(true);
        int circularBufferSize = 50;

        streamingTestShouldStop = !start;

        streaming_samples = 1000;

        circularBufferChannelA.set_capacity(settings->captures());
        circularBufferChannelB.set_capacity(settings->captures());
        circularBufferChannelC.set_capacity(settings->captures());
        circularBufferChannelD.set_capacity(settings->captures());
        circularBufferChannelA.clear();
        circularBufferChannelB.clear();
        circularBufferChannelC.clear();
        circularBufferChannelD.clear();

        QFuture<void> future = QtConcurrent::run(this, &PicoScope::streamingTestWorker);
    }
    else
        isStreaming(false);
}

double PicoScope::getRangeFactorStreamingTest(PSChannel channel)
{
    switch(settings->range(channel))
    {
    case PSRange::R50mV: return 0.05; break;
    case PSRange::R100mV: return 0.1; break;
    case PSRange::R200mV: return 0.2; break;
    case PSRange::R500mV: return 0.5; break;
    case PSRange::R1V: return 1.0; break;
    case PSRange::R2V: return 2.0; break;
    case PSRange::R5V: return 5.0; break;
    case PSRange::R10V: return 10.0; break;
    case PSRange::R20V: return 20.0; break;
    }
}

void PicoScope::streamingTestWorker()
{
    int overflowCounter = 0;
    bool overflow = false;

    while(!streamingTestShouldStop)
    {
        overflowCounter++;
        if(overflowCounter > 50)
        {
            overflow = !overflow;
            overflowCounter = 0;
        }
        QThread::msleep(50);

        if(settings->getAveragingBufferSize() != circularBufferChannelA.capacity())
        {
            circularBufferChannelA.set_capacity(settings->getAveragingBufferSize());
            circularBufferChannelB.set_capacity(settings->getAveragingBufferSize());
            circularBufferChannelC.set_capacity(settings->getAveragingBufferSize());
            circularBufferChannelD.set_capacity(settings->getAveragingBufferSize());
            //clearCircularBuffer = true;
        }
        if(clearCircularBuffer)
        {
            //circularBufferChannelA.set_capacity(settings->getAveragingBufferSize());
            circularBufferChannelA.clear();
            //circularBufferChannelB.set_capacity(settings->getAveragingBufferSize());
            circularBufferChannelB.clear();
            //circularBufferChannelC.set_capacity(settings->getAveragingBufferSize());
            circularBufferChannelC.clear();
            //circularBufferChannelD.set_capacity(settings->getAveragingBufferSize());
            circularBufferChannelD.clear();

            clearCircularBuffer = false;
        }

        streamSignalA.data.clear();
        streamSignalA.dt = 0.0000001;
        streamSignalA.start_time = 0;

        for(int i = 0; i < 1000; i++)
        {
            streamSignalA.data.append((sin(0.01*(double)(i + 0)) + ((double)(rand() % 100) - 50) / 300) * getRangeFactorStreamingTest(PSChannel::A));
        }

        circularBufferChannelA.push_back(streamSignalA);

        streamSignalB.data.clear();
        streamSignalB.dt = 0.0000001;
        streamSignalB.start_time = 0;

        for(int i = 0; i < 1000; i++)
        {
            streamSignalB.data.append((sin(0.01*(double)(i + 100)) + ((double)(rand() % 100) - 50) / 400) * getRangeFactorStreamingTest(PSChannel::B));
        }

        circularBufferChannelB.push_back(streamSignalB);

        streamSignalC.data.clear();
        streamSignalC.dt = 0.0000001;
        streamSignalC.start_time = 0;

        for(int i = 0; i < 1000; i++)
        {
            streamSignalC.data.append((sin(0.01*(double)(i + 200)) + ((double)(rand() % 100) - 50) / 200) * getRangeFactorStreamingTest(PSChannel::C));
        }

        circularBufferChannelC.push_back(streamSignalC);

        streamSignalD.data.clear();
        streamSignalD.dt = 0.0000001;
        streamSignalD.start_time = 0;

        for(int i = 0; i < 1000; i++)
        {
            streamSignalD.data.append((sin(0.01*(double)(i + 300)) + ((double)(rand() % 100) - 50) / 500) * getRangeFactorStreamingTest(PSChannel::D));
        }

        circularBufferChannelD.push_back(streamSignalD);

        calculateAverage();

        switch(settings->streaming_mode)
        {
        case PSStreamingMode::SingleMeasurement:
        {
            StreamPoint point;

            point.overflowA = overflow;
            point.overflowB = overflow;
            point.overflowC = overflow;
            point.overflowD = overflow;

            point.averageBufferFilling = circularBufferChannelA.size();

            if(settings->channel(PSChannel::A))
                point.single.insert(1, streamSignalA);
            if(settings->channel(PSChannel::B))
                point.single.insert(2, streamSignalB);
            if(settings->channel(PSChannel::C))
                point.single.insert(3, streamSignalC);
            if(settings->channel(PSChannel::D))
                point.single.insert(4, streamSignalD);

            emit streamingData(point);
        }
        break;

        case PSStreamingMode::AverageMeasurement:
        {
            StreamPoint point;

            point.overflowA = overflow;
            point.overflowB = overflow;
            point.overflowC = overflow;
            point.overflowD = overflow;

            point.averageBufferFilling = circularBufferChannelA.size();

            if(settings->channel(PSChannel::A))
                point.average.insert(1, averageStreamSignalA);
            if(settings->channel(PSChannel::B))
                point.average.insert(2, averageStreamSignalB);
            if(settings->channel(PSChannel::C))
                point.average.insert(3, averageStreamSignalC);
            if(settings->channel(PSChannel::D))
                point.average.insert(4, averageStreamSignalD);

            emit streamingData(point);
        }
        break;

        case PSStreamingMode::Both:
        {
            StreamPoint point;

            point.overflowA = overflow;
            point.overflowB = overflow;
            point.overflowC = overflow;
            point.overflowD = overflow;

            point.averageBufferFilling = circularBufferChannelA.size();

            if(settings->channel(PSChannel::A))
                point.single.insert(1, streamSignalA);
            if(settings->channel(PSChannel::B))
                point.single.insert(2, streamSignalB);
            if(settings->channel(PSChannel::C))
                point.single.insert(3, streamSignalC);
            if(settings->channel(PSChannel::D))
                point.single.insert(4, streamSignalD);

            if(settings->channel(PSChannel::A))
                point.average.insert(1, averageStreamSignalA);
            if(settings->channel(PSChannel::B))
                point.average.insert(2, averageStreamSignalB);
            if(settings->channel(PSChannel::C))
                point.average.insert(3, averageStreamSignalC);
            if(settings->channel(PSChannel::D))
                point.average.insert(4, averageStreamSignalD);

            emit streamingData(point);
        }
        break;
        }
    }
}

#endif


AverageCalculator::AverageCalculator()
{
    newAverage = false;
    stdevCalculation = false;
}


void AverageCalculator::setBufferData(const boost::circular_buffer<Signal> buffer)
{
    this->buffer = buffer;
}


bool AverageCalculator::newAverageAvailable()
{
    return newAverage;
}


Signal AverageCalculator::getAverage()
{
    newAverage = false;
    return output;
}


void AverageCalculator::calcStdevNext()
{
    stdevCalculation = true;
}


void AverageCalculator::run()
{
    output.dt = buffer.at(0).dt;
    output.start_time = buffer.at(0).start_time;

    QVector<double> avgSignal;
    QVector<double> stdevSignal;
    for(int i = 0; i < buffer.at(0).data.size(); i++)
    {
        double value = 0;

        for(int j = 0; j < buffer.size(); j++)
        {
            value += buffer.at(j).data.at(i) / buffer.size();
        }

        avgSignal.push_back(value);

        if(stdevCalculation)
        {
            StandardDeviation stdev;

            for(int j = 0; j < buffer.size(); j++)
                stdev.addValue(buffer.at(j).data.at(i));

            stdevSignal.push_back(stdev.getStandardDeviation());
        }
    }

    output.data = avgSignal;
    if(stdevCalculation)
    {
        output.stdev = stdevSignal;
        stdevCalculation = false;
    }
    newAverage = true;
}


