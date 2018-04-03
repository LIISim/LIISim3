#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QThread>
#include <QList>
#include <QStringList>

extern "C"
{
    #include "../externalLibraries/nidaqmx/include/NIDAQmx.h"
}

#include "daqdevice.h"
#include "daqio.h"

class DeviceManager;

class DiscoveryWorker : public QThread
{
    Q_OBJECT
public:
    DiscoveryWorker(DeviceManager *parent);
    void run() Q_DECL_OVERRIDE;
};

class AnalogOutputWorker : public QThread
{
    Q_OBJECT
public:
    AnalogOutputWorker(DeviceManager *parent);
    void run() Q_DECL_OVERRIDE;
    bool threadShouldStop;
};

class AnalogInputWorker : public QThread
{
    Q_OBJECT
public:
    AnalogInputWorker(DeviceManager *parent);
    void run() Q_DECL_OVERRIDE;
    bool threadShouldStop;
};

class DigitalOutputWorker : public QThread
{
    Q_OBJECT
public:
    DigitalOutputWorker(DeviceManager *parent);
    void run() Q_DECL_OVERRIDE;
    bool threadShouldStop;
};

class LaserAnalogInputWorker : public QThread
{
    Q_OBJECT
public:
    LaserAnalogInputWorker(DeviceManager *parent);
    void run() Q_DECL_OVERRIDE;
    bool threadShouldStop;
};

class LaserAnalogOutputWorker : public QThread
{
    Q_OBJECT
public:
    LaserAnalogOutputWorker(DeviceManager *parent);
    void run() Q_DECL_OVERRIDE;
    bool threadShouldStop;
};

class DeviceManager : public QObject
{
    Q_OBJECT

friend class DiscoverWorker;
friend class DiscoveryWorker;
friend class AnalogOutputWorker;
friend class AnalogInputWorker;
friend class DigitalOutputWorker;
friend class LaserAnalogInputWorker;
friend class LaserAnalogOutputWorker;

public:
    DeviceManager(QObject *parent = 0);
    ~DeviceManager();

    bool isDiscovering();

    QList<DAQDevice> devices();

    void handleStartup();

    void setAverageAnalogIn(bool enabled);
    bool getAverageAnalogIn();

    void setOutputVoltage(unsigned int internalHandle, double voltage);
    void setOutputChannel(unsigned int internalHandle, QString channel, float voltage);
    QString getAnalogOutChannelIdentifier(unsigned int internalHandle);

    void setAnalogOutLimit(double min, double max);
    double getAnalogOutLimitMax();
    double getAnalogOutLimitMin();

    void setInputChannel(unsigned int internalHandle, QString channel);
    QString getAnalogInChannelIdentifier(unsigned int internalHandle);

    bool getAnalogInputIsValid();
    double getAnalogInputValue(unsigned int internalHandle);
    double getAnalogInputAverageValue(unsigned int internalHandle);

    void setDigitalOutputChannel(unsigned int internalHandle, QString channel, bool inverted);
    void setDigitalOutputEnabled(unsigned int internalHandle, bool enabled);
    QString getDigitalOutputChannel(unsigned int internalHandle);
    bool getDigitalOutputChannelEnabled(unsigned int internalHandle);
    bool getDigitalOutputInverted(unsigned int internalHandle);


    void setLaserOutputVoltage(double voltage);
    void setLaserOutputChannel(QString channel, double voltage);

    void setLaserInputChannel(QString channel);

    bool getLaserAnalogOutRunning();
    bool getLaserAnalogInRunning();

    void setAnalogInAverageSampleSize(unsigned int samples);
    unsigned int getAnalogInAverageSampleSize();


private:
    DiscoveryWorker discoveryWorker;
    QList<DAQDevice> devlist;

    AnalogOutputWorker analogOutputWorker;
    AnalogInputWorker analogInputWorker;
    DigitalOutputWorker digitalOutputWorker;

    LaserAnalogInputWorker analogInputWorkerLaser;
    LaserAnalogOutputWorker analogOutputWorkerLaser;
    DAQAnalogIn laserAnalogIn;
    DAQAnalogOut laserAnalogOut;

    bool averageAnalogIn;

    void setAnalogOutputEnabled(bool enabled);
    bool analogOutputEnabled;

    void setAnalogInputEnabled(bool enabled);
    bool analogInputEnabled;

    void setDigitalOutputEnabled(bool enabled);
    bool digitalOutputEnabled;

    void setLaserAnalogOutputEnabled(bool enabled);
    bool laserAnalogOutputEnabled;

    void setLaserAnalogInputEnabled(bool enabled);
    bool laserAnalogInputEnabled;

    QList<DAQAnalogOut> analogOut;
    QList<DAQAnalogIn> analogIn;
    QList<DAQDigitalOut> digitalOut;

    float64 AOLimitMin;
    float64 AOLimitMax;

    unsigned int analogInSampleBufferSize;

    void setOutputChannel(unsigned int internalHandle, QString channel, float voltage, bool suppressSaving);

    void saveAnalogOutputChannel();
    void loadAnalogOutputChannel();

    void setInputChannel(unsigned int internalHandle, QString channel, bool suppressSaving);

    void saveAnalogInputChannel();
    void loadAnalogInputChannel();

    void setDigitalOutputChannel(unsigned int internalHandle, QString channel, bool inverted, bool suppressSaving);

    void saveDigitalOutputChannel();
    void loadDigitalOutputChannel();

    QString DAQmxErrorAsString(int error);

    QString getDeviceTypeFromChannel(QString channel);

signals:
    void listUpdated();
    void analogOutputLimitChanged();
    void analogInputValue(int channel, float value); //---
    void analogInputReset();
    void analogInputStateChanged(bool enabled);
    void analogOutputStateChanged(bool enabled);
    void digitalOutputStateChanged(bool enabled);
    void digitalOutputChanged();

    void laserAnalogOutStateChanged(bool enabled);
    void laserAnalogInStateChanged(bool enabled);
    void laserAnalogValue(double value);

private slots:
    void onDiscoveryWorkerFinished();

public slots:
    void discover();

    void startAnalogOUT();
    void stopAnalogOUT();

    void startAnalogIN();
    void stopAnalogIN();

    void startDigitalOUT();
    void stopDigitalOUT();

    void onSystemSuspending();

    void startLaserAnalogOUT();
    void stopLaserAnalogOUT();

    void startLaserAnalogIN();
    void stopLaserAnalogIN();

};

#endif // DEVICEMANAGER_H
