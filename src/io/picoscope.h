#ifndef PICOSCOPE_H
#define PICOSCOPE_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QSemaphore>

#include "../settings/picoscopesettings.h"
#include "../signal/signal.h"
#include "../signal/mrun.h"
#include "../signal/streampoint.h"

#include "../externalLibraries/picoscope6000/include/ps6000Api.h"
#include <externalLibraries/boost/boost/circular_buffer.hpp>

//#define  PICOSCOPE_TEST_MODE

class PicoScope;

class AverageCalculator : public QThread
{
    Q_OBJECT
public:
    AverageCalculator();
    void run() Q_DECL_OVERRIDE;
    void setBufferData(const boost::circular_buffer<Signal> buffer);
    bool newAverageAvailable();
    Signal getAverage();
    void calcStdevNext();

private:
    boost::circular_buffer<Signal> buffer;
    Signal output;
    bool newAverage;
    bool stdevCalculation;
};

class PicoScope: public QObject
{
    Q_OBJECT

public:

    PicoScope(PicoScopeSettings *settings, QObject *parent = 0);
    ~PicoScope();

    bool open();
    void close();

    bool run();
    void getRun(MRun *run, bool averageCaptures, bool calculateStdev);

    bool isOpen();
    bool isStreaming();
    bool isTriggered();
    bool isBlockrun();
    bool isProcessing();
    bool isError();

    void startStreaming();
    void stopStreaming();

    const QString getLastError();

    void offsetBounds(PSChannel channel, float *max, float *min);

    Signal getAverageStreamSignal(PSChannel channel);

    StreamPoint getLastAverage();

    void getStreamBufferContent(MRun *run, bool asAverage);

    /* for testing purpose */
    void streamingTest(bool start = true);

    /* for testing purpose */
#ifdef PICOSCOPE_TEST_MODE
    void streamingTestWorker();
    bool streamingTestShouldStop;
    double getRangeFactorStreamingTest(PSChannel channel);
    int streaming_samples;
#endif

private:
    bool internalOpen();

    bool setup(bool blockmode);

    //bool setupBlock();
    //bool setupStreaming();

    void isOpen(bool driverOpen);
    void isStreaming(bool streaming);
    void isTriggered(bool triggered);
    void isBlockrun(bool blockrun);
    void isProcessing(bool processing);
    void isError(bool error);
    void setLastError(QString error);

#ifdef __MINGW32__
    __stdcall static void callbackBlockReady(int16_t handle, PICO_STATUS status, void *pParameter);
#else
 #ifdef _MSC_VER
    static void callbackBlockReady(int16_t handle, PICO_STATUS status, void *pParameter);
 #else
  #error UNRECOGNIZED COMPILER
 #endif
#endif
    void handleBlockReady(PICO_STATUS status);

#ifdef __MINGW32__
    __stdcall static void callbackStreamingReady(int16_t handle, PICO_STATUS status, void *pParameter);
#else
 #ifdef _MSC_VER
    static void callbackStreamingReady(int16_t handle, PICO_STATUS status, void *pParameter);
#else
 #error UNRECOGNIZED COMPILER
#endif
#endif

    void handleStreamingReady(PICO_STATUS status);

    QString picoStatusAsString(PICO_STATUS status);

    PicoScopeSettings *settings;

    int16_t handle;         /* PicoScope handle */
    bool driverOpen;
    bool streaming;
    bool blockrun;
    bool processing;
    bool streamingDataProcessed;
    bool triggered;
    bool error;

    bool clearCircularBuffer = false;

    // see also bool PicoScope::resizeBuffer(uint32_t size)
    int16_t **bufferA;
    int16_t **bufferB;
    int16_t **bufferC;
    int16_t **bufferD;
    uint32_t bufferSize;
    uint32_t bufferCaptures;
    bool resizeBuffer(uint32_t size, uint32_t captures);

    QString lastError;

    void streamingWorker();
    void blockWorker(MRun *run, bool averageCaptures, bool calculateStdev);

    double getRangeFactor(PSRange range);
    PS6000_COUPLING getCoupling(PSCoupling coupling);
    PS6000_RANGE getRange(PSRange range);

    void calculateAverage();

    AverageCalculator *calculationThreads[4];

    boost::circular_buffer<Signal> circularBufferChannelA;
    boost::circular_buffer<Signal> circularBufferChannelB;
    boost::circular_buffer<Signal> circularBufferChannelC;
    boost::circular_buffer<Signal> circularBufferChannelD;

    Signal streamSignalA;
    Signal streamSignalB;
    Signal streamSignalC;
    Signal streamSignalD;

    Signal averageStreamSignalA;
    Signal averageStreamSignalB;
    Signal averageStreamSignalC;
    Signal averageStreamSignalD;

    QMutex averageLock;

    bool restartStreaming;

    QMutex picoscopeAccess;
    QMutex singleStreamingRunFinished;

    QSemaphore streamingLock;

    int lastOverflowCounter[4];
    bool lastOverflow[4];

signals:
    void runReady();
    //void streamingDataReady(Signal *a, Signal *b, Signal *c, Signal *d);
    void streamingBlockReady(MPoint *point);
    void processingProgress(float percent);
    void processingFinished(MRun *run);

    void errorMsg(QString message);
    void infoMsg(QString message);

    void statusOpenChanged(bool open);
    void statusStreamingChanged(bool streaming);
    void statusTriggeredChanged(bool triggered);
    void statusBlockrunChanged(bool blockrun);
    void statusProcessingChanged(bool processing);
    void statusErrorChanged(bool error);

    void streamingData(Signal channelA, Signal channelB, Signal channelC, Signal channelD, bool average);
    void streamingData(Signal channelA, Signal channelB, Signal channelC, Signal channelD,
                       Signal averageA, Signal averageB, Signal averageC, Signal averageD);

    void streamingData(StreamPoint point);

    void updateSignalCounter(int number);

public slots:
    void clearStreamingAvg();

private slots:
    void updateSettings();
};

#endif // PICOSCOPE_H
