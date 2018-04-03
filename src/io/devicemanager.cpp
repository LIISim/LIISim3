#include "devicemanager.h"

#include "QtConcurrent/QtConcurrent"
#include "core.h"
#include <QMutexLocker>

//--- DiscoveryWorker

DiscoveryWorker::DiscoveryWorker(DeviceManager *parent) : QThread(static_cast<QObject*>(parent))
{

}


/**
 * @brief DiscoveryWorker::run Thread which manages the DAQmx device discovery and updates
 *        the device list in the DeviceManager class with the received information
 */
void DiscoveryWorker::run()
{
    DeviceManager *dm = static_cast<DeviceManager*>(parent());

    uInt32 major;
    uInt32 minor;
    uInt32 update;
    DAQmxGetSysNIDAQMajorVersion(&major);
    DAQmxGetSysNIDAQMinorVersion(&minor);
    DAQmxGetSysNIDAQUpdateVersion(&update);

    MSG_INFO(QString("DeviceManager: Using NIDAQmx version %0.%1.%2").arg(major).arg(minor).arg(update));
    MSG_INFO("DeviceManager: Starting discovery");

    int ret;
    char data[2048];

    ret = DAQmxGetSysDevNames(&data[0], 2048);
    if(ret != 0)
        MSG_ERR(QString("DeviceManager: DAQmxGetSysDevNames returned ").append(dm->DAQmxErrorAsString(ret)));
    else
    {
        QStringList devices = QString(data).split(',', QString::SkipEmptyParts);

        QList<DAQDevice> *devlist = &dm->devlist;
        devlist->clear();

        for(int i = 0; i < devices.size(); i++)
        {
            uInt32 tempInt;
            //add the device to the device list
            devlist->push_back(DAQDevice(devices.at(i).trimmed()));
            //get the device product type
            ret = DAQmxGetDevProductType(devlist->back().identifier.toStdString().data(), &data[0], 2048);
            if(ret != 0)
                MSG_ERR(QString("DeviceManager: DAQmxGetDevProductType returned ").append(dm->DAQmxErrorAsString(ret)));
            else
                devlist->back().type = QString(data);
            //get the serial number of the device
            ret = DAQmxGetDevSerialNum(devlist->back().identifier.toStdString().data(), &tempInt);
            if(ret != 0)
                MSG_ERR(QString("DeviceManager: DAQmxGetDevSerialNum returned ").append(dm->DAQmxErrorAsString(ret)));
            else
                devlist->back().serialNumber = tempInt;

            bool32 simulated;
            //get if the device is simulated
            ret = DAQmxGetDevIsSimulated(devlist->back().identifier.toStdString().data(), &simulated);
            if(ret != 0)
                MSG_ERR(QString("DeviceManager: DAQmxGetDevIsSimulated returned ").append(dm->DAQmxErrorAsString(ret)));
            else
                devlist->back().isSimulated = simulated;
            //get the analog inputs for the last added device
            ret = DAQmxGetDevAIPhysicalChans(devlist->back().identifier.toStdString().data(), &data[0], 2048);
            if(ret != 0)
                MSG_ERR(QString("DeviceManager: DAQmxGetDevAIPhysicalChans returned ").append(dm->DAQmxErrorAsString(ret)));
            else
            {
                QStringList analogInputs = QString(data).split(',', QString::SkipEmptyParts);
                for(int i = 0; i < analogInputs.size(); i++)
                    devlist->back().analogIn.insert(i, analogInputs.at(i).trimmed());
            }
            //get the analog outputs for the last added device
            ret = DAQmxGetDevAOPhysicalChans(devlist->back().identifier.toStdString().data(), &data[0], 2048);
            if(ret != 0)
                MSG_ERR(QString("DeviceManager: DAQmxGetDevAOPhysicalChans returned ").append(dm->DAQmxErrorAsString(ret)));
            else
            {
                QStringList analogOutputs = QString(data).split(',', QString::SkipEmptyParts);
                for(int i = 0; i < analogOutputs.size(); i++)
                    devlist->back().analogOut.insert(i, analogOutputs.at(i).trimmed());
            }
            //get the digital inputs for the last added device
            ret = DAQmxGetDevDILines(devlist->back().identifier.toStdString().data(), &data[0], 2048);
            if(ret != 0)
                MSG_ERR(QString("DeviceManager: DAQmxGetDevDILines returned ").append(dm->DAQmxErrorAsString(ret)));
            else
            {
                QStringList digitalInputs = QString(data).split(',', QString::SkipEmptyParts);
                for(int i = 0; i < digitalInputs.size(); i++)
                    devlist->back().digitalIn.insert(i, digitalInputs.at(i).trimmed());
            }
            //get the digital outputs for the last added device
            ret = DAQmxGetDevDOLines(devlist->back().identifier.toStdString().data(), &data[0], 2048);
            if(ret != 0)
                MSG_ERR(QString("DeviceManager: DAQmxGetDevDOLines returned ").append(dm->DAQmxErrorAsString(ret)));
            else
            {
                QStringList digitalOutputs = QString(data).split(',', QString::SkipEmptyParts);
                for(int i = 0; i < digitalOutputs.size(); i++)
                    devlist->back().digitalOut.insert(i, digitalOutputs.at(i).trimmed());
            }
        }
    }
    QThread::exit();
}


//--- AnalogOutputWorker

AnalogOutputWorker::AnalogOutputWorker(DeviceManager *parent) : QThread(static_cast<QObject*>(parent))
{
    threadShouldStop = false;
}


/**
 * @brief AnalogOutputWorker::run Thread wich writes the initial analog output value and when
 *        the value is changed. Checks every 100ms for a new value.
 */
void AnalogOutputWorker::run()
{
    MSG_DEBUG("AnalogOutputWorker::run() started");
    while(!threadShouldStop)
    {
        int ret;
        DeviceManager *dm = static_cast<DeviceManager*>(parent());

        for(int i = 0; i < dm->analogOut.size(); i++)
        {
            QMutexLocker locker(dm->analogOut[i].lock);

            if(dm->analogOut[i].voltageChanged)
            {
                //qDebug() << "writing output voltage for channel" << dm->analogOut[i].channel << "to" << dm->analogOut[i].out;

                ret = DAQmxWriteAnalogScalarF64(dm->analogOut[i].taskHandle, true, 5.0, dm->analogOut[i].out, NULL);
                if(ret != 0)
                {
                    if(ret > 0)
                        MSG_WARN(QString("DAQmxWriteAnalogScalarF64: ").append(dm->DAQmxErrorAsString(ret)).append(" for %0").arg(i));
                    else
                        MSG_ERR(QString("DAQmxWriteAnalogScalarF64: ").append(dm->DAQmxErrorAsString(ret)).append(" for %0").arg(i));
                }
                dm->analogOut[i].voltageChanged = false;
            }
        }
        QThread::msleep(100);
    }
    MSG_DEBUG("AnalogOutputWorker::run() stopped");
    QThread::exit();
}


//--- AnalogInputWorker

AnalogInputWorker::AnalogInputWorker(DeviceManager *parent): QThread(static_cast<QObject*>(parent))
{
    threadShouldStop = false;
}


/**
 * @brief AnalogInputWorker::run Thread which reads the analog input values every 100ms
 *        and calculates the average.
 */
void AnalogInputWorker::run()
{
    MSG_DEBUG("AnalogInputWorker::run() started");
    while(!threadShouldStop)
    {
        int ret;
        DeviceManager *dm = static_cast<DeviceManager*>(parent());

        for(int i = 0; i < dm->analogIn.size(); i++)
        {
            float64 value = 0.0;
            float64 average = 0.0;
            ret = DAQmxReadAnalogScalarF64(dm->analogIn[i].taskHandle, 5.0, &value, NULL);
            if(ret != 0)
            {
                if(ret > 0)
                    MSG_WARN(QString("DAQmxReadAnalogScalarF64: ").append(dm->DAQmxErrorAsString(ret)).append(" for %0").arg(i));
                else
                    MSG_ERR(QString("DAQmxReadAnalogScalarF64: ").append(dm->DAQmxErrorAsString(ret)).append(" for %0").arg(i));
            }
            dm->analogIn[i].buffer.push_back(value);
            for(int j = 0; j < dm->analogIn[i].buffer.size(); j++)
            {
                average += dm->analogIn[i].buffer.at(j) / dm->analogIn[i].buffer.size();
            }
            dm->analogIn[i].lastValue = value;
            dm->analogIn[i].lastAverage = average;
            if(dm->averageAnalogIn)
                emit dm->analogInputValue(dm->analogIn[i].internalHandle, average);
            else
                emit dm->analogInputValue(dm->analogIn[i].internalHandle, value);
        }
        QThread::msleep(100);
    }
    MSG_DEBUG("AnalogInputWorker::run() stopped");
    QThread::exit();
}


//--- DigitalOutputWorker

DigitalOutputWorker::DigitalOutputWorker(DeviceManager *parent): QThread(static_cast<QObject*>(parent))
{
    threadShouldStop = false;
}


/**
 * @brief DigitalOutputWorker::run Digital output thread which writes the initial digital
 *        output and also every time the value for a channel is changed. Checks every
 *        100ms for a changed value.
 */
void DigitalOutputWorker::run()
{
    MSG_DEBUG("DigitalOutputWorker::run() started");

    DeviceManager *dm = static_cast<DeviceManager*>(parent());
    int ret;

    while(!threadShouldStop)
    {


        for(int i = 0; i < dm->digitalOut.size(); i++)
        {
            if(dm->digitalOut[i].changed)
            {
                ret = DAQmxWriteDigitalU32(dm->digitalOut[i].taskHandle, 1, true, 5.0, DAQmx_Val_GroupByChannel, dm->digitalOut[i].out, NULL, NULL);
                if(ret != 0)
                {
                    if(ret > 0)
                        MSG_WARN(QString("DAQmxWriteDigitalU32: ").append(dm->DAQmxErrorAsString(ret)).append(" for %0 %1").arg(dm->digitalOut[i].dev).arg(dm->digitalOut[i].port));
                    else
                        MSG_ERR(QString("DAQmxWriteDigitalU32: ").append(dm->DAQmxErrorAsString(ret)).append(" for %0 %1").arg(dm->digitalOut[i].dev).arg(dm->digitalOut[i].port));
                }
                dm->digitalOut[i].changed = false;
                //qDebug() << "Changed digital output" << dm->digitalOut[i].dev << dm->digitalOut[i].port << "to" << dm->digitalOut[i].out[0];
            }
        }
        QThread::msleep(100);
    }

    MSG_DEBUG("DigitalOutputWorker: setting all digital outs to 0");

    uInt32 out[1];

    for(int i = 0; i < dm->digitalOut.size(); i++)
    {
        out[0] = 0;

        for(int j = 0; j < dm->digitalOut[i].list.size(); j++)
        {
            if(dm->digitalOut[i].list[j].inverted)
                out[0] |= (1 << dm->digitalOut[i].list[j].channelNumber);
        }

        ret = DAQmxWriteDigitalU32(dm->digitalOut[i].taskHandle, 1, true, 5.0, DAQmx_Val_GroupByChannel, out, NULL, NULL);
        if(ret != 0)
        {
            if(ret > 0)
                MSG_WARN(QString("DAQmxWriteDigitalU32: ").append(dm->DAQmxErrorAsString(ret)).append(" for %0 %1").arg(dm->digitalOut[i].dev).arg(dm->digitalOut[i].port));
            else
                MSG_ERR(QString("DAQmxWriteDigitalU32: ").append(dm->DAQmxErrorAsString(ret)).append(" for %0 %1").arg(dm->digitalOut[i].dev).arg(dm->digitalOut[i].port));
        }
    }

    MSG_DEBUG("DigitalOutputWorker::run() stopped");
    QThread::exit();
}


//--- LaserAnalogInputWorker

LaserAnalogInputWorker::LaserAnalogInputWorker(DeviceManager *parent) : QThread(static_cast<QObject*>(parent))
{
    threadShouldStop = false;
}


void LaserAnalogInputWorker::run()
{
    MSG_DEBUG("LaserAnalogInputWorker::run()");

    DeviceManager *dm = static_cast<DeviceManager*>(parent());

    emit dm->laserAnalogInStateChanged(true);

    int ret;

    while(!threadShouldStop)
    {
        float64 value = 0.0;
        float64 average = 0.0;
        ret = DAQmxReadAnalogScalarF64(dm->laserAnalogIn.taskHandle, 5.0, &value, NULL);
        if(ret != 0)
        {
            if(ret > 0)
                MSG_WARN(QString("DAQmxReadAnalogScalarF64: ").append(dm->DAQmxErrorAsString(ret)));
            else
                MSG_ERR(QString("DAQmxReadAnalogScalarF64: ").append(dm->DAQmxErrorAsString(ret)));
        }
        dm->laserAnalogIn.buffer.push_back(value);
        for(int j = 0; j < dm->laserAnalogIn.buffer.size(); j++)
        {
            average += dm->laserAnalogIn.buffer.at(j) / dm->laserAnalogIn.buffer.size();
        }
        dm->laserAnalogIn.lastValue = value;
        dm->laserAnalogIn.lastAverage = average;
        if(dm->averageAnalogIn) //TODO:
            emit dm->laserAnalogValue(average);
        else
            emit dm->laserAnalogValue(value);

        QThread::msleep(100);
    }

    emit dm->laserAnalogInStateChanged(false);
    MSG_DEBUG("LaserAnalogInputWorker::run() stopped");
    QThread::exit();
}


//--- LaserAnalogOutputWorker

LaserAnalogOutputWorker::LaserAnalogOutputWorker(DeviceManager *parent) : QThread(static_cast<QObject*>(parent))
{
    threadShouldStop = false;
}


void LaserAnalogOutputWorker::run()
{
    MSG_DEBUG("LaserAnalogOutputWorker::run() started");

    int ret;

    DeviceManager *dm = static_cast<DeviceManager*>(parent());

    emit dm->laserAnalogOutStateChanged(true);

    while(!threadShouldStop)
    {
        QMutexLocker locker(dm->laserAnalogOut.lock);

        if(dm->laserAnalogOut.voltageChanged)
        {
            //qDebug() << "writing output voltage for channel" << dm->analogOut[i].channel << "to" << dm->analogOut[i].out;

            ret = DAQmxWriteAnalogScalarF64(dm->laserAnalogOut.taskHandle, true, 5.0, dm->laserAnalogOut.out, NULL);
            if(ret != 0)
            {
                if(ret > 0)
                    MSG_WARN(QString("DAQmxWriteAnalogScalarF64: ").append(dm->DAQmxErrorAsString(ret)));
                else
                    MSG_ERR(QString("DAQmxWriteAnalogScalarF64: ").append(dm->DAQmxErrorAsString(ret)));
            }

            qDebug() << "DeviceManager: laser analog output changed to" << dm->laserAnalogOut.out;

            dm->laserAnalogOut.voltageChanged = false;
        }
        QThread::msleep(100);
    }
    MSG_DEBUG("LaserAnalogOutputWorker::run() stopped");

    emit dm->laserAnalogOutStateChanged(false);

    QThread::exit();
}


//--- DeviceManager

DeviceManager::DeviceManager(QObject *parent) : QObject(parent), discoveryWorker(this), analogOutputWorker(this), analogInputWorker(this), digitalOutputWorker(this), analogInputWorkerLaser(this), analogOutputWorkerLaser(this)
{
    analogOutputEnabled = false;
    analogInputEnabled = false;
    digitalOutputEnabled = false;
    laserAnalogInputEnabled = false;
    laserAnalogOutputEnabled = false;

    averageAnalogIn = false;
    analogInSampleBufferSize = 1;

    AOLimitMin = 0.3;
    AOLimitMax = 0.7;

    connect(&discoveryWorker, SIGNAL(finished()), SLOT(onDiscoveryWorkerFinished()));
}


DeviceManager::~DeviceManager()
{
    //stop running io threads
    stopAnalogOUT();
    stopAnalogIN();
    stopDigitalOUT();

    stopLaserAnalogOUT();
    stopLaserAnalogIN();
    //stop maybe running discovery
    disconnect(&discoveryWorker, SIGNAL(finished()), this, SLOT(onDiscoveryWorkerFinished()));
    discoveryWorker.terminate();
}


/**
 * @brief DeviceManager::isDiscovering Returns if the discovery worker is running.
 * @return True if the discovery worker is running, false otherwise
 */
bool DeviceManager::isDiscovering()
{
    return discoveryWorker.isRunning();
}


/**
 * @brief DeviceManager::onDiscoveryWorkerFinished Loads the previously saved channels
 *        and emits a signal that the device list is updated
 */
void DeviceManager::onDiscoveryWorkerFinished()
{
    MSG_INFO("[DeviceManager] Discovery finished");

    loadAnalogInputChannel();
    loadAnalogOutputChannel();
    loadDigitalOutputChannel();

    emit listUpdated();
}


/**
 * @brief DeviceManager::devices Returns a list containing all discovered devices
 * @return QList with devices as DAQDevice objects
 */
QList<DAQDevice> DeviceManager::devices()
{
    return devlist;
}


/**
 * @brief DeviceManager::handleStartup Contains elements which should run at program start
 *        but after the settings are loaded / the core is initalized
 */
void DeviceManager::handleStartup()
{
    if(Core::instance()->ioSettings->hasEntry("deviceManager", "discoverAtStartup"))
        if(Core::instance()->ioSettings->value("deviceManager", "discoverAtStartup").toBool())
            discover();
    if(Core::instance()->ioSettings->hasEntry("deviceManager", "averageAnalogIn"))
        averageAnalogIn = Core::instance()->ioSettings->value("deviceManager", "averageAnalogIn").toBool();

    if(Core::instance()->ioSettings->hasEntry("DeviceManager", "AnalogInSampleBufferSize"))
        if(Core::instance()->ioSettings->value("DeviceManager", "AnalogInSampleBufferSize").toUInt() > 0)
            analogInSampleBufferSize = Core::instance()->ioSettings->value("DeviceManager", "AnalogInSampleBufferSize").toUInt();

    connect(Core::instance()->nativeEventFilter, SIGNAL(systemAboutToSuspend()), SLOT(onSystemSuspending()));
}


/**
 * @brief DeviceManager::discover Starts the discovery worker, if not already started
 */
void DeviceManager::discover()
{
    if(!discoveryWorker.isRunning())
        discoveryWorker.start();
}


/**
 * @brief DeviceManager::startAnalogOUT Creates the tasks and channels for the analog output
 *        and starts the analog output worker afterwards
 */
void DeviceManager::startAnalogOUT()
{
    MSG_DEBUG("startAnalogOUT()");

    if(analogOutputEnabled)
        stopAnalogOUT();

    int ret;

    for(int i = 0; i < analogOut.size(); i++)
    {
        ret = DAQmxCreateTask("", &analogOut[i].taskHandle);
        if(ret != 0)
        {
            if(ret > 0)
                MSG_WARN(QString("DAQmxCreateTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
            else
            {
                MSG_ERR(QString("DAQmxCreateTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
                stopAnalogOUT();
                return;
            }
        }
        ret = DAQmxCreateAOVoltageChan(analogOut[i].taskHandle, analogOut[i].channel.toStdString().data(), "", AOLimitMin, AOLimitMax, DAQmx_Val_Volts, "");
        if(ret != 0)
        {
            if(ret > 0)
                MSG_WARN(QString("DAQmxCreateAOVoltageChan: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
            else
            {
                MSG_ERR(QString("DAQmxCreateAOVoltageChan: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
                stopAnalogOUT();
                return;
            }
        }
        analogOut[i].voltageChanged = true;
    }

    if(analogOut.size() > 0)
    {
        analogOutputWorker.threadShouldStop = false;
        analogOutputWorker.start();
        setAnalogOutputEnabled(true);
        MSG_DEBUG("analog output started");
    }
    else
        stopAnalogOUT();
}


/**
 * @brief DeviceManager::stopAnalogOUT Stops the analog output worker and stops and clears
 *        all analog output tasks.
 */
void DeviceManager::stopAnalogOUT()
{
    MSG_DEBUG("stopAnalogOUT()");

    int ret;

    analogOutputWorker.threadShouldStop = true;
    int terminationCounter = 10;
    while(analogOutputWorker.isRunning())
    {
        terminationCounter--;
        QThread::msleep(10);
        if(terminationCounter == 0)
            analogOutputWorker.terminate();
    }

    for(int i = 0; i < analogOut.size(); i++)
    {
        if(analogOut[i].taskHandle != 0)
        {
            ret = DAQmxStopTask(analogOut[i].taskHandle);
            if(ret != 0)
            {
                if(ret > 0)
                    MSG_WARN(QString("DAQmxStopTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
                else
                    MSG_ERR(QString("DAQmxStopTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
            }
            ret = DAQmxClearTask(analogOut[i].taskHandle);
            if(ret != 0)
            {
                if(ret > 0)
                    MSG_WARN(QString("DAQmxClearTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
                else
                    MSG_ERR(QString("DAQmxClearTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
            }
            analogOut[i].taskHandle = 0;
        }
    }

    setAnalogOutputEnabled(false);
    MSG_DEBUG("analogOut stopped");
}


/**
 * @brief DeviceManager::setOutputVoltage Sets the voltage of the channel identified by the handle. If
 *        the analog output is enabled, the value is directly written
 * @param internalHandle The handle used to set up the channel
 * @param voltage The voltag to set in V
 */
void DeviceManager::setOutputVoltage(unsigned int internalHandle, double voltage)
{
    if(voltage < AOLimitMin)
        voltage = AOLimitMin;
    if(voltage > AOLimitMax)
        voltage = AOLimitMax;

    for(int i = 0; i < analogOut.size(); i++)
    {
        if(analogOut[i].internalHandle == internalHandle)
        {
            //qDebug() << "setting new output voltage" << analogOut_[i].internalHandle << analogOut_[i].channel << analogOut_[i].out;
            QMutexLocker locker(analogOut[i].lock);
            analogOut[i].out = voltage;
            analogOut[i].voltageChanged = true;
        }
    }
}


/**
 * @brief DeviceManager::setOutputChannel Set up an analog output channel.
 *        If the output is enabled when called, first of all the output is stopped and restarted
 *        after the channel is set.
 *        If the handle is already defined, the channel is changed to the one defined in the 'channel'
 *        parameter.
 *        To remove / disable a channel (and according handle), call with an empty string as 'channel'
 *        parameter.
 * @param internalHandle The handle to use, must be >= 0, used to change the voltage and channel,
 *        can be any value.
 * @param channel The full output channel, e.g. "dev1/ao1" or empty string to remove channel and handle.
 * @param voltage The initial output voltage in V, should be within analog output limits (is clamped
 *        otherwise).
 */
void DeviceManager::setOutputChannel(unsigned int internalHandle, QString channel, float voltage)
{
    setOutputChannel(internalHandle, channel, voltage, false);
}


void DeviceManager::setOutputChannel(unsigned int internalHandle, QString channel, float voltage, bool suppressSaving)
{
    bool outputWasEnabled = analogOutputEnabled;
    if(analogOutputEnabled)
        stopAnalogOUT();

    if(voltage < AOLimitMin)
        voltage = AOLimitMin;
    if(voltage > AOLimitMax)
        voltage = AOLimitMax;

    for(int i = 0; i < analogOut.size(); i++)
    {
        if(analogOut[i].internalHandle == internalHandle)
            analogOut.removeAt(i);
    }

    if(!channel.isEmpty())
    {
        analogOut.push_back(DAQAnalogOut(internalHandle, channel, voltage));
    }

    if(!suppressSaving)
        saveAnalogOutputChannel();

    if(outputWasEnabled)
        if(!analogOut.isEmpty())
            startAnalogOUT();
}


/**
 * @brief DeviceManager::setAnalogOutLimit Called to set the analog output limits.
 * @param min Analog output minimum, in V
 * @param max Analog output maximum, in V
 */
void DeviceManager::setAnalogOutLimit(double min, double max)
{
    AOLimitMin = min;
    AOLimitMax = max;

    emit analogOutputLimitChanged();
}


/**
 * @brief DeviceManager::getAnalogOutLimitMin
 * @return Analog output minimum, in V
 */
double DeviceManager::getAnalogOutLimitMin()
{
    return AOLimitMin;
}


/**
 * @brief DeviceManager::getAnalogOutLimitMax
 * @return Analog output maximum, in V
 */
double DeviceManager::getAnalogOutLimitMax()
{
    return AOLimitMax;
}


/**
 * @brief DeviceManager::setAnalogOutputEnabled Internally used, emits signal if the
 *        analog output is enabled or disabled.
 */
void DeviceManager::setAnalogOutputEnabled(bool enabled)
{
    analogOutputEnabled = enabled;
    emit analogOutputStateChanged(enabled);
}


/**
 * @brief DeviceManager::setAnalogInputEnabled Internally used, emits signal if the
 *        analog input is enabled or disabled.
 */
void DeviceManager::setAnalogInputEnabled(bool enabled)
{
    analogInputEnabled = enabled;
    emit analogInputStateChanged(enabled);
}


/**
 * @brief DeviceManager::startAnalogIN Creates the tasks and channels for the analog input
 *        and starts the analog input worker afterwards.
 *        Clears the averaging buffer of the channels.
 */
void DeviceManager::startAnalogIN()
{
    MSG_DEBUG("startAnalogIN()");

    if(analogInputEnabled)
        stopAnalogIN();

    int ret;

    for(int i = 0; i < analogIn.size(); i++)
    {
        ret = DAQmxCreateTask("", &analogIn[i].taskHandle);
        if(ret != 0)
        {
            if(ret > 0)
                MSG_WARN(QString("DAQmxCreateTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
            else
            {
                MSG_ERR(QString("DAQmxCreateTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
                stopAnalogIN();
                return;
            }
        }

        if(getDeviceTypeFromChannel(analogIn[i].channel).contains("NI 9207"))
            ret = DAQmxCreateAIVoltageChan(analogIn[i].taskHandle, analogIn[i].channel.toStdString().data(), "", DAQmx_Val_Diff, AOLimitMin, AOLimitMax, DAQmx_Val_Volts, NULL);
        else
            ret = DAQmxCreateAIVoltageChan(analogIn[i].taskHandle, analogIn[i].channel.toStdString().data(), "", DAQmx_Val_RSE, AOLimitMin, AOLimitMax, DAQmx_Val_Volts, NULL);

        if(ret != 0)
        {
            if(ret > 0)
                MSG_WARN(QString("DAQmxCreateAIVoltageChan: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
            else
            {
                MSG_ERR(QString("DAQmxCreateAIVoltageChan: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
                stopAnalogIN();
                return;
            }
        }
        analogIn[i].buffer.clear();
    }

    if(analogIn.size() > 0)
    {
        analogInputWorker.threadShouldStop = false;
        analogInputWorker.start();
        setAnalogInputEnabled(true);
        MSG_DEBUG("analog input started");
    }
    else
        stopAnalogIN();
}


/**
 * @brief DeviceManager::stopAnalogIN Stops the analog input worker and stops and clears
 *        all analog input tasks.
 */
void DeviceManager::stopAnalogIN()
{
    MSG_DEBUG("stopAnalogIN()");

    analogInputWorker.threadShouldStop = true;
    int terminationCounter = 10;
    while(analogInputWorker.isRunning())
    {
        //qDebug() << "terminationCounter analogIn =" << terminationCounter;
        QThread::msleep(10);
        if(terminationCounter == 0)
            analogInputWorker.terminate();
        terminationCounter--;
    }

    for(int i = 0; i < analogIn.size(); i++)
    {
        int ret;
        if(analogIn[i].taskHandle != 0)
        {
            ret = DAQmxStopTask(analogIn[i].taskHandle);
            if(ret != 0)
            {
                if(ret > 0)
                    MSG_WARN(QString("DAQmxStopTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
                else
                    MSG_ERR(QString("DAQmxStopTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
            }
            ret = DAQmxClearTask(analogIn[i].taskHandle);
            if(ret != 0)
            {
                if(ret > 0)
                    MSG_WARN(QString("DAQmxClearTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
                else
                    MSG_ERR(QString("DAQmxClearTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
            }
            analogIn[i].taskHandle = 0;
        }
    }
    MSG_DEBUG("analog input stopped");

    setAnalogInputEnabled(false);
    emit analogInputReset();
}


/**
 * @brief DeviceManager::setInputChannel Set up an analog input channel.
 *        If the input is enabled when called, it is firstly stopped and restarted after the channel
 *        was set.
 *        If the handle is already defined, the channel is changed to the one defined in the 'channel'
 *        parameter.
 *        To remove / disable a channel (and according handle), call with an empty string as 'channel'
 *        parameter.
 * @param internalHandle The handle to use, must be >= 0, used to change the channel, can be any value.
 * @param channel The full output channel, e.g. "dev1/ai1" or empty string to remove channel and handle.
 */
void DeviceManager::setInputChannel(unsigned int internalHandle, QString channel)
{
    setInputChannel(internalHandle, channel, false);
}


void DeviceManager::setInputChannel(unsigned int internalHandle, QString channel, bool suppressSaving)
{
    bool inputWasEnabled = analogInputEnabled;
    if(analogInputEnabled)
        stopAnalogIN();

    for(int i = 0; i < analogIn.size(); i++)
    {
        if(analogIn[i].internalHandle == internalHandle)
            analogIn.removeAt(i);
    }

    if(!channel.isEmpty())
    {
        analogIn.push_back(DAQAnalogIn(internalHandle, channel, analogInSampleBufferSize));
    }

    if(!suppressSaving)
            saveAnalogInputChannel();

    if(inputWasEnabled)
        if(!analogIn.isEmpty())
            startAnalogIN();
}


/**
 * @brief DeviceManager::getAnalogOutChannelIdentifier Returns the channel associated with the handle if
 *        present, an empty string otherwise.
 * @param internalHandle
 * @return QString containing the channel, similar to "dev1/ao0", or an empty string
 */
QString DeviceManager::getAnalogOutChannelIdentifier(unsigned int internalHandle)
{
    QString identifier("");
    for(int i = 0; i < analogOut.size(); i++)
    {
        if(analogOut[i].internalHandle == internalHandle)
            identifier = analogOut[i].channel;
    }
    return identifier;
}


/**
 * @brief DeviceManager::getAnalogInChannelIdentifier Returns the channel associated with the handle if
 *        present, an empty string otherwise.
 * @param internalHandle
 * @return QString containing the channel, similar to "dev1/ai0", or an empty string
 */
QString DeviceManager::getAnalogInChannelIdentifier(unsigned int internalHandle)
{
    QString channel("");
    for(int i = 0; i < analogIn.size(); i++)
    {
        if(analogIn[i].internalHandle == internalHandle)
            channel = analogIn[i].channel;
    }
    return channel;
}


/**
 * @brief DeviceManager::saveAnalogInputChannel Internally used to save the configured analog output
 *        channels.
 *        TODO: If more than the defined 4 channel should be used, this must be extended.
 */
void DeviceManager::saveAnalogInputChannel()
{
    //none-generic approach following, because reasons
    for(int i = 1; i < 5; i++)
    {
        QString identifier("");
        for(int j = 0; j < analogIn.size(); j++)
        {
            if(analogIn[j].internalHandle == i)
                identifier = analogIn[j].channel;
        }
        Core::instance()->ioSettings->setValue("DeviceManager", "AnalogIn"+QString::number(i), identifier);
    }
}


/**
 * @brief DeviceManager::loadAnalogInputChannel Internally used to load the configured analog output
 *        channels.
 *        TODO: If more than the defined 4 channel should be used, this must be extended
 */
void DeviceManager::loadAnalogInputChannel()
{
    for(int i = 1; i < 5; i++)
    {
        if(Core::instance()->ioSettings->hasEntry("DeviceManager", "AnalogIn"+QString::number(i)))
        {
            if(!Core::instance()->ioSettings->value("DeviceManager", "AnalogIn"+QString::number(i)).toString().isEmpty())
                setInputChannel(i, Core::instance()->ioSettings->value("DeviceManager", "AnalogIn"+QString::number(i)).toString(), true);
        }
    }
}


/**
 * @brief DeviceManager::loadAnalogOutputChannel Internally used to load the configured analog output
 *        channels.
 *        TODO: If more than the defined 4 channel should be used, this must be extended
 */
void DeviceManager::loadAnalogOutputChannel()
{
    for(int i = 1; i < 5; i++)
    {
        if(Core::instance()->ioSettings->hasEntry("DeviceManager", "AnalogOut"+QString::number(i)))
        {
            if(!Core::instance()->ioSettings->value("DeviceManager", "AnalogOut"+QString::number(i)).toString().isEmpty())
                setOutputChannel(i, Core::instance()->ioSettings->value("DeviceManager", "AnalogOut"+QString::number(i)).toString(), 0.3, true);
        }
    }
}


/**
 * @brief DeviceManager::saveAnalogOutputChannel Internally used to save the configured analog output
 *        channels.
 *        TODO: If more than the defined 4 channel should be used, this must be extended
 */
void DeviceManager::saveAnalogOutputChannel()
{
    for(int i = 1; i < 5; i++)
    {
        QString channel("");
        for(int j = 0; j < analogOut.size(); j++)
        {
            if(analogOut[j].internalHandle == i)
                channel = analogOut[j].channel;
        }
        Core::instance()->ioSettings->setValue("DeviceManager", "AnalogOut"+QString::number(i), channel);
    }
}


/**
 * @brief DeviceManager::startDigitalOUT Creates the tasks and channels for the digital output
 *        and starts the digital output worker afterwards
 */
void DeviceManager::startDigitalOUT()
{
    MSG_DEBUG("startDigitalOUT()");

    if(digitalOutputEnabled)
        stopDigitalOUT();

    int ret;

    for(int i = 0; i < digitalOut.size(); i++)
    {
        ret = DAQmxCreateTask("", &digitalOut[i].taskHandle);
        if(ret != 0)
        {
            if(ret > 0)
                MSG_WARN(QString("DAQmxCreateTask: ").append(DAQmxErrorAsString(ret)).append(" for %0 %1").arg(digitalOut[i].dev).arg(digitalOut[i].port));
            else
            {
                MSG_ERR(QString("DAQmxCreateTask: ").append(DAQmxErrorAsString(ret)).append(" for %0 %1").arg(digitalOut[i].dev).arg(digitalOut[i].port));
                stopDigitalOUT();
                return;
            }
        }

        QString port = QString().append(digitalOut[i].dev).append("/").append(digitalOut[i].port);
        uInt32 portwidth = 0;
        ret = DAQmxGetPhysicalChanDOPortWidth(port.toStdString().data(), &portwidth);
        if(ret != 0)
        {
            if(ret > 0)
                MSG_WARN(QString("DAQmxGetPhysicalChanDOPortWidth: ").append(DAQmxErrorAsString(ret)).append(" for %0 %1").arg(digitalOut[i].dev).arg(digitalOut[i].port));
            else
            {
                MSG_ERR(QString("DAQmxGetPhysicalChanDOPortWidth: ").append(DAQmxErrorAsString(ret)).append(" for %0 %1").arg(digitalOut[i].dev).arg(digitalOut[i].port));
                stopDigitalOUT();
                return;
            }
        }
        if(portwidth == 0)
        {
            MSG_ERR(QString("Portwidth returned 0 for %0 %1").arg(digitalOut[i].dev).arg(digitalOut[i].port));
            stopDigitalOUT();
            return;
        }
        port.append("/line0:").append(QString::number(portwidth-1));

        ret = DAQmxCreateDOChan(digitalOut[i].taskHandle, port.toStdString().data(), "", DAQmx_Val_ChanForAllLines);
        if(ret != 0)
        {
            if(ret > 0)
                MSG_WARN(QString("DAQmxCreateDOChan: ").append(DAQmxErrorAsString(ret)).append(" for %0 %1").arg(digitalOut[i].dev).arg(digitalOut[i].port));
            else
            {
                MSG_ERR(QString("DAQmxCreateDOChan: ").append(DAQmxErrorAsString(ret)).append(" for %0 %1").arg(digitalOut[i].dev).arg(digitalOut[i].port));
                stopDigitalOUT();
                return;
            }
        }
        digitalOut[i].out[0] = 0;
        for(int j = 0; j < digitalOut[i].list.size(); j++)
        {
            if(digitalOut[i].list[j].enabled)
                digitalOut[i].out[0] |= (1 << digitalOut[i].list[j].channelNumber);
        }
        //qDebug() << "out =" << digitalOut[i].out[0];
        digitalOut[i].changed = true;
    }

    if(digitalOut.size() > 0)
    {
        digitalOutputWorker.threadShouldStop = false;
        digitalOutputWorker.start();
        setDigitalOutputEnabled(true);
        MSG_DEBUG("digital output started");
    }
    else
        stopDigitalOUT();
}


/**
 * @brief DeviceManager::stopDigitalOUT Stops the digital output worker and stops / clears all
 *        digital output tasks
 */
void DeviceManager::stopDigitalOUT()
{
    MSG_DEBUG("stopDigitalOUT()");

    int ret;

    digitalOutputWorker.threadShouldStop = true;
    int terminationCounter = 10;
    while(digitalOutputWorker.isRunning())
    {
        terminationCounter--;
        QThread::msleep(10);
        if(terminationCounter == 0)
            digitalOutputWorker.terminate();
    }

    for(int i = 0; i < digitalOut.size(); i++)
    {
        if(digitalOut[i].taskHandle != 0)
        {
            ret = DAQmxStopTask(digitalOut[i].taskHandle);
            if(ret != 0)
            {
                if(ret > 0)
                    MSG_WARN(QString("DAQmxStopTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
                else
                    MSG_ERR(QString("DAQmxStopTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
            }
            ret = DAQmxClearTask(digitalOut[i].taskHandle);
            if(ret != 0)
            {
                if(ret > 0)
                    MSG_WARN(QString("DAQmxClearTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
                else
                    MSG_ERR(QString("DAQmxClearTask: ").append(DAQmxErrorAsString(ret)).append(" for %0").arg(i));
            }
            digitalOut[i].taskHandle = 0;
        }
    }

    setDigitalOutputEnabled(false);
    emit digitalOutputChanged();
    MSG_DEBUG("digitalOut stopped");
}


/**
 * @brief DeviceManager::setDigitalOutputChannel Set up an digital output channel.
 *        If the output is enabled when called, first of all the output is stopped and restarted
 *        after the channel is set.
 *        If the handle is already defined, the channel is changed to the one defined in the 'channel'
 *        parameter.
 *        To remove / disable a channel (and according handle), call with an empty string as 'channel'
 *        parameter.
 * @param internalHandle The handle to use, must be >= 0, used to change the voltage and channel,
 *        can be any value.
 * @param channel The full output channel, e.g. "dev1/port0/line0" or an empty string to remove
 *        channel and handle.
 * @param inverted If the channel should be inverted, e.g. set to high when disabled, low when enabled.
 */
void DeviceManager::setDigitalOutputChannel(unsigned int internalHandle, QString channel, bool inverted)
{
    setDigitalOutputChannel(internalHandle, channel, inverted, false);
}


void DeviceManager::setDigitalOutputChannel(unsigned int internalHandle, QString channel, bool inverted, bool suppressSaving)
{
    //qDebug() << "setDigitalOutputChannel" << internalHandle << channel;

    bool outputWasEnabled = digitalOutputEnabled;
    if(digitalOutputEnabled)
        stopDigitalOUT();

    //dev# / port# / line#
    QStringList dpl = channel.split("/");

    for(int i = 0; i < digitalOut.size(); i++)
    {
        for(int j = 0; j < digitalOut[i].list.size(); j++)
        {
            if(digitalOut[i].list[j].internalHandle == internalHandle)
                digitalOut[i].list.removeAt(j);
        }
        if(digitalOut[i].list.size() == 0)
            digitalOut.removeAt(i);
    }

    emit digitalOutputChanged();

    if(dpl.size() == 3)
    {
        bool numberok;
        unsigned int line = dpl[2].remove("line").toUInt(&numberok);
        if(numberok)
        {
            int inListPosition = -1;
            for(int i = 0; i < digitalOut.size(); i++)
            {
                if(digitalOut[i].dev == dpl[0])
                    if(digitalOut[i].port == dpl[1])
                    {
                        //qDebug() << digitalOut[i].dev << "=" << dpl[0];
                        //qDebug() << digitalOut[i].port << "=" << dpl[1];
                        inListPosition = i;
                    }
            }

            if(inListPosition == -1)
            {
                digitalOut.push_back(DAQDigitalOut(dpl[0], dpl[1]));
                digitalOut[digitalOut.size()-1].list.append(DAQDigitalOutHelper(internalHandle, line, inverted, inverted));
            }
            else
            {
                digitalOut[inListPosition].list.append(DAQDigitalOutHelper(internalHandle, line, inverted, inverted));
            }
        }
        else
            MSG_ERR("SetDigitalOutputChannel: Could not parse line number");
    }

    if(!suppressSaving)
        saveDigitalOutputChannel();

    if(outputWasEnabled)
        if(!digitalOut.isEmpty())
            startDigitalOUT();

//    for(int i = 0; i < digitalOut.size(); i++)
//    {
//        for(int j = 0; j < digitalOut[i].list.size(); j++)
//        {
//            qDebug() << digitalOut[i].dev << digitalOut[i].port << digitalOut[i].list[j].internalHandle << digitalOut[i].list[j].channelNumber << digitalOut[i].list[j].enabled << digitalOut[i].list[j].inverted;
//        }
//    }
}


/**
 * @brief DeviceManager::setDigitalOutputEnabled Sets the state of the channel identified by the
 *        handle. Writes the new value if digital output is enabled.
 * @param internalHandle The handle used to set up the channel (ignores any handle which was not
 *        previously defined by calling 'setDigitalOutputChannel', throws no error).
 * @param enabled If output should be enabled(high, low if inverted) or disabled (low, high if
 *        inverted).
 */
void DeviceManager::setDigitalOutputEnabled(unsigned int internalHandle, bool enabled)
{
    for(int i = 0; i < digitalOut.size(); i++)
    {
        QMutexLocker locker(digitalOut[i].lock);
        for(int j = 0; j < digitalOut[i].list.size(); j++)
        {
            if(digitalOut[i].list[j].internalHandle == internalHandle)
            {
                if(digitalOut[i].list[j].inverted)
                    digitalOut[i].list[j].enabled = !enabled;
                else
                    digitalOut[i].list[j].enabled = enabled;
                digitalOut[i].changed = true;
            }
        }
        digitalOut[i].out[0] = 0;
        for(int j = 0; j < digitalOut[i].list.size(); j++)
        {
            if(digitalOut[i].list[j].enabled)
                digitalOut[i].out[0] |= (1 << digitalOut[i].list[j].channelNumber);
        }
    }
}


/**
 * @brief DeviceManager::getDigitalOutputChannel Returns the channel associated with the handle if
 *        present, otherwise an empty string
 * @param internalHandle
 * @return QString containing the channel, similar to "dev1/port0/line0", or an empty string
 */
QString DeviceManager::getDigitalOutputChannel(unsigned int internalHandle)
{
    QString line("");
    for(int i = 0; i < digitalOut.size(); i++)
    {
        for(int j = 0; j < digitalOut[i].list.size(); j++)
        {
            if(digitalOut[i].list[j].internalHandle == internalHandle)
            {
                line.append(digitalOut[i].dev);
                line.append("/");
                line.append(digitalOut[i].port);
                line.append("/line");
                line.append(QString::number(digitalOut[i].list[j].channelNumber));
                //qDebug() << "getDigitalOutputChannel" << line;
            }
        }
    }
    return line;
}


/**
 * @brief DeviceManager::getDigitalOutputChannelEnabled Returns the state of the channel associated with
 *        the handle if present, false otherwise
 * @param internalHandle
 * @return bool containing the state
 */
bool DeviceManager::getDigitalOutputChannelEnabled(unsigned int internalHandle)
{
    for(int i = 0; i < digitalOut.size(); i++)
    {
        for(int j = 0; j < digitalOut[i].list.size(); j++)
        {
            if(digitalOut[i].list[j].internalHandle == internalHandle)
            {
                if(digitalOut[i].list[j].inverted)
                    return !digitalOut[i].list[j].enabled;
                else
                    return digitalOut[i].list[j].enabled;
            }
        }
    }
    return false;
}


/**
 * @brief DeviceManager::getDigitalOutputInverted Returns if the channel associated with the handle is
 *        inverted, if the handle is present. Otherwise returns false.
 * @param internalHandle
 * @return bool containing if inverted
 */
bool DeviceManager::getDigitalOutputInverted(unsigned int internalHandle)
{
    for(int i = 0; i < digitalOut.size(); i++)
    {
        for(int j = 0; j < digitalOut[i].list.size(); j++)
        {
            if(digitalOut[i].list[j].internalHandle == internalHandle)
                return digitalOut[i].list[j].inverted;
        }
    }
    return false;
}


/**
 * @brief DeviceManager::setDigitalOutputEnabled Internally used, emits signal if the digital output
 *        is enabled or disabled.
 */
void DeviceManager::setDigitalOutputEnabled(bool enabled)
{
    digitalOutputEnabled = enabled;
    emit digitalOutputStateChanged(enabled);
}


/**
 * @brief DeviceManager::saveDigitalOutputChannel Internally used to save the configured digital output
 *        channels.
 *        TODO: Needs to be extended if more than the defined 9 channels should be used/saved.
 */
void DeviceManager::saveDigitalOutputChannel()
{
    for(int i = 1; i < 10; i++)
    {
        for(int j = 0; j < digitalOut.size(); j++)
        {
            QString channel("");
            bool inverted = false;
            for(int k = 0; k < digitalOut[j].list.size(); k++)
            {
                if(digitalOut[j].list[k].internalHandle == i)
                {
                    channel.append(digitalOut[j].dev).append("/").append(digitalOut[j].port).append("/line").append(QString::number(digitalOut[j].list[k].channelNumber));
                    inverted = digitalOut[j].list[k].inverted;
                }
            }
            Core::instance()->ioSettings->setValue("DeviceManager", "DigitalOut"+QString::number(i), channel);
            Core::instance()->ioSettings->setValue("DeviceManager", "DigitalOutInverted"+QString::number(i), inverted);
        }
    }
}


/**
 * @brief DeviceManager::loadDigitalOutputChannel Internally used to load the configured digital output
 *        channels.
 *        TODO: Needs to be extended if more than the defined 9 channels should be used/loaded.
 */
void DeviceManager::loadDigitalOutputChannel()
{   
    for(int i = 1; i < 9; i++)
    {
        if(Core::instance()->ioSettings->hasEntry("DeviceManager", "DigitalOut"+QString::number(i)))
        {
            if(!Core::instance()->ioSettings->value("DeviceManager", "DigitalOut"+QString::number(i)).toString().isEmpty())
                setDigitalOutputChannel(i, Core::instance()->ioSettings->value("DeviceManager", "DigitalOut"+QString::number(i)).toString(), Core::instance()->ioSettings->value("DeviceManager", "DigitalOutInverted"+QString::number(i)).toBool(), true);
        }
    }
}


/**
 * @brief DeviceManager::onSystemSuspending Should be connected with the event filter, to prevent
 *        device lockups / program crashes when the system is suspended / waked up
 */
void DeviceManager::onSystemSuspending()
{
    stopAnalogOUT();
    stopAnalogIN();
    stopDigitalOUT();
}


/**
 * @brief DeviceManager::setAverageAnalogIn Set if the analog output value emitted by the analog
 *        output worker should be the average or the single readed value
 * @param enabled True: average is emitted | False: single readed value is emitted
 */
void DeviceManager::setAverageAnalogIn(bool enabled)
{
    averageAnalogIn = enabled;
    Core::instance()->ioSettings->setValue("deviceManager", "averageAnalogIn", enabled);
}


/**
 * @brief DeviceManager::getAverageAnalogIn
 * @return bool: True when set to emit average, false when set to emit single readed value
 */
bool DeviceManager::getAverageAnalogIn()
{
    return averageAnalogIn;
}


/**
 * @brief DeviceManager::getAnalogInputIsValid
 * @return bool: True if the analog input is enabled, false otherwise
 */
bool DeviceManager::getAnalogInputIsValid()
{
    return analogInputEnabled;
}


/**
 * @brief DeviceManager::getAnalogInputValue Returns the latest readed value of the channel
 *        associated with the handle. When the handle is not present, returns 0.0.
 * @param internalHandle
 * @return double containing the value
 */
double DeviceManager::getAnalogInputValue(unsigned int internalHandle)
{
    for(int i = 0; i < analogIn.size(); i++)
    {
        if(analogIn[i].internalHandle == internalHandle)
            return analogIn[i].lastValue;
    }
    MSG_ERR(QString("DeviceManager::getAnalogInputValue value for internal handle %0 not available, returning 0.0 (are the channels set in the device manager?)").arg(internalHandle));
    return 0.0;
}


/**
 * @brief DeviceManager::getAnalogInputAverageValue Returns the latest averaged value of
 *        the channel associated with the handle. When the handle is not present, returns 0.0.
 * @param internalHandle
 * @return double containing the value
 */
double DeviceManager::getAnalogInputAverageValue(unsigned int internalHandle)
{
    for(int i = 0; i < analogIn.size(); i++)
    {
        if(analogIn[i].internalHandle == internalHandle)
            return analogIn[i].lastAverage;
    }
    MSG_ERR(QString("DeviceManager::getAnalogInputAverageValue value for internal handle %0 not available, returning 0.0 (are the channels set in the device manager?)").arg(internalHandle));
    return 0.0;
}


void DeviceManager::setLaserAnalogOutputEnabled(bool enabled)
{
    laserAnalogOutputEnabled = enabled;
}


void DeviceManager::setLaserAnalogInputEnabled(bool enabled)
{
    laserAnalogInputEnabled = enabled;
}


void DeviceManager::setLaserOutputVoltage(double voltage)
{
    if(voltage < 0.0)
        voltage = 0.0;
    if(voltage > 5.0)
        voltage = 5.0;

    QMutexLocker locker(laserAnalogOut.lock);
    laserAnalogOut.out = voltage;
    laserAnalogOut.voltageChanged  = true;
}

void DeviceManager::setLaserOutputChannel(QString channel, double voltage)
{
    bool outputWasEnabled = laserAnalogOutputEnabled;
    if(outputWasEnabled)
        stopLaserAnalogOUT();

    if(voltage < 0.0)
        voltage = 0.0;
    if(voltage > 5.0)
        voltage = 5.0;

    laserAnalogOut.channel = channel;
    laserAnalogOut.voltageChanged = true;
    laserAnalogOut.out = voltage;

    if(outputWasEnabled)
        startLaserAnalogOUT();
}

void DeviceManager::setLaserInputChannel(QString channel)
{
    bool inputWasEnabled = laserAnalogInputEnabled;
    if(inputWasEnabled)
        stopLaserAnalogIN();

    laserAnalogIn.channel = channel;

    if(inputWasEnabled)
        startLaserAnalogIN();
}

void DeviceManager::startLaserAnalogOUT()
{
    MSG_DEBUG("startLaserAnalogOUT()");

    if(laserAnalogOutputEnabled)
        stopLaserAnalogOUT();

    int ret;

    ret = DAQmxCreateTask("", &laserAnalogOut.taskHandle);
    if(ret != 0)
    {
        if(ret > 0)
            MSG_WARN(QString("Laser DAQmxCreateTask: ").append(DAQmxErrorAsString(ret)));
        else
        {
            MSG_ERR(QString("Laser DAQmxCreateTask: ").append(DAQmxErrorAsString(ret)));
            stopLaserAnalogOUT();
            return;
        }
    }
    ret = DAQmxCreateAOVoltageChan(laserAnalogOut.taskHandle, laserAnalogOut.channel.toStdString().data(), "", 0.0, 5.0, DAQmx_Val_Volts, "");
    if(ret != 0)
    {
        if(ret > 0)
            MSG_WARN(QString("Laser DAQmxCreateAOVoltageChan: ").append(DAQmxErrorAsString(ret)));
        else
        {
            MSG_ERR(QString("Laser DAQmxCreateAOVoltageChan: ").append(DAQmxErrorAsString(ret)));
            stopLaserAnalogOUT();
            return;
        }
    }
    laserAnalogOut.voltageChanged = true;

    analogOutputWorkerLaser.threadShouldStop = false;
    analogOutputWorkerLaser.start();

    setLaserAnalogOutputEnabled(true);

    MSG_DEBUG("laser analog output started");
}

void DeviceManager::stopLaserAnalogOUT()
{
    MSG_DEBUG("stopLaserAnalogOUT()");

    int ret;

    analogOutputWorkerLaser.threadShouldStop = true;
    int terminationCounter = 10;
    while(analogOutputWorkerLaser.isRunning())
    {
        terminationCounter--;
        QThread::msleep(10);
        if(terminationCounter == 0)
            analogOutputWorkerLaser.terminate();
    }

    ret = DAQmxStopTask(laserAnalogOut.taskHandle);
    if(ret != 0)
    {
        if(ret > 0)
            MSG_WARN(QString("Laser DAQmxStopTask: ").append(DAQmxErrorAsString(ret)));
        else
            MSG_ERR(QString("Laser DAQmxStopTask: ").append(DAQmxErrorAsString(ret)));
    }
    ret = DAQmxClearTask(laserAnalogOut.taskHandle);
    if(ret != 0)
    {
        if(ret > 0)
            MSG_WARN(QString("Laser DAQmxClearTask: ").append(DAQmxErrorAsString(ret)));
        else
            MSG_ERR(QString("Laser DAQmxClearTask: ").append(DAQmxErrorAsString(ret)));
    }
    laserAnalogOut.taskHandle = 0;

    setLaserAnalogOutputEnabled(false);

    MSG_DEBUG("laser analogOut stopped");
}

void DeviceManager::startLaserAnalogIN()
{
    MSG_DEBUG("startLaserAnalogIN()");

    if(laserAnalogInputEnabled)
        stopLaserAnalogIN();

    int ret;

    ret = DAQmxCreateTask("", &laserAnalogIn.taskHandle);
    if(ret != 0)
    {
        if(ret > 0)
            MSG_WARN(QString("DAQmxCreateTask: ").append(DAQmxErrorAsString(ret)));
        else
        {
            MSG_ERR(QString("DAQmxCreateTask: ").append(DAQmxErrorAsString(ret)));
            stopLaserAnalogIN();
            return;
        }
    }
    if(getDeviceTypeFromChannel(laserAnalogIn.channel).contains("NI 9207"))
        ret = DAQmxCreateAIVoltageChan(laserAnalogIn.taskHandle, laserAnalogIn.channel.toStdString().data(), "", DAQmx_Val_Diff, 0.0, 5.0, DAQmx_Val_Volts, NULL);
    else
        ret = DAQmxCreateAIVoltageChan(laserAnalogIn.taskHandle, laserAnalogIn.channel.toStdString().data(), "", DAQmx_Val_RSE, 0.0, 5.0, DAQmx_Val_Volts, NULL);
    if(ret != 0)
    {
        if(ret > 0)
            MSG_WARN(QString("DAQmxCreateAIVoltageChan: ").append(DAQmxErrorAsString(ret)));
        else
        {
            MSG_ERR(QString("DAQmxCreateAIVoltageChan: ").append(DAQmxErrorAsString(ret)));
            stopLaserAnalogIN();
            return;
        }
    }
    laserAnalogIn.buffer.clear();
    laserAnalogIn.buffer.set_capacity(analogInSampleBufferSize);

    analogInputWorkerLaser.threadShouldStop = false;
    analogInputWorkerLaser.start();

    setLaserAnalogInputEnabled(true);

    MSG_DEBUG("laser analog input started");
}

void DeviceManager::stopLaserAnalogIN()
{
    MSG_DEBUG("stopLaserAnalogIN()");

    analogInputWorkerLaser.threadShouldStop = true;
    int terminationCounter = 10;
    while(analogInputWorkerLaser.isRunning())
    {
        //qDebug() << "terminationCounter analogIn =" << terminationCounter;
        QThread::msleep(10);
        if(terminationCounter == 0)
            analogInputWorkerLaser.terminate();
        terminationCounter--;
    }

    int ret;
    if(laserAnalogIn.taskHandle != 0)
    {
        ret = DAQmxStopTask(laserAnalogIn.taskHandle);
        if(ret != 0)
        {
            if(ret > 0)
                MSG_WARN(QString("DAQmxStopTask: ").append(DAQmxErrorAsString(ret)));
            else
                MSG_ERR(QString("DAQmxStopTask: ").append(DAQmxErrorAsString(ret)));
        }
        ret = DAQmxClearTask(laserAnalogIn.taskHandle);
        if(ret != 0)
        {
            if(ret > 0)
                MSG_WARN(QString("DAQmxClearTask: ").append(DAQmxErrorAsString(ret)));
            else
                MSG_ERR(QString("DAQmxClearTask: ").append(DAQmxErrorAsString(ret)));
        }
        laserAnalogIn.taskHandle = 0;
    }

    setLaserAnalogInputEnabled(false);

    MSG_DEBUG("laser analog input stopped");
}


bool DeviceManager::getLaserAnalogOutRunning()
{
    return analogOutputWorkerLaser.isRunning();
}


bool DeviceManager::getLaserAnalogInRunning()
{
    return analogInputWorkerLaser.isRunning();
}


void DeviceManager::setAnalogInAverageSampleSize(unsigned int samples)
{
    if(samples > 0)
    {
        analogInSampleBufferSize = samples;

        for(int i = 0; i < analogIn.size(); i++)
        {
            analogIn[i].buffer.set_capacity(analogInSampleBufferSize);
        }
        laserAnalogIn.buffer.set_capacity(analogInSampleBufferSize);
        Core::instance()->ioSettings->setValue("DeviceManager", "AnalogInSampleBufferSize", analogInSampleBufferSize);
    }
}


unsigned int DeviceManager::getAnalogInAverageSampleSize()
{
    return analogInSampleBufferSize;
}


QString DeviceManager::getDeviceTypeFromChannel(QString channel)
{
    for(int i = 0; i < devlist.size(); i++)
    {
        for(int j = 0; j < devlist.at(i).analogIn.size(); j++)
        {
            if(devlist.at(i).analogIn.value(j).compare(channel) == 0)
                return devlist.at(i).type;
        }
        for(int j = 0; j < devlist.at(i).analogOut.size(); j++)
        {
            if(devlist.at(i).analogOut.value(j).compare(channel) == 0)
                return devlist.at(i).type;
        }
    }
    return QString();
}


/**
 * @brief DeviceManager::DAQmxErrorAsString Internally used, returns the error string
 *        associated with the error code.
 */
QString DeviceManager::DAQmxErrorAsString(int error)
{
    switch(error)
    {
    case -209825: return "DAQmxErrorInconsistentAODACRangeAcrossTasks"; break;
    case -209824: return "DAQmxErrorInconsistentDTToWrite"; break;
    case -209823: return "DAQmxErrorFunctionObsolete"; break;
    case -209822: return "DAQmxErrorNegativeDurationNotSupported"; break;
    case -209821: return "DAQmxErrorDurationTooSmall"; break;
    case -209820: return "DAQmxErrorDurationTooLong"; break;
    case -209819: return "DAQmxErrorDurationBasedNotSupportedForSpecifiedTimingMode"; break;
    case -209818: return "DAQmxErrorInvalidLEDState"; break;
    case -209817: return "DAQmxErrorWatchdogStatesNotUniform"; break;
    case -209816: return "DAQmxErrorSelfTestFailedPowerSupplyOutOfTolerance"; break;
    case -209815: return "DAQmxErrorHWTSPMultiSampleWrite"; break;
    case -209814: return "DAQmxErrorOnboardRegenExceedsChannelLimit"; break;
    case -209813: return "DAQmxErrorWatchdogChannelExpirationStateNotSpecified"; break;
    case -209812: return "DAQmxErrorInvalidShuntSourceForCalibration"; break;
    case -209811: return "DAQmxErrorInvalidShuntSelectForCalibration"; break;
    case -209810: return "DAQmxErrorInvalidShuntCalibrationConfiguration"; break;
    case -209809: return "DAQmxErrorBufferedOperationsNotSupportedOnChannelStandalone"; break;
    case -209808: return "DAQmxErrorFeatureNotAvailableOnAccessory"; break;
    case -209807: return "DAQmxErrorInconsistentThreshVoltageAcrossTerminals"; break;
    case -209806: return "DAQmxErrorDAQmxIsNotInstalledOnTarget"; break;
    case -209805: return "DAQmxErrorCOCannotKeepUpInHWTimedSinglePoint"; break;
    case -209803: return "DAQmxErrorWaitForNextSampClkDetected3OrMoreSampClks"; break;
    case -209802: return "DAQmxErrorWaitForNextSampClkDetectedMissedSampClk"; break;
    case -209801: return "DAQmxErrorWriteNotCompleteBeforeSampClk"; break;
    case -209800: return "DAQmxErrorReadNotCompleteBeforeSampClk"; break;
    case -201510: return "DAQmxErrorInconsistentDigitalFilteringAcrossTerminals"; break;
    case -201509: return "DAQmxErrorInconsistentPullUpCfgAcrossTerminals"; break;
    case -201508: return "DAQmxErrorInconsistentTermCfgAcrossTerminals"; break;
    case -201507: return "DAQmxErrorVCXODCMBecameUnlocked"; break;
    case -201506: return "DAQmxErrorPLLDACUpdateFailed"; break;
    case -201505: return "DAQmxErrorNoCabledDevice"; break;
    case -201504: return "DAQmxErrorLostRefClk"; break;
    case -201503: return "DAQmxErrorCantUseAITimingEngineWithCounters"; break;
    case -201502: return "DAQmxErrorDACOffsetValNotSet"; break;
    case -201501: return "DAQmxErrorCalAdjustRefValOutOfRange"; break;
    case -201500: return "DAQmxErrorChansForCalAdjustMustPerformSetContext"; break;
    case -201499: return "DAQmxErrorGetCalDataInvalidForCalMode"; break;
    case -201498: return "DAQmxErrorNoIEPEWithACNotAllowed"; break;
    case -201497: return "DAQmxErrorSetupCalNeededBeforeGetCalDataPoints"; break;
    case -201496: return "DAQmxErrorVoltageNotCalibrated"; break;
    case -201495: return "DAQmxErrorMissingRangeForCalibration"; break;
    case -201494: return "DAQmxErrorMultipleChansNotSupportedDuringCalAdjust"; break;
    case -201493: return "DAQmxErrorShuntCalFailedOutOfRange"; break;
    case -201492: return "DAQmxErrorOperationNotSupportedOnSimulatedDevice"; break;
    case -201491: return "DAQmxErrorFirmwareVersionSameAsInstalledVersion"; break;
    case -201490: return "DAQmxErrorFirmwareVersionOlderThanInstalledVersion"; break;
    case -201489: return "DAQmxErrorFirmwareUpdateInvalidState"; break;
    case -201488: return "DAQmxErrorFirmwareUpdateInvalidID"; break;
    case -201487: return "DAQmxErrorFirmwareUpdateAutomaticManagementEnabled"; break;
    case -201486: return "DAQmxErrorSetupCalibrationNotCalled"; break;
    case -201485: return "DAQmxErrorCalMeasuredDataSizeVsActualDataSizeMismatch"; break;
    case -201484: return "DAQmxErrorCDAQMissingDSAMasterForChanExpansion"; break;
    case -201483: return "DAQmxErrorCDAQMasterNotFoundForChanExpansion"; break;
    case -201482: return "DAQmxErrorAllChansShouldBeProvidedForCalibration"; break;
    case -201481: return "DAQmxErrorMustSpecifyExpirationStateForAllLinesInRange"; break;
    case -201480: return "DAQmxErrorOpenSessionExists"; break;
    case -201479: return "DAQmxErrorCannotQueryTerminalForSWArmStart"; break;
    case -201478: return "DAQmxErrorChassisWatchdogTimerExpired"; break;
    case -201477: return "DAQmxErrorCantReserveWatchdogTaskWhileOtherTasksReserved"; break;
    case -201476: return "DAQmxErrorCantReserveTaskWhileWatchdogTaskReserving"; break;
    case -201475: return "DAQmxErrorAuxPowerSourceRequired"; break;
    case -201474: return "DAQmxErrorDeviceNotSupportedOnLocalSystem"; break;
    case -201472: return "DAQmxErrorOneTimestampChannelRequiredForCombinedNavigationRead"; break;
    case -201471: return "DAQmxErrorMultDevsMultPhysChans"; break;
    case -201470: return "DAQmxErrorInvalidCalAdjustmentPointValues"; break;
    case -201469: return "DAQmxErrorDifferentDigitizerFromCommunicator"; break;
    case -201468: return "DAQmxErrorCDAQSyncMasterClockNotPresent"; break;
    case -201467: return "DAQmxErrorAssociatedChansHaveConflictingProps"; break;
    case -201466: return "DAQmxErrorAutoConfigBetweenMultipleDeviceStatesInvalid"; break;
    case -201465: return "DAQmxErrorAutoConfigOfOfflineDevicesInvalid"; break;
    case -201464: return "DAQmxErrorExternalFIFOFault"; break;
    case -201463: return "DAQmxErrorConnectionsNotReciprocal"; break;
    case -201462: return "DAQmxErrorInvalidOutputToInputCDAQSyncConnection"; break;
    case -201461: return "DAQmxErrorReferenceClockNotPresent"; break;
    case -201460: return "DAQmxErrorBlankStringExpansionFoundNoSupportedCDAQSyncConnectionDevices"; break;
    case -201459: return "DAQmxErrorNoDevicesSupportCDAQSyncConnections"; break;
    case -201458: return "DAQmxErrorInvalidCDAQSyncTimeoutValue"; break;
    case -201457: return "DAQmxErrorCDAQSyncConnectionToSamePort"; break;
    case -201456: return "DAQmxErrorDevsWithoutCommonSyncConnectionStrategy"; break;
    case -201455: return "DAQmxErrorNoCDAQSyncBetweenPhysAndSimulatedDevs"; break;
    case -201454: return "DAQmxErrorUnableToContainCards"; break;
    case -201453: return "DAQmxErrorFindDisconnectedBetweenPhysAndSimDeviceStatesInvalid"; break;
    case -201452: return "DAQmxErrorOperationAborted"; break;
    case -201451: return "DAQmxErrorTwoPortsRequired"; break;
    case -201450: return "DAQmxErrorDeviceDoesNotSupportCDAQSyncConnections"; break;
    case -201449: return "DAQmxErrorInvalidcDAQSyncPortConnectionFormat"; break;
    case -201448: return "DAQmxErrorRosetteMeasurementsNotSpecified"; break;
    case -201447: return "DAQmxErrorInvalidNumOfPhysChansForDeltaRosette"; break;
    case -201446: return "DAQmxErrorInvalidNumOfPhysChansForTeeRosette"; break;
    case -201445: return "DAQmxErrorRosetteStrainChanNamesNeeded"; break;
    case -201444: return "DAQmxErrorMultideviceWithOnDemandTiming"; break;
    case -201443: return "DAQmxErrorFREQOUTCannotProduceDesiredFrequency3"; break;
    case -201442: return "DAQmxErrorTwoEdgeSeparationSameTerminalSameEdge"; break;
    case -201441: return "DAQmxErrorDontMixSyncPulseAndSampClkTimebaseOn449x"; break;
    case -201440: return "DAQmxErrorNeitherRefClkNorSampClkTimebaseConfiguredForDSASync"; break;
    case -201439: return "DAQmxErrorRetriggeringFiniteCONotAllowed"; break;
    case -201438: return "DAQmxErrorDeviceRebootedFromWDTTimeout"; break;
    case -201437: return "DAQmxErrorTimeoutValueExceedsMaximum"; break;
    case -201436: return "DAQmxErrorSharingDifferentWireModes"; break;
    case -201435: return "DAQmxErrorCantPrimeWithEmptyBuffer"; break;
    case -201434: return "DAQmxErrorConfigFailedBecauseWatchdogExpired"; break;
    case -201433: return "DAQmxErrorWriteFailedBecauseWatchdogChangedLineDirection"; break;
    case -201432: return "DAQmxErrorMultipleSubsytemCalibration"; break;
    case -201431: return "DAQmxErrorIncorrectChannelForOffsetAdjustment"; break;
    case -201430: return "DAQmxErrorInvalidNumRefVoltagesToWrite"; break;
    case -201429: return "DAQmxErrorStartTrigDelayWithDSAModule"; break;
    case -201428: return "DAQmxErrorMoreThanOneSyncPulseDetected"; break;
    case -201427: return "DAQmxErrorDevNotSupportedWithinDAQmxAPI"; break;
    case -201426: return "DAQmxErrorDevsWithoutSyncStrategies"; break;
    case -201425: return "DAQmxErrorDevsWithoutCommonSyncStrategy"; break;
    case -201424: return "DAQmxErrorSyncStrategiesCannotSync"; break;
    case -201423: return "DAQmxErrorChassisCommunicationInterrupted"; break;
    case -201422: return "DAQmxErrorUnknownCardPowerProfileInCarrier"; break;
    case -201421: return "DAQmxErrorAttrNotSupportedOnAccessory"; break;
    case -201420: return "DAQmxErrorNetworkDeviceReservedByAnotherHost"; break;
    case -201419: return "DAQmxErrorIncorrectFirmwareFileUploaded"; break;
    case -201418: return "DAQmxErrorInvalidFirmwareFileUploaded"; break;
    case -201417: return "DAQmxErrorInTimerTimeoutOnArm"; break;
    case -201416: return "DAQmxErrorCantExceedSlotRelayDriveLimit"; break;
    case -201415: return "DAQmxErrorModuleUnsupportedFor9163"; break;
    case -201414: return "DAQmxErrorConnectionsNotSupported"; break;
    case -201413: return "DAQmxErrorAccessoryNotPresent"; break;
    case -201412: return "DAQmxErrorSpecifiedAccessoryChannelsNotPresentOnDevice"; break;
    case -201411: return "DAQmxErrorConnectionsNotSupportedOnAccessory"; break;
    case -201410: return "DAQmxErrorRateTooFastForHWTSP"; break;
    case -201409: return "DAQmxErrorDelayFromSampleClockOutOfRangeForHWTSP"; break;
    case -201408: return "DAQmxErrorAveragingWhenNotInternalHWTSP"; break;
    case -201407: return "DAQmxErrorAttributeNotSupportedUnlessHWTSP"; break;
    case -201406: return "DAQmxErrorFiveVoltDetectFailed"; break;
    case -201405: return "DAQmxErrorAnalogBusStateInconsistent"; break;
    case -201404: return "DAQmxErrorCardDetectedDoesNotMatchExpectedCard"; break;
    case -201403: return "DAQmxErrorLoggingStartNewFileNotCalled"; break;
    case -201402: return "DAQmxErrorLoggingSampsPerFileNotDivisible"; break;
    case -201401: return "DAQmxErrorRetrievingNetworkDeviceProperties"; break;
    case -201400: return "DAQmxErrorFilePreallocationFailed"; break;
    case -201399: return "DAQmxErrorModuleMismatchInSameTimedTask"; break;
    case -201398: return "DAQmxErrorInvalidAttributeValuePossiblyDueToOtherAttributeValues"; break;
    case -201397: return "DAQmxErrorChangeDetectionStoppedToPreventDeviceHang"; break;
    case -201396: return "DAQmxErrorFilterDelayRemovalNotPosssibleWithAnalogTrigger"; break;
    case -201395: return "DAQmxErrorNonbufferedOrNoChannels"; break;
    case -201394: return "DAQmxErrorTristateLogicLevelNotSpecdForEntirePort"; break;
    case -201393: return "DAQmxErrorTristateLogicLevelNotSupportedOnDigOutChan"; break;
    case -201392: return "DAQmxErrorTristateLogicLevelNotSupported"; break;
    case -201391: return "DAQmxErrorIncompleteGainAndCouplingCalAdjustment"; break;
    case -201390: return "DAQmxErrorNetworkStatusConnectionLost"; break;
    case -201389: return "DAQmxErrorModuleChangeDuringConnectionLoss"; break;
    case -201388: return "DAQmxErrorNetworkDeviceNotReservedByHost"; break;
    case -201387: return "DAQmxErrorDuplicateCalibrationAdjustmentInput"; break;
    case -201386: return "DAQmxErrorSelfCalFailedContactTechSupport"; break;
    case -201385: return "DAQmxErrorSelfCalFailedToConverge"; break;
    case -201384: return "DAQmxErrorUnsupportedSimulatedModuleForSimulatedChassis"; break;
    case -201383: return "DAQmxErrorLoggingWriteSizeTooBig"; break;
    case -201382: return "DAQmxErrorLoggingWriteSizeNotDivisible"; break;
    case -201381: return "DAQmxErrorMyDAQPowerRailFault"; break;
    case -201380: return "DAQmxErrorDeviceDoesNotSupportThisOperation"; break;
    case -201379: return "DAQmxErrorNetworkDevicesNotSupportedOnThisPlatform"; break;
    case -201378: return "DAQmxErrorUnknownFirmwareVersion"; break;
    case -201377: return "DAQmxErrorFirmwareIsUpdating"; break;
    case -201376: return "DAQmxErrorAccessoryEEPROMIsCorrupt"; break;
    case -201375: return "DAQmxErrorThrmcplLeadOffsetNullingCalNotSupported"; break;
    case -201374: return "DAQmxErrorSelfCalFailedTryExtCal"; break;
    case -201373: return "DAQmxErrorOutputP2PNotSupportedWithMultithreadedScripts"; break;
    case -201372: return "DAQmxErrorThrmcplCalibrationChannelsOpen"; break;
    case -201371: return "DAQmxErrorMDNSServiceInstanceAlreadyInUse"; break;
    case -201370: return "DAQmxErrorIPAddressAlreadyInUse"; break;
    case -201369: return "DAQmxErrorHostnameAlreadyInUse"; break;
    case -201368: return "DAQmxErrorInvalidNumberOfCalAdjustmentPoints"; break;
    case -201367: return "DAQmxErrorFilterOrDigitalSyncInternalSignal"; break;
    case -201366: return "DAQmxErrorBadDDSSource"; break;
    case -201365: return "DAQmxErrorOnboardRegenWithMoreThan16Channels"; break;
    case -201364: return "DAQmxErrorTriggerTooFast"; break;
    case -201363: return "DAQmxErrorMinMaxOutsideTableRange"; break;
    case -201362: return "DAQmxErrorChannelExpansionWithInvalidAnalogTriggerDevice"; break;
    case -201361: return "DAQmxErrorSyncPulseSrcInvalidForTask"; break;
    case -201360: return "DAQmxErrorInvalidCarrierSlotNumberSpecd"; break;
    case -201359: return "DAQmxErrorCardsMustBeInSameCarrier"; break;
    case -201358: return "DAQmxErrorCardDevCarrierSimMustMatch"; break;
    case -201357: return "DAQmxErrorDevMustHaveAtLeastOneCard"; break;
    case -201356: return "DAQmxErrorCardTopologyError"; break;
    case -201355: return "DAQmxErrorExceededCarrierPowerLimit"; break;
    case -201354: return "DAQmxErrorCardsIncompatible"; break;
    case -201353: return "DAQmxErrorAnalogBusNotValid"; break;
    case -201352: return "DAQmxErrorReservationConflict"; break;
    case -201351: return "DAQmxErrorMemMappedOnDemandNotSupported"; break;
    case -201350: return "DAQmxErrorSlaveWithNoStartTriggerConfigured"; break;
    case -201349: return "DAQmxErrorChannelExpansionWithDifferentTriggerDevices"; break;
    case -201348: return "DAQmxErrorCounterSyncAndRetriggered"; break;
    case -201347: return "DAQmxErrorNoExternalSyncPulseDetected"; break;
    case -201346: return "DAQmxErrorSlaveAndNoExternalSyncPulse"; break;
    case -201345: return "DAQmxErrorCustomTimingRequiredForAttribute"; break;
    case -201344: return "DAQmxErrorCustomTimingModeNotSet"; break;
    case -201343: return "DAQmxErrorAccessoryPowerTripped"; break;
    case -201342: return "DAQmxErrorUnsupportedAccessory"; break;
    case -201341: return "DAQmxErrorInvalidAccessoryChange"; break;
    case -201340: return "DAQmxErrorFirmwareRequiresUpgrade"; break;
    case -201339: return "DAQmxErrorFastExternalTimebaseNotSupportedForDevice"; break;
    case -201338: return "DAQmxErrorInvalidShuntLocationForCalibration"; break;
    case -201337: return "DAQmxErrorDeviceNameTooLong"; break;
    case -201336: return "DAQmxErrorBridgeScalesUnsupported"; break;
    case -201335: return "DAQmxErrorMismatchedElecPhysValues"; break;
    case -201334: return "DAQmxErrorLinearRequiresUniquePoints"; break;
    case -201333: return "DAQmxErrorMissingRequiredScalingParameter"; break;
    case -201332: return "DAQmxErrorLoggingNotSupportOnOutputTasks"; break;
    case -201331: return "DAQmxErrorMemoryMappedHardwareTimedNonBufferedUnsupported"; break;
    case -201330: return "DAQmxErrorCannotUpdatePulseTrainWithAutoIncrementEnabled"; break;
    case -201329: return "DAQmxErrorHWTimedSinglePointAndDataXferNotDMA"; break;
    case -201328: return "DAQmxErrorSCCSecondStageEmpty"; break;
    case -201327: return "DAQmxErrorSCCInvalidDualStageCombo"; break;
    case -201326: return "DAQmxErrorSCCInvalidSecondStage"; break;
    case -201325: return "DAQmxErrorSCCInvalidFirstStage"; break;
    case -201324: return "DAQmxErrorCounterMultipleSampleClockedChannels"; break;
    case -201323: return "DAQmxError2CounterMeasurementModeAndSampleClocked"; break;
    case -201322: return "DAQmxErrorCantHaveBothMemMappedAndNonMemMappedTasks"; break;
    case -201321: return "DAQmxErrorMemMappedDataReadByAnotherProcess"; break;
    case -201320: return "DAQmxErrorRetriggeringInvalidForGivenSettings"; break;
    case -201319: return "DAQmxErrorAIOverrun"; break;
    case -201318: return "DAQmxErrorCOOverrun"; break;
    case -201317: return "DAQmxErrorCounterMultipleBufferedChannels"; break;
    case -201316: return "DAQmxErrorInvalidTimebaseForCOHWTSP"; break;
    case -201315: return "DAQmxErrorWriteBeforeEvent"; break;
    case -201314: return "DAQmxErrorCIOverrun"; break;
    case -201313: return "DAQmxErrorCounterNonResponsiveAndReset"; break;
    case -201312: return "DAQmxErrorMeasTypeOrChannelNotSupportedForLogging"; break;
    case -201311: return "DAQmxErrorFileAlreadyOpenedForWrite"; break;
    case -201310: return "DAQmxErrorTdmsNotFound"; break;
    case -201309: return "DAQmxErrorGenericFileIO"; break;
    case -201308: return "DAQmxErrorFiniteSTCCounterNotSupportedForLogging"; break;
    case -201307: return "DAQmxErrorMeasurementTypeNotSupportedForLogging"; break;
    case -201306: return "DAQmxErrorFileAlreadyOpened"; break;
    case -201305: return "DAQmxErrorDiskFull"; break;
    case -201304: return "DAQmxErrorFilePathInvalid"; break;
    case -201303: return "DAQmxErrorFileVersionMismatch"; break;
    case -201302: return "DAQmxErrorFileWriteProtected"; break;
    case -201301: return "DAQmxErrorReadNotSupportedForLoggingMode"; break;
    case -201300: return "DAQmxErrorAttributeNotSupportedWhenLogging"; break;
    case -201299: return "DAQmxErrorLoggingModeNotSupportedNonBuffered"; break;
    case -201298: return "DAQmxErrorPropertyNotSupportedWithConflictingProperty"; break;
    case -201297: return "DAQmxErrorParallelSSHOnConnector1"; break;
    case -201296: return "DAQmxErrorCOOnlyImplicitSampleTimingTypeSupported"; break;
    case -201295: return "DAQmxErrorCalibrationFailedAOOutOfRange"; break;
    case -201294: return "DAQmxErrorCalibrationFailedAIOutOfRange"; break;
    case -201293: return "DAQmxErrorCalPWMLinearityFailed"; break;
    case -201292: return "DAQmxErrorOverrunUnderflowConfigurationCombo"; break;
    case -201291: return "DAQmxErrorCannotWriteToFiniteCOTask"; break;
    case -201290: return "DAQmxErrorNetworkDAQInvalidWEPKeyLength"; break;
    case -201289: return "DAQmxErrorCalInputsShortedNotSupported"; break;
    case -201288: return "DAQmxErrorCannotSetPropertyWhenTaskIsReserved"; break;
    case -201287: return "DAQmxErrorMinus12VFuseBlown"; break;
    case -201286: return "DAQmxErrorPlus12VFuseBlown"; break;
    case -201285: return "DAQmxErrorPlus5VFuseBlown"; break;
    case -201284: return "DAQmxErrorPlus3VFuseBlown"; break;
    case -201283: return "DAQmxErrorDeviceSerialPortError"; break;
    case -201282: return "DAQmxErrorPowerUpStateMachineNotDone"; break;
    case -201281: return "DAQmxErrorTooManyTriggersSpecifiedInTask"; break;
    case -201280: return "DAQmxErrorVerticalOffsetNotSupportedOnDevice"; break;
    case -201279: return "DAQmxErrorInvalidCouplingForMeasurementType"; break;
    case -201278: return "DAQmxErrorDigitalLineUpdateTooFastForDevice"; break;
    case -201277: return "DAQmxErrorCertificateIsTooBigToTransfer"; break;
    case -201276: return "DAQmxErrorOnlyPEMOrDERCertiticatesAccepted"; break;
    case -201275: return "DAQmxErrorCalCouplingNotSupported"; break;
    case -201274: return "DAQmxErrorDeviceNotSupportedIn64Bit"; break;
    case -201273: return "DAQmxErrorNetworkDeviceInUse"; break;
    case -201272: return "DAQmxErrorInvalidIPv4AddressFormat"; break;
    case -201271: return "DAQmxErrorNetworkProductTypeMismatch"; break;
    case -201270: return "DAQmxErrorOnlyPEMCertificatesAccepted"; break;
    case -201269: return "DAQmxErrorCalibrationRequiresPrototypingBoardEnabled"; break;
    case -201268: return "DAQmxErrorAllCurrentLimitingResourcesAlreadyTaken"; break;
    case -201267: return "DAQmxErrorUserDefInfoStringBadLength"; break;
    case -201266: return "DAQmxErrorPropertyNotFound"; break;
    case -201265: return "DAQmxErrorOverVoltageProtectionActivated"; break;
    case -201264: return "DAQmxErrorScaledIQWaveformTooLarge"; break;
    case -201263: return "DAQmxErrorFirmwareFailedToDownload"; break;
    case -201262: return "DAQmxErrorPropertyNotSupportedForBusType"; break;
    case -201261: return "DAQmxErrorChangeRateWhileRunningCouldNotBeCompleted"; break;
    case -201260: return "DAQmxErrorCannotQueryManualControlAttribute"; break;
    case -201259: return "DAQmxErrorInvalidNetworkConfiguration"; break;
    case -201258: return "DAQmxErrorInvalidWirelessConfiguration"; break;
    case -201257: return "DAQmxErrorInvalidWirelessCountryCode"; break;
    case -201256: return "DAQmxErrorInvalidWirelessChannel"; break;
    case -201255: return "DAQmxErrorNetworkEEPROMHasChanged"; break;
    case -201254: return "DAQmxErrorNetworkSerialNumberMismatch"; break;
    case -201253: return "DAQmxErrorNetworkStatusDown"; break;
    case -201252: return "DAQmxErrorNetworkTargetUnreachable"; break;
    case -201251: return "DAQmxErrorNetworkTargetNotFound"; break;
    case -201250: return "DAQmxErrorNetworkStatusTimedOut"; break;
    case -201249: return "DAQmxErrorInvalidWirelessSecuritySelection"; break;
    case -201248: return "DAQmxErrorNetworkDeviceConfigurationLocked"; break;
    case -201247: return "DAQmxErrorNetworkDAQDeviceNotSupported"; break;
    case -201246: return "DAQmxErrorNetworkDAQCannotCreateEmptySleeve"; break;
    case -201245: return "DAQmxErrorUserDefInfoStringTooLong"; break;
    case -201244: return "DAQmxErrorModuleTypeDoesNotMatchModuleTypeInDestination"; break;
    case -201243: return "DAQmxErrorInvalidTEDSInterfaceAddress"; break;
    case -201242: return "DAQmxErrorDevDoesNotSupportSCXIComm"; break;
    case -201241: return "DAQmxErrorSCXICommDevConnector0MustBeCabledToModule"; break;
    case -201240: return "DAQmxErrorSCXIModuleDoesNotSupportDigitizationMode"; break;
    case -201239: return "DAQmxErrorDevDoesNotSupportMultiplexedSCXIDigitizationMode"; break;
    case -201238: return "DAQmxErrorDevOrDevPhysChanDoesNotSupportSCXIDigitization"; break;
    case -201237: return "DAQmxErrorInvalidPhysChanName"; break;
    case -201236: return "DAQmxErrorSCXIChassisCommModeInvalid"; break;
    case -201235: return "DAQmxErrorRequiredDependencyNotFound"; break;
    case -201234: return "DAQmxErrorInvalidStorage"; break;
    case -201233: return "DAQmxErrorInvalidObject"; break;
    case -201232: return "DAQmxErrorStorageAlteredPriorToSave"; break;
    case -201231: return "DAQmxErrorTaskDoesNotReferenceLocalChannel"; break;
    case -201230: return "DAQmxErrorReferencedDevSimMustMatchTarget"; break;
    case -201229: return "DAQmxErrorProgrammedIOFailsBecauseOfWatchdogTimer"; break;
    case -201228: return "DAQmxErrorWatchdogTimerFailsBecauseOfProgrammedIO"; break;
    case -201227: return "DAQmxErrorCantUseThisTimingEngineWithAPort"; break;
    case -201226: return "DAQmxErrorProgrammedIOConflict"; break;
    case -201225: return "DAQmxErrorChangeDetectionIncompatibleWithProgrammedIO"; break;
    case -201224: return "DAQmxErrorTristateNotEnoughLines"; break;
    case -201223: return "DAQmxErrorTristateConflict"; break;
    case -201222: return "DAQmxErrorGenerateOrFiniteWaitExpectedBeforeBreakBlock"; break;
    case -201221: return "DAQmxErrorBreakBlockNotAllowedInLoop"; break;
    case -201220: return "DAQmxErrorClearTriggerNotAllowedInBreakBlock"; break;
    case -201219: return "DAQmxErrorNestingNotAllowedInBreakBlock"; break;
    case -201218: return "DAQmxErrorIfElseBlockNotAllowedInBreakBlock"; break;
    case -201217: return "DAQmxErrorRepeatUntilTriggerLoopNotAllowedInBreakBlock"; break;
    case -201216: return "DAQmxErrorWaitUntilTriggerNotAllowedInBreakBlock"; break;
    case -201215: return "DAQmxErrorMarkerPosInvalidInBreakBlock"; break;
    case -201214: return "DAQmxErrorInvalidWaitDurationInBreakBlock"; break;
    case -201213: return "DAQmxErrorInvalidSubsetLengthInBreakBlock"; break;
    case -201212: return "DAQmxErrorInvalidWaveformLengthInBreakBlock"; break;
    case -201211: return "DAQmxErrorInvalidWaitDurationBeforeBreakBlock"; break;
    case -201210: return "DAQmxErrorInvalidSubsetLengthBeforeBreakBlock"; break;
    case -201209: return "DAQmxErrorInvalidWaveformLengthBeforeBreakBlock"; break;
    case -201208: return "DAQmxErrorSampleRateTooHighForADCTimingMode"; break;
    case -201207: return "DAQmxErrorActiveDevNotSupportedWithMultiDevTask"; break;
    case -201206: return "DAQmxErrorRealDevAndSimDevNotSupportedInSameTask"; break;
    case -201205: return "DAQmxErrorRTSISimMustMatchDevSim"; break;
    case -201204: return "DAQmxErrorBridgeShuntCaNotSupported"; break;
    case -201203: return "DAQmxErrorStrainShuntCaNotSupported"; break;
    case -201202: return "DAQmxErrorGainTooLargeForGainCalConst"; break;
    case -201201: return "DAQmxErrorOffsetTooLargeForOffsetCalConst"; break;
    case -201200: return "DAQmxErrorElvisPrototypingBoardRemoved"; break;
    case -201199: return "DAQmxErrorElvis2PowerRailFault"; break;
    case -201198: return "DAQmxErrorElvis2PhysicalChansFault"; break;
    case -201197: return "DAQmxErrorElvis2PhysicalChansThermalEvent"; break;
    case -201196: return "DAQmxErrorRXBitErrorRateLimitExceeded"; break;
    case -201195: return "DAQmxErrorPHYBitErrorRateLimitExceeded"; break;
    case -201194: return "DAQmxErrorTwoPartAttributeCalledOutOfOrder"; break;
    case -201193: return "DAQmxErrorInvalidSCXIChassisAddress"; break;
    case -201192: return "DAQmxErrorCouldNotConnectToRemoteMXS"; break;
    case -201191: return "DAQmxErrorExcitationStateRequiredForAttributes"; break;
    case -201190: return "DAQmxErrorDeviceNotUsableUntilUSBReplug"; break;
    case -201189: return "DAQmxErrorInputFIFOOverflowDuringCalibrationOnFullSpeedUSB"; break;
    case -201188: return "DAQmxErrorInputFIFOOverflowDuringCalibration"; break;
    case -201187: return "DAQmxErrorCJCChanConflictsWithNonThermocoupleChan"; break;
    case -201186: return "DAQmxErrorCommDeviceForPXIBackplaneNotInRightmostSlot"; break;
    case -201185: return "DAQmxErrorCommDeviceForPXIBackplaneNotInSameChassis"; break;
    case -201184: return "DAQmxErrorCommDeviceForPXIBackplaneNotPXI"; break;
    case -201183: return "DAQmxErrorInvalidCalExcitFrequency"; break;
    case -201182: return "DAQmxErrorInvalidCalExcitVoltage"; break;
    case -201181: return "DAQmxErrorInvalidAIInputSrc"; break;
    case -201180: return "DAQmxErrorInvalidCalInputRef"; break;
    case -201179: return "DAQmxErrordBReferenceValueNotGreaterThanZero"; break;
    case -201178: return "DAQmxErrorSampleClockRateIsTooFastForSampleClockTiming"; break;
    case -201177: return "DAQmxErrorDeviceNotUsableUntilColdStart"; break;
    case -201176: return "DAQmxErrorSampleClockRateIsTooFastForBurstTiming"; break;
    case -201175: return "DAQmxErrorDevImportFailedAssociatedResourceIDsNotSupported"; break;
    case -201174: return "DAQmxErrorSCXI1600ImportNotSupported"; break;
    case -201173: return "DAQmxErrorPowerSupplyConfigurationFailed"; break;
    case -201172: return "DAQmxErrorIEPEWithDCNotAllowed"; break;
    case -201171: return "DAQmxErrorMinTempForThermocoupleTypeOutsideAccuracyForPolyScaling"; break;
    case -201170: return "DAQmxErrorDevImportFailedNoDeviceToOverwriteAndSimulationNotSupported"; break;
    case -201169: return "DAQmxErrorDevImportFailedDeviceNotSupportedOnDestination"; break;
    case -201168: return "DAQmxErrorFirmwareIsTooOld"; break;
    case -201167: return "DAQmxErrorFirmwareCouldntUpdate"; break;
    case -201166: return "DAQmxErrorFirmwareIsCorrupt"; break;
    case -201165: return "DAQmxErrorFirmwareTooNew"; break;
    case -201164: return "DAQmxErrorSampClockCannotBeExportedFromExternalSampClockSrc"; break;
    case -201163: return "DAQmxErrorPhysChanReservedForInputWhenDesiredForOutput"; break;
    case -201162: return "DAQmxErrorPhysChanReservedForOutputWhenDesiredForInput"; break;
    case -201161: return "DAQmxErrorSpecifiedCDAQSlotNotEmpty"; break;
    case -201160: return "DAQmxErrorDeviceDoesNotSupportSimulation"; break;
    case -201159: return "DAQmxErrorInvalidCDAQSlotNumberSpecd"; break;
    case -201158: return "DAQmxErrorCSeriesModSimMustMatchCDAQChassisSim"; break;
    case -201157: return "DAQmxErrorSCCCabledDevMustNotBeSimWhenSCCCarrierIsNotSim"; break;
    case -201156: return "DAQmxErrorSCCModSimMustMatchSCCCarrierSim"; break;
    case -201155: return "DAQmxErrorSCXIModuleDoesNotSupportSimulation"; break;
    case -201154: return "DAQmxErrorSCXICableDevMustNotBeSimWhenModIsNotSim"; break;
    case -201153: return "DAQmxErrorSCXIDigitizerSimMustNotBeSimWhenModIsNotSim"; break;
    case -201152: return "DAQmxErrorSCXIModSimMustMatchSCXIChassisSim"; break;
    case -201151: return "DAQmxErrorSimPXIDevReqSlotAndChassisSpecd"; break;
    case -201150: return "DAQmxErrorSimDevConflictWithRealDev"; break;
    case -201149: return "DAQmxErrorInsufficientDataForCalibration"; break;
    case -201148: return "DAQmxErrorTriggerChannelMustBeEnabled"; break;
    case -201147: return "DAQmxErrorCalibrationDataConflictCouldNotBeResolved"; break;
    case -201146: return "DAQmxErrorSoftwareTooNewForSelfCalibrationData"; break;
    case -201145: return "DAQmxErrorSoftwareTooNewForExtCalibrationData"; break;
    case -201144: return "DAQmxErrorSelfCalibrationDataTooNewForSoftware"; break;
    case -201143: return "DAQmxErrorExtCalibrationDataTooNewForSoftware"; break;
    case -201142: return "DAQmxErrorSoftwareTooNewForEEPROM"; break;
    case -201141: return "DAQmxErrorEEPROMTooNewForSoftware"; break;
    case -201140: return "DAQmxErrorSoftwareTooNewForHardware"; break;
    case -201139: return "DAQmxErrorHardwareTooNewForSoftware"; break;
    case -201138: return "DAQmxErrorTaskCannotRestartFirstSampNotAvailToGenerate"; break;
    case -201137: return "DAQmxErrorOnlyUseStartTrigSrcPrptyWithDevDataLines"; break;
    case -201136: return "DAQmxErrorOnlyUsePauseTrigSrcPrptyWithDevDataLines"; break;
    case -201135: return "DAQmxErrorOnlyUseRefTrigSrcPrptyWithDevDataLines"; break;
    case -201134: return "DAQmxErrorPauseTrigDigPatternSizeDoesNotMatchSrcSize"; break;
    case -201133: return "DAQmxErrorLineConflictCDAQ"; break;
    case -201132: return "DAQmxErrorCannotWriteBeyondFinalFiniteSample"; break;
    case -201131: return "DAQmxErrorRefAndStartTriggerSrcCantBeSame"; break;
    case -201130: return "DAQmxErrorMemMappingIncompatibleWithPhysChansInTask"; break;
    case -201129: return "DAQmxErrorOutputDriveTypeMemMappingConflict"; break;
    case -201128: return "DAQmxErrorCAPIDeviceIndexInvalid"; break;
    case -201127: return "DAQmxErrorRatiometricDevicesMustUseExcitationForScaling"; break;
    case -201126: return "DAQmxErrorPropertyRequiresPerDeviceCfg"; break;
    case -201125: return "DAQmxErrorAICouplingAndAIInputSourceConflict"; break;
    case -201124: return "DAQmxErrorOnlyOneTaskCanPerformDOMemoryMappingAtATime"; break;
    case -201123: return "DAQmxErrorTooManyChansForAnalogRefTrigCDAQ"; break;
    case -201122: return "DAQmxErrorSpecdPropertyValueIsIncompatibleWithSampleTimingType"; break;
    case -201121: return "DAQmxErrorCPUNotSupportedRequireSSE"; break;
    case -201120: return "DAQmxErrorSpecdPropertyValueIsIncompatibleWithSampleTimingResponseMode"; break;
    case -201119: return "DAQmxErrorConflictingNextWriteIsLastAndRegenModeProperties"; break;
    case -201118: return "DAQmxErrorMStudioOperationDoesNotSupportDeviceContext"; break;
    case -201117: return "DAQmxErrorPropertyValueInChannelExpansionContextInvalid"; break;
    case -201116: return "DAQmxErrorHWTimedNonBufferedAONotSupported"; break;
    case -201115: return "DAQmxErrorWaveformLengthNotMultOfQuantum"; break;
    case -201114: return "DAQmxErrorDSAExpansionMixedBoardsWrongOrderInPXIChassis"; break;
    case -201113: return "DAQmxErrorPowerLevelTooLowForOOK"; break;
    case -201112: return "DAQmxErrorDeviceComponentTestFailure"; break;
    case -201111: return "DAQmxErrorUserDefinedWfmWithOOKUnsupported"; break;
    case -201110: return "DAQmxErrorInvalidDigitalModulationUserDefinedWaveform"; break;
    case -201109: return "DAQmxErrorBothRefInAndRefOutEnabled"; break;
    case -201108: return "DAQmxErrorBothAnalogAndDigitalModulationEnabled"; break;
    case -201107: return "DAQmxErrorBufferedOpsNotSupportedInSpecdSlotForCDAQ"; break;
    case -201106: return "DAQmxErrorPhysChanNotSupportedInSpecdSlotForCDAQ"; break;
    case -201105: return "DAQmxErrorResourceReservedWithConflictingSettings"; break;
    case -201104: return "DAQmxErrorInconsistentAnalogTrigSettingsCDAQ"; break;
    case -201103: return "DAQmxErrorTooManyChansForAnalogPauseTrigCDAQ"; break;
    case -201102: return "DAQmxErrorAnalogTrigNotFirstInScanListCDAQ"; break;
    case -201101: return "DAQmxErrorTooManyChansGivenTimingType"; break;
    case -201100: return "DAQmxErrorSampClkTimebaseDivWithExtSampClk"; break;
    case -201099: return "DAQmxErrorCantSaveTaskWithPerDeviceTimingProperties"; break;
    case -201098: return "DAQmxErrorConflictingAutoZeroMode"; break;
    case -201097: return "DAQmxErrorSampClkRateNotSupportedWithEAREnabled"; break;
    case -201096: return "DAQmxErrorSampClkTimebaseRateNotSpecd"; break;
    case -201095: return "DAQmxErrorSessionCorruptedByDLLReload"; break;
    case -201094: return "DAQmxErrorActiveDevNotSupportedWithChanExpansion"; break;
    case -201093: return "DAQmxErrorSampClkRateInvalid"; break;
    case -201092: return "DAQmxErrorExtSyncPulseSrcCannotBeExported"; break;
    case -201091: return "DAQmxErrorSyncPulseMinDelayToStartNeededForExtSyncPulseSrc"; break;
    case -201090: return "DAQmxErrorSyncPulseSrcInvalid"; break;
    case -201089: return "DAQmxErrorSampClkTimebaseRateInvalid"; break;
    case -201088: return "DAQmxErrorSampClkTimebaseSrcInvalid"; break;
    case -201087: return "DAQmxErrorSampClkRateMustBeSpecd"; break;
    case -201086: return "DAQmxErrorInvalidAttributeName"; break;
    case -201085: return "DAQmxErrorCJCChanNameMustBeSetWhenCJCSrcIsScannableChan"; break;
    case -201084: return "DAQmxErrorHiddenChanMissingInChansPropertyInCfgFile"; break;
    case -201083: return "DAQmxErrorChanNamesNotSpecdInCfgFile"; break;
    case -201082: return "DAQmxErrorDuplicateHiddenChanNamesInCfgFile"; break;
    case -201081: return "DAQmxErrorDuplicateChanNameInCfgFile"; break;
    case -201080: return "DAQmxErrorInvalidSCCModuleForSlotSpecd"; break;
    case -201079: return "DAQmxErrorInvalidSCCSlotNumberSpecd"; break;
    case -201078: return "DAQmxErrorInvalidSectionIdentifier"; break;
    case -201077: return "DAQmxErrorInvalidSectionName"; break;
    case -201076: return "DAQmxErrorDAQmxVersionNotSupported"; break;
    case -201075: return "DAQmxErrorSWObjectsFoundInFile"; break;
    case -201074: return "DAQmxErrorHWObjectsFoundInFile"; break;
    case -201073: return "DAQmxErrorLocalChannelSpecdWithNoParentTask"; break;
    case -201072: return "DAQmxErrorTaskReferencesMissingLocalChannel"; break;
    case -201071: return "DAQmxErrorTaskReferencesLocalChannelFromOtherTask"; break;
    case -201070: return "DAQmxErrorTaskMissingChannelProperty"; break;
    case -201069: return "DAQmxErrorInvalidLocalChanName"; break;
    case -201068: return "DAQmxErrorInvalidEscapeCharacterInString"; break;
    case -201067: return "DAQmxErrorInvalidTableIdentifier"; break;
    case -201066: return "DAQmxErrorValueFoundInInvalidColumn"; break;
    case -201065: return "DAQmxErrorMissingStartOfTable"; break;
    case -201064: return "DAQmxErrorFileMissingRequiredDAQmxHeader"; break;
    case -201063: return "DAQmxErrorDeviceIDDoesNotMatch"; break;
    case -201062: return "DAQmxErrorBufferedOperationsNotSupportedOnSelectedLines"; break;
    case -201061: return "DAQmxErrorPropertyConflictsWithScale"; break;
    case -201060: return "DAQmxErrorInvalidINIFileSyntax"; break;
    case -201059: return "DAQmxErrorDeviceInfoFailedPXIChassisNotIdentified"; break;
    case -201058: return "DAQmxErrorInvalidHWProductNumber"; break;
    case -201057: return "DAQmxErrorInvalidHWProductType"; break;
    case -201056: return "DAQmxErrorInvalidNumericFormatSpecd"; break;
    case -201055: return "DAQmxErrorDuplicatePropertyInObject"; break;
    case -201054: return "DAQmxErrorInvalidEnumValueSpecd"; break;
    case -201053: return "DAQmxErrorTEDSSensorPhysicalChannelConflict"; break;
    case -201052: return "DAQmxErrorTooManyPhysicalChansForTEDSInterfaceSpecd"; break;
    case -201051: return "DAQmxErrorIncapableTEDSInterfaceControllingDeviceSpecd"; break;
    case -201050: return "DAQmxErrorSCCCarrierSpecdIsMissing"; break;
    case -201049: return "DAQmxErrorIncapableSCCDigitizingDeviceSpecd"; break;
    case -201048: return "DAQmxErrorAccessorySettingNotApplicable"; break;
    case -201047: return "DAQmxErrorDeviceAndConnectorSpecdAlreadyOccupied"; break;
    case -201046: return "DAQmxErrorIllegalAccessoryTypeForDeviceSpecd"; break;
    case -201045: return "DAQmxErrorInvalidDeviceConnectorNumberSpecd"; break;
    case -201044: return "DAQmxErrorInvalidAccessoryName"; break;
    case -201043: return "DAQmxErrorMoreThanOneMatchForSpecdDevice"; break;
    case -201042: return "DAQmxErrorNoMatchForSpecdDevice"; break;
    case -201041: return "DAQmxErrorProductTypeAndProductNumberConflict"; break;
    case -201040: return "DAQmxErrorExtraPropertyDetectedInSpecdObject"; break;
    case -201039: return "DAQmxErrorRequiredPropertyMissing"; break;
    case -201038: return "DAQmxErrorCantSetAuthorForLocalChan"; break;
    case -201037: return "DAQmxErrorInvalidTimeValue"; break;
    case -201036: return "DAQmxErrorInvalidTimeFormat"; break;
    case -201035: return "DAQmxErrorDigDevChansSpecdInModeOtherThanParallel"; break;
    case -201034: return "DAQmxErrorCascadeDigitizationModeNotSupported"; break;
    case -201033: return "DAQmxErrorSpecdSlotAlreadyOccupied"; break;
    case -201032: return "DAQmxErrorInvalidSCXISlotNumberSpecd"; break;
    case -201031: return "DAQmxErrorAddressAlreadyInUse"; break;
    case -201030: return "DAQmxErrorSpecdDeviceDoesNotSupportRTSI"; break;
    case -201029: return "DAQmxErrorSpecdDeviceIsAlreadyOnRTSIBus"; break;
    case -201028: return "DAQmxErrorIdentifierInUse"; break;
    case -201027: return "DAQmxErrorWaitForNextSampleClockOrReadDetected3OrMoreMissedSampClks"; break;
    case -201026: return "DAQmxErrorHWTimedAndDataXferPIO"; break;
    case -201025: return "DAQmxErrorNonBufferedAndHWTimed"; break;
    case -201024: return "DAQmxErrorCTROutSampClkPeriodShorterThanGenPulseTrainPeriodPolled"; break;
    case -201023: return "DAQmxErrorCTROutSampClkPeriodShorterThanGenPulseTrainPeriod2"; break;
    case -201022: return "DAQmxErrorCOCannotKeepUpInHWTimedSinglePointPolled"; break;
    case -201021: return "DAQmxErrorWriteRecoveryCannotKeepUpInHWTimedSinglePoint"; break;
    case -201020: return "DAQmxErrorNoChangeDetectionOnSelectedLineForDevice"; break;
    case -201019: return "DAQmxErrorSMIOPauseTriggersNotSupportedWithChannelExpansion"; break;
    case -201018: return "DAQmxErrorClockMasterForExternalClockNotLongestPipeline"; break;
    case -201017: return "DAQmxErrorUnsupportedUnicodeByteOrderMarker"; break;
    case -201016: return "DAQmxErrorTooManyInstructionsInLoopInScript"; break;
    case -201015: return "DAQmxErrorPLLNotLocked"; break;
    case -201014: return "DAQmxErrorIfElseBlockNotAllowedInFiniteRepeatLoopInScript"; break;
    case -201013: return "DAQmxErrorIfElseBlockNotAllowedInConditionalRepeatLoopInScript"; break;
    case -201012: return "DAQmxErrorClearIsLastInstructionInIfElseBlockInScript"; break;
    case -201011: return "DAQmxErrorInvalidWaitDurationBeforeIfElseBlockInScript"; break;
    case -201010: return "DAQmxErrorMarkerPosInvalidBeforeIfElseBlockInScript"; break;
    case -201009: return "DAQmxErrorInvalidSubsetLengthBeforeIfElseBlockInScript"; break;
    case -201008: return "DAQmxErrorInvalidWaveformLengthBeforeIfElseBlockInScript"; break;
    case -201007: return "DAQmxErrorGenerateOrFiniteWaitInstructionExpectedBeforeIfElseBlockInScript"; break;
    case -201006: return "DAQmxErrorCalPasswordNotSupported"; break;
    case -201005: return "DAQmxErrorSetupCalNeededBeforeAdjustCal"; break;
    case -201004: return "DAQmxErrorMultipleChansNotSupportedDuringCalSetup"; break;
    case -201003: return "DAQmxErrorDevCannotBeAccessed"; break;
    case -201002: return "DAQmxErrorSampClkRateDoesntMatchSampClkSrc"; break;
    case -201001: return "DAQmxErrorSampClkRateNotSupportedWithEARDisabled"; break;
    case -201000: return "DAQmxErrorLabVIEWVersionDoesntSupportDAQmxEvents"; break;
    case -200999: return "DAQmxErrorCOReadyForNewValNotSupportedWithOnDemand"; break;
    case -200998: return "DAQmxErrorCIHWTimedSinglePointNotSupportedForMeasType"; break;
    case -200997: return "DAQmxErrorOnDemandNotSupportedWithHWTimedSinglePoint"; break;
    case -200996: return "DAQmxErrorHWTimedSinglePointAndDataXferNotProgIO"; break;
    case -200995: return "DAQmxErrorMemMapAndHWTimedSinglePoint"; break;
    case -200994: return "DAQmxErrorCannotSetPropertyWhenHWTimedSinglePointTaskIsRunning"; break;
    case -200993: return "DAQmxErrorCTROutSampClkPeriodShorterThanGenPulseTrainPeriod"; break;
    case -200992: return "DAQmxErrorTooManyEventsGenerated"; break;
    case -200991: return "DAQmxErrorMStudioCppRemoveEventsBeforeStop"; break;
    case -200990: return "DAQmxErrorCAPICannotRegisterSyncEventsFromMultipleThreads"; break;
    case -200989: return "DAQmxErrorReadWaitNextSampClkWaitMismatchTwo"; break;
    case -200988: return "DAQmxErrorReadWaitNextSampClkWaitMismatchOne"; break;
    case -200987: return "DAQmxErrorDAQmxSignalEventTypeNotSupportedByChanTypesOrDevicesInTask"; break;
    case -200986: return "DAQmxErrorCannotUnregisterDAQmxSoftwareEventWhileTaskIsRunning"; break;
    case -200985: return "DAQmxErrorAutoStartWriteNotAllowedEventRegistered"; break;
    case -200984: return "DAQmxErrorAutoStartReadNotAllowedEventRegistered"; break;
    case -200983: return "DAQmxErrorCannotGetPropertyWhenTaskNotReservedCommittedOrRunning"; break;
    case -200982: return "DAQmxErrorSignalEventsNotSupportedByDevice"; break;
    case -200981: return "DAQmxErrorEveryNSamplesAcqIntoBufferEventNotSupportedByDevice"; break;
    case -200980: return "DAQmxErrorEveryNSampsTransferredFromBufferEventNotSupportedByDevice"; break;
    case -200979: return "DAQmxErrorCAPISyncEventsTaskStateChangeNotAllowedFromDifferentThread"; break;
    case -200978: return "DAQmxErrorDAQmxSWEventsWithDifferentCallMechanisms"; break;
    case -200977: return "DAQmxErrorCantSaveChanWithPolyCalScaleAndAllowInteractiveEdit"; break;
    case -200976: return "DAQmxErrorChanDoesNotSupportCJC"; break;
    case -200975: return "DAQmxErrorCOReadyForNewValNotSupportedWithHWTimedSinglePoint"; break;
    case -200974: return "DAQmxErrorDACAllowConnToGndNotSupportedByDevWhenRefSrcExt"; break;
    case -200973: return "DAQmxErrorCantGetPropertyTaskNotRunning"; break;
    case -200972: return "DAQmxErrorCantSetPropertyTaskNotRunning"; break;
    case -200971: return "DAQmxErrorCantSetPropertyTaskNotRunningCommitted"; break;
    case -200970: return "DAQmxErrorAIEveryNSampsEventIntervalNotMultipleOf2"; break;
    case -200969: return "DAQmxErrorInvalidTEDSPhysChanNotAI"; break;
    case -200968: return "DAQmxErrorCAPICannotPerformTaskOperationInAsyncCallback"; break;
    case -200967: return "DAQmxErrorEveryNSampsTransferredFromBufferEventAlreadyRegistered"; break;
    case -200966: return "DAQmxErrorEveryNSampsAcqIntoBufferEventAlreadyRegistered"; break;
    case -200965: return "DAQmxErrorEveryNSampsTransferredFromBufferNotForInput"; break;
    case -200964: return "DAQmxErrorEveryNSampsAcqIntoBufferNotForOutput"; break;
    case -200963: return "DAQmxErrorAOSampTimingTypeDifferentIn2Tasks"; break;
    case -200962: return "DAQmxErrorCouldNotDownloadFirmwareHWDamaged"; break;
    case -200961: return "DAQmxErrorCouldNotDownloadFirmwareFileMissingOrDamaged"; break;
    case -200960: return "DAQmxErrorCannotRegisterDAQmxSoftwareEventWhileTaskIsRunning"; break;
    case -200959: return "DAQmxErrorDifferentRawDataCompression"; break;
    case -200958: return "DAQmxErrorConfiguredTEDSInterfaceDevNotDetected"; break;
    case -200957: return "DAQmxErrorCompressedSampSizeExceedsResolution"; break;
    case -200956: return "DAQmxErrorChanDoesNotSupportCompression"; break;
    case -200955: return "DAQmxErrorDifferentRawDataFormats"; break;
    case -200954: return "DAQmxErrorSampClkOutputTermIncludesStartTrigSrc"; break;
    case -200953: return "DAQmxErrorStartTrigSrcEqualToSampClkSrc"; break;
    case -200952: return "DAQmxErrorEventOutputTermIncludesTrigSrc"; break;
    case -200951: return "DAQmxErrorCOMultipleWritesBetweenSampClks"; break;
    case -200950: return "DAQmxErrorDoneEventAlreadyRegistered"; break;
    case -200949: return "DAQmxErrorSignalEventAlreadyRegistered"; break;
    case -200948: return "DAQmxErrorCannotHaveTimedLoopAndDAQmxSignalEventsInSameTask"; break;
    case -200947: return "DAQmxErrorNeedLabVIEW711PatchToUseDAQmxEvents"; break;
    case -200946: return "DAQmxErrorStartFailedDueToWriteFailure"; break;
    case -200945: return "DAQmxErrorDataXferCustomThresholdNotDMAXferMethodSpecifiedForDev"; break;
    case -200944: return "DAQmxErrorDataXferRequestConditionNotSpecifiedForCustomThreshold"; break;
    case -200943: return "DAQmxErrorDataXferCustomThresholdNotSpecified"; break;
    case -200942: return "DAQmxErrorCAPISyncCallbackNotSupportedOnThisPlatform"; break;
    case -200941: return "DAQmxErrorCalChanReversePolyCoefNotSpecd"; break;
    case -200940: return "DAQmxErrorCalChanForwardPolyCoefNotSpecd"; break;
    case -200939: return "DAQmxErrorChanCalRepeatedNumberInPreScaledVals"; break;
    case -200938: return "DAQmxErrorChanCalTableNumScaledNotEqualNumPrescaledVals"; break;
    case -200937: return "DAQmxErrorChanCalTableScaledValsNotSpecd"; break;
    case -200936: return "DAQmxErrorChanCalTablePreScaledValsNotSpecd"; break;
    case -200935: return "DAQmxErrorChanCalScaleTypeNotSet"; break;
    case -200934: return "DAQmxErrorChanCalExpired"; break;
    case -200933: return "DAQmxErrorChanCalExpirationDateNotSet"; break;
    case -200932: return "DAQmxError3OutputPortCombinationGivenSampTimingType653x"; break;
    case -200931: return "DAQmxError3InputPortCombinationGivenSampTimingType653x"; break;
    case -200930: return "DAQmxError2OutputPortCombinationGivenSampTimingType653x"; break;
    case -200929: return "DAQmxError2InputPortCombinationGivenSampTimingType653x"; break;
    case -200928: return "DAQmxErrorPatternMatcherMayBeUsedByOneTrigOnly"; break;
    case -200927: return "DAQmxErrorNoChansSpecdForPatternSource"; break;
    case -200926: return "DAQmxErrorChangeDetectionChanNotInTask"; break;
    case -200925: return "DAQmxErrorChangeDetectionChanNotTristated"; break;
    case -200924: return "DAQmxErrorWaitModeValueNotSupportedNonBuffered"; break;
    case -200923: return "DAQmxErrorWaitModePropertyNotSupportedNonBuffered"; break;
    case -200922: return "DAQmxErrorCantSavePerLineConfigDigChanSoInteractiveEditsAllowed"; break;
    case -200921: return "DAQmxErrorCantSaveNonPortMultiLineDigChanSoInteractiveEditsAllowed"; break;
    case -200920: return "DAQmxErrorBufferSizeNotMultipleOfEveryNSampsEventIntervalNoIrqOnDev"; break;
    case -200919: return "DAQmxErrorGlobalTaskNameAlreadyChanName"; break;
    case -200918: return "DAQmxErrorGlobalChanNameAlreadyTaskName"; break;
    case -200917: return "DAQmxErrorAOEveryNSampsEventIntervalNotMultipleOf2"; break;
    case -200916: return "DAQmxErrorSampleTimebaseDivisorNotSupportedGivenTimingType"; break;
    case -200915: return "DAQmxErrorHandshakeEventOutputTermNotSupportedGivenTimingType"; break;
    case -200914: return "DAQmxErrorChangeDetectionOutputTermNotSupportedGivenTimingType"; break;
    case -200913: return "DAQmxErrorReadyForTransferOutputTermNotSupportedGivenTimingType"; break;
    case -200912: return "DAQmxErrorRefTrigOutputTermNotSupportedGivenTimingType"; break;
    case -200911: return "DAQmxErrorStartTrigOutputTermNotSupportedGivenTimingType"; break;
    case -200910: return "DAQmxErrorSampClockOutputTermNotSupportedGivenTimingType"; break;
    case -200909: return "DAQmxError20MhzTimebaseNotSupportedGivenTimingType"; break;
    case -200908: return "DAQmxErrorSampClockSourceNotSupportedGivenTimingType"; break;
    case -200907: return "DAQmxErrorRefTrigTypeNotSupportedGivenTimingType"; break;
    case -200906: return "DAQmxErrorPauseTrigTypeNotSupportedGivenTimingType"; break;
    case -200905: return "DAQmxErrorHandshakeTrigTypeNotSupportedGivenTimingType"; break;
    case -200904: return "DAQmxErrorStartTrigTypeNotSupportedGivenTimingType"; break;
    case -200903: return "DAQmxErrorRefClkSrcNotSupported"; break;
    case -200902: return "DAQmxErrorDataVoltageLowAndHighIncompatible"; break;
    case -200901: return "DAQmxErrorInvalidCharInDigPatternString"; break;
    case -200900: return "DAQmxErrorCantUsePort3AloneGivenSampTimingTypeOn653x"; break;
    case -200899: return "DAQmxErrorCantUsePort1AloneGivenSampTimingTypeOn653x"; break;
    case -200898: return "DAQmxErrorPartialUseOfPhysicalLinesWithinPortNotSupported653x"; break;
    case -200897: return "DAQmxErrorPhysicalChanNotSupportedGivenSampTimingType653x"; break;
    case -200896: return "DAQmxErrorCanExportOnlyDigEdgeTrigs"; break;
    case -200895: return "DAQmxErrorRefTrigDigPatternSizeDoesNotMatchSourceSize"; break;
    case -200894: return "DAQmxErrorStartTrigDigPatternSizeDoesNotMatchSourceSize"; break;
    case -200893: return "DAQmxErrorChangeDetectionRisingAndFallingEdgeChanDontMatch"; break;
    case -200892: return "DAQmxErrorPhysicalChansForChangeDetectionAndPatternMatch653x"; break;
    case -200891: return "DAQmxErrorCanExportOnlyOnboardSampClk"; break;
    case -200890: return "DAQmxErrorInternalSampClkNotRisingEdge"; break;
    case -200889: return "DAQmxErrorRefTrigDigPatternChanNotInTask"; break;
    case -200888: return "DAQmxErrorRefTrigDigPatternChanNotTristated"; break;
    case -200887: return "DAQmxErrorStartTrigDigPatternChanNotInTask"; break;
    case -200886: return "DAQmxErrorStartTrigDigPatternChanNotTristated"; break;
    case -200885: return "DAQmxErrorPXIStarAndClock10Sync"; break;
    case -200884: return "DAQmxErrorGlobalChanCannotBeSavedSoInteractiveEditsAllowed"; break;
    case -200883: return "DAQmxErrorTaskCannotBeSavedSoInteractiveEditsAllowed"; break;
    case -200882: return "DAQmxErrorInvalidGlobalChan"; break;
    case -200881: return "DAQmxErrorEveryNSampsEventAlreadyRegistered"; break;
    case -200880: return "DAQmxErrorEveryNSampsEventIntervalZeroNotSupported"; break;
    case -200879: return "DAQmxErrorChanSizeTooBigForU16PortWrite"; break;
    case -200878: return "DAQmxErrorChanSizeTooBigForU16PortRead"; break;
    case -200877: return "DAQmxErrorBufferSizeNotMultipleOfEveryNSampsEventIntervalWhenDMA"; break;
    case -200876: return "DAQmxErrorWriteWhenTaskNotRunningCOTicks"; break;
    case -200875: return "DAQmxErrorWriteWhenTaskNotRunningCOFreq"; break;
    case -200874: return "DAQmxErrorWriteWhenTaskNotRunningCOTime"; break;
    case -200873: return "DAQmxErrorAOMinMaxNotSupportedDACRangeTooSmall"; break;
    case -200872: return "DAQmxErrorAOMinMaxNotSupportedGivenDACRange"; break;
    case -200871: return "DAQmxErrorAOMinMaxNotSupportedGivenDACRangeAndOffsetVal"; break;
    case -200870: return "DAQmxErrorAOMinMaxNotSupportedDACOffsetValInappropriate"; break;
    case -200869: return "DAQmxErrorAOMinMaxNotSupportedGivenDACOffsetVal"; break;
    case -200868: return "DAQmxErrorAOMinMaxNotSupportedDACRefValTooSmall"; break;
    case -200867: return "DAQmxErrorAOMinMaxNotSupportedGivenDACRefVal"; break;
    case -200866: return "DAQmxErrorAOMinMaxNotSupportedGivenDACRefAndOffsetVal"; break;
    case -200865: return "DAQmxErrorWhenAcqCompAndNumSampsPerChanExceedsOnBrdBufSize"; break;
    case -200864: return "DAQmxErrorWhenAcqCompAndNoRefTrig"; break;
    case -200863: return "DAQmxErrorWaitForNextSampClkNotSupported"; break;
    case -200862: return "DAQmxErrorDevInUnidentifiedPXIChassis"; break;
    case -200861: return "DAQmxErrorMaxSoundPressureMicSensitivitRelatedAIPropertiesNotSupportedByDev"; break;
    case -200860: return "DAQmxErrorMaxSoundPressureAndMicSensitivityNotSupportedByDev"; break;
    case -200859: return "DAQmxErrorAOBufferSizeZeroForSampClkTimingType"; break;
    case -200858: return "DAQmxErrorAOCallWriteBeforeStartForSampClkTimingType"; break;
    case -200857: return "DAQmxErrorInvalidCalLowPassCutoffFreq"; break;
    case -200856: return "DAQmxErrorSimulationCannotBeDisabledForDevCreatedAsSimulatedDev"; break;
    case -200855: return "DAQmxErrorCannotAddNewDevsAfterTaskConfiguration"; break;
    case -200854: return "DAQmxErrorDifftSyncPulseSrcAndSampClkTimebaseSrcDevMultiDevTask"; break;
    case -200853: return "DAQmxErrorTermWithoutDevInMultiDevTask"; break;
    case -200852: return "DAQmxErrorSyncNoDevSampClkTimebaseOrSyncPulseInPXISlot2"; break;
    case -200851: return "DAQmxErrorPhysicalChanNotOnThisConnector"; break;
    case -200850: return "DAQmxErrorNumSampsToWaitNotGreaterThanZeroInScript"; break;
    case -200849: return "DAQmxErrorNumSampsToWaitNotMultipleOfAlignmentQuantumInScript"; break;
    case -200848: return "DAQmxErrorEveryNSamplesEventNotSupportedForNonBufferedTasks"; break;
    case -200847: return "DAQmxErrorBufferedAndDataXferPIO"; break;
    case -200846: return "DAQmxErrorCannotWriteWhenAutoStartFalseAndTaskNotRunning"; break;
    case -200845: return "DAQmxErrorNonBufferedAndDataXferInterrupts"; break;
    case -200844: return "DAQmxErrorWriteFailedMultipleCtrsWithFREQOUT"; break;
    case -200843: return "DAQmxErrorReadNotCompleteBefore3SampClkEdges"; break;
    case -200842: return "DAQmxErrorCtrHWTimedSinglePointAndDataXferNotProgIO"; break;
    case -200841: return "DAQmxErrorPrescalerNot1ForInputTerminal"; break;
    case -200840: return "DAQmxErrorPrescalerNot1ForTimebaseSrc"; break;
    case -200839: return "DAQmxErrorSampClkTimingTypeWhenTristateIsFalse"; break;
    case -200838: return "DAQmxErrorOutputBufferSizeNotMultOfXferSize"; break;
    case -200837: return "DAQmxErrorSampPerChanNotMultOfXferSize"; break;
    case -200836: return "DAQmxErrorWriteToTEDSFailed"; break;
    case -200835: return "DAQmxErrorSCXIDevNotUsablePowerTurnedOff"; break;
    case -200834: return "DAQmxErrorCannotReadWhenAutoStartFalseBufSizeZeroAndTaskNotRunning"; break;
    case -200833: return "DAQmxErrorCannotReadWhenAutoStartFalseHWTimedSinglePtAndTaskNotRunning"; break;
    case -200832: return "DAQmxErrorCannotReadWhenAutoStartFalseOnDemandAndTaskNotRunning"; break;
    case -200831: return "DAQmxErrorSimultaneousAOWhenNotOnDemandTiming"; break;
    case -200830: return "DAQmxErrorMemMapAndSimultaneousAO"; break;
    case -200829: return "DAQmxErrorWriteFailedMultipleCOOutputTypes"; break;
    case -200828: return "DAQmxErrorWriteToTEDSNotSupportedOnRT"; break;
    case -200827: return "DAQmxErrorVirtualTEDSDataFileError"; break;
    case -200826: return "DAQmxErrorTEDSSensorDataError"; break;
    case -200825: return "DAQmxErrorDataSizeMoreThanSizeOfEEPROMOnTEDS"; break;
    case -200824: return "DAQmxErrorPROMOnTEDSContainsBasicTEDSData"; break;
    case -200823: return "DAQmxErrorPROMOnTEDSAlreadyWritten"; break;
    case -200822: return "DAQmxErrorTEDSDoesNotContainPROM"; break;
    case -200821: return "DAQmxErrorHWTimedSinglePointNotSupportedAI"; break;
    case -200820: return "DAQmxErrorHWTimedSinglePointOddNumChansInAITask"; break;
    case -200819: return "DAQmxErrorCantUseOnlyOnBoardMemWithProgrammedIO"; break;
    case -200818: return "DAQmxErrorSwitchDevShutDownDueToHighTemp"; break;
    case -200817: return "DAQmxErrorExcitationNotSupportedWhenTermCfgDiff"; break;
    case -200816: return "DAQmxErrorTEDSMinElecValGEMaxElecVal"; break;
    case -200815: return "DAQmxErrorTEDSMinPhysValGEMaxPhysVal"; break;
    case -200814: return "DAQmxErrorCIOnboardClockNotSupportedAsInputTerm"; break;
    case -200813: return "DAQmxErrorInvalidSampModeForPositionMeas"; break;
    case -200812: return "DAQmxErrorTrigWhenAOHWTimedSinglePtSampMode"; break;
    case -200811: return "DAQmxErrorDAQmxCantUseStringDueToUnknownChar"; break;
    case -200810: return "DAQmxErrorDAQmxCantRetrieveStringDueToUnknownChar"; break;
    case -200809: return "DAQmxErrorClearTEDSNotSupportedOnRT"; break;
    case -200808: return "DAQmxErrorCfgTEDSNotSupportedOnRT"; break;
    case -200807: return "DAQmxErrorProgFilterClkCfgdToDifferentMinPulseWidthBySameTask1PerDev"; break;
    case -200806: return "DAQmxErrorProgFilterClkCfgdToDifferentMinPulseWidthByAnotherTask1PerDev"; break;
    case -200804: return "DAQmxErrorNoLastExtCalDateTimeLastExtCalNotDAQmx"; break;
    case -200803: return "DAQmxErrorCannotWriteNotStartedAutoStartFalseNotOnDemandHWTimedSglPt"; break;
    case -200802: return "DAQmxErrorCannotWriteNotStartedAutoStartFalseNotOnDemandBufSizeZero"; break;
    case -200801: return "DAQmxErrorCOInvalidTimingSrcDueToSignal"; break;
    case -200800: return "DAQmxErrorCIInvalidTimingSrcForSampClkDueToSampTimingType"; break;
    case -200799: return "DAQmxErrorCIInvalidTimingSrcForEventCntDueToSampMode"; break;
    case -200798: return "DAQmxErrorNoChangeDetectOnNonInputDigLineForDev"; break;
    case -200797: return "DAQmxErrorEmptyStringTermNameNotSupported"; break;
    case -200796: return "DAQmxErrorMemMapEnabledForHWTimedNonBufferedAO"; break;
    case -200795: return "DAQmxErrorDevOnboardMemOverflowDuringHWTimedNonBufferedGen"; break;
    case -200794: return "DAQmxErrorCODAQmxWriteMultipleChans"; break;
    case -200793: return "DAQmxErrorCantMaintainExistingValueAOSync"; break;
    case -200792: return "DAQmxErrorMStudioMultiplePhysChansNotSupported"; break;
    case -200791: return "DAQmxErrorCantConfigureTEDSForChan"; break;
    case -200790: return "DAQmxErrorWriteDataTypeTooSmall"; break;
    case -200789: return "DAQmxErrorReadDataTypeTooSmall"; break;
    case -200788: return "DAQmxErrorMeasuredBridgeOffsetTooHigh"; break;
    case -200787: return "DAQmxErrorStartTrigConflictWithCOHWTimedSinglePt"; break;
    case -200786: return "DAQmxErrorSampClkRateExtSampClkTimebaseRateMismatch"; break;
    case -200785: return "DAQmxErrorInvalidTimingSrcDueToSampTimingType"; break;
    case -200784: return "DAQmxErrorVirtualTEDSFileNotFound"; break;
    case -200783: return "DAQmxErrorMStudioNoForwardPolyScaleCoeffs"; break;
    case -200782: return "DAQmxErrorMStudioNoReversePolyScaleCoeffs"; break;
    case -200781: return "DAQmxErrorMStudioNoPolyScaleCoeffsUseCalc"; break;
    case -200780: return "DAQmxErrorMStudioNoForwardPolyScaleCoeffsUseCalc"; break;
    case -200779: return "DAQmxErrorMStudioNoReversePolyScaleCoeffsUseCalc"; break;
    case -200778: return "DAQmxErrorCOSampModeSampTimingTypeSampClkConflict"; break;
    case -200777: return "DAQmxErrorDevCannotProduceMinPulseWidth"; break;
    case -200776: return "DAQmxErrorCannotProduceMinPulseWidthGivenPropertyValues"; break;
    case -200775: return "DAQmxErrorTermCfgdToDifferentMinPulseWidthByAnotherTask"; break;
    case -200774: return "DAQmxErrorTermCfgdToDifferentMinPulseWidthByAnotherProperty"; break;
    case -200773: return "DAQmxErrorDigSyncNotAvailableOnTerm"; break;
    case -200772: return "DAQmxErrorDigFilterNotAvailableOnTerm"; break;
    case -200771: return "DAQmxErrorDigFilterEnabledMinPulseWidthNotCfg"; break;
    case -200770: return "DAQmxErrorDigFilterAndSyncBothEnabled"; break;
    case -200769: return "DAQmxErrorHWTimedSinglePointAOAndDataXferNotProgIO"; break;
    case -200768: return "DAQmxErrorNonBufferedAOAndDataXferNotProgIO"; break;
    case -200767: return "DAQmxErrorProgIODataXferForBufferedAO"; break;
    case -200766: return "DAQmxErrorTEDSLegacyTemplateIDInvalidOrUnsupported"; break;
    case -200765: return "DAQmxErrorTEDSMappingMethodInvalidOrUnsupported"; break;
    case -200764: return "DAQmxErrorTEDSLinearMappingSlopeZero"; break;
    case -200763: return "DAQmxErrorAIInputBufferSizeNotMultOfXferSize"; break;
    case -200762: return "DAQmxErrorNoSyncPulseExtSampClkTimebase"; break;
    case -200761: return "DAQmxErrorNoSyncPulseAnotherTaskRunning"; break;
    case -200760: return "DAQmxErrorAOMinMaxNotInGainRange"; break;
    case -200759: return "DAQmxErrorAOMinMaxNotInDACRange"; break;
    case -200758: return "DAQmxErrorDevOnlySupportsSampClkTimingAO"; break;
    case -200757: return "DAQmxErrorDevOnlySupportsSampClkTimingAI"; break;
    case -200756: return "DAQmxErrorTEDSIncompatibleSensorAndMeasType"; break;
    case -200755: return "DAQmxErrorTEDSMultipleCalTemplatesNotSupported"; break;
    case -200754: return "DAQmxErrorTEDSTemplateParametersNotSupported"; break;
    case -200753: return "DAQmxErrorParsingTEDSData"; break;
    case -200752: return "DAQmxErrorMultipleActivePhysChansNotSupported"; break;
    case -200751: return "DAQmxErrorNoChansSpecdForChangeDetect"; break;
    case -200750: return "DAQmxErrorInvalidCalVoltageForGivenGain"; break;
    case -200749: return "DAQmxErrorInvalidCalGain"; break;
    case -200748: return "DAQmxErrorMultipleWritesBetweenSampClks"; break;
    case -200747: return "DAQmxErrorInvalidAcqTypeForFREQOUT"; break;
    case -200746: return "DAQmxErrorSuitableTimebaseNotFoundTimeCombo2"; break;
    case -200745: return "DAQmxErrorSuitableTimebaseNotFoundFrequencyCombo2"; break;
    case -200744: return "DAQmxErrorRefClkRateRefClkSrcMismatch"; break;
    case -200743: return "DAQmxErrorNoTEDSTerminalBlock"; break;
    case -200742: return "DAQmxErrorCorruptedTEDSMemory"; break;
    case -200741: return "DAQmxErrorTEDSNotSupported"; break;
    case -200740: return "DAQmxErrorTimingSrcTaskStartedBeforeTimedLoop"; break;
    case -200739: return "DAQmxErrorPropertyNotSupportedForTimingSrc"; break;
    case -200738: return "DAQmxErrorTimingSrcDoesNotExist"; break;
    case -200737: return "DAQmxErrorInputBufferSizeNotEqualSampsPerChanForFiniteSampMode"; break;
    case -200736: return "DAQmxErrorFREQOUTCannotProduceDesiredFrequency2"; break;
    case -200735: return "DAQmxErrorExtRefClkRateNotSpecified"; break;
    case -200734: return "DAQmxErrorDeviceDoesNotSupportDMADataXferForNonBufferedAcq"; break;
    case -200733: return "DAQmxErrorDigFilterMinPulseWidthSetWhenTristateIsFalse"; break;
    case -200732: return "DAQmxErrorDigFilterEnableSetWhenTristateIsFalse"; break;
    case -200731: return "DAQmxErrorNoHWTimingWithOnDemand"; break;
    case -200730: return "DAQmxErrorCannotDetectChangesWhenTristateIsFalse"; break;
    case -200729: return "DAQmxErrorCannotHandshakeWhenTristateIsFalse"; break;
    case -200728: return "DAQmxErrorLinesUsedForStaticInputNotForHandshakingControl"; break;
    case -200727: return "DAQmxErrorLinesUsedForHandshakingControlNotForStaticInput"; break;
    case -200726: return "DAQmxErrorLinesUsedForStaticInputNotForHandshakingInput"; break;
    case -200725: return "DAQmxErrorLinesUsedForHandshakingInputNotForStaticInput"; break;
    case -200724: return "DAQmxErrorDifferentDITristateValsForChansInTask"; break;
    case -200723: return "DAQmxErrorTimebaseCalFreqVarianceTooLarge"; break;
    case -200722: return "DAQmxErrorTimebaseCalFailedToConverge"; break;
    case -200721: return "DAQmxErrorInadequateResolutionForTimebaseCal"; break;
    case -200720: return "DAQmxErrorInvalidAOGainCalConst"; break;
    case -200719: return "DAQmxErrorInvalidAOOffsetCalConst"; break;
    case -200718: return "DAQmxErrorInvalidAIGainCalConst"; break;
    case -200717: return "DAQmxErrorInvalidAIOffsetCalConst"; break;
    case -200716: return "DAQmxErrorDigOutputOverrun"; break;
    case -200715: return "DAQmxErrorDigInputOverrun"; break;
    case -200714: return "DAQmxErrorAcqStoppedDriverCantXferDataFastEnough"; break;
    case -200713: return "DAQmxErrorChansCantAppearInSameTask"; break;
    case -200712: return "DAQmxErrorInputCfgFailedBecauseWatchdogExpired"; break;
    case -200711: return "DAQmxErrorAnalogTrigChanNotExternal"; break;
    case -200710: return "DAQmxErrorTooManyChansForInternalAIInputSrc"; break;
    case -200709: return "DAQmxErrorTEDSSensorNotDetected"; break;
    case -200708: return "DAQmxErrorPrptyGetSpecdActiveItemFailedDueToDifftValues"; break;
    case -200706: return "DAQmxErrorRoutingDestTermPXIClk10InNotInSlot2"; break;
    case -200705: return "DAQmxErrorRoutingDestTermPXIStarXNotInSlot2"; break;
    case -200704: return "DAQmxErrorRoutingSrcTermPXIStarXNotInSlot2"; break;
    case -200703: return "DAQmxErrorRoutingSrcTermPXIStarInSlot16AndAbove"; break;
    case -200702: return "DAQmxErrorRoutingDestTermPXIStarInSlot16AndAbove"; break;
    case -200701: return "DAQmxErrorRoutingDestTermPXIStarInSlot2"; break;
    case -200700: return "DAQmxErrorRoutingSrcTermPXIStarInSlot2"; break;
    case -200699: return "DAQmxErrorRoutingDestTermPXIChassisNotIdentified"; break;
    case -200698: return "DAQmxErrorRoutingSrcTermPXIChassisNotIdentified"; break;
    case -200697: return "DAQmxErrorFailedToAcquireCalData"; break;
    case -200696: return "DAQmxErrorBridgeOffsetNullingCalNotSupported"; break;
    case -200695: return "DAQmxErrorAIMaxNotSpecified"; break;
    case -200694: return "DAQmxErrorAIMinNotSpecified"; break;
    case -200693: return "DAQmxErrorOddTotalBufferSizeToWrite"; break;
    case -200692: return "DAQmxErrorOddTotalNumSampsToWrite"; break;
    case -200691: return "DAQmxErrorBufferWithWaitMode"; break;
    case -200690: return "DAQmxErrorBufferWithHWTimedSinglePointSampMode"; break;
    case -200689: return "DAQmxErrorCOWritePulseLowTicksNotSupported"; break;
    case -200688: return "DAQmxErrorCOWritePulseHighTicksNotSupported"; break;
    case -200687: return "DAQmxErrorCOWritePulseLowTimeOutOfRange"; break;
    case -200686: return "DAQmxErrorCOWritePulseHighTimeOutOfRange"; break;
    case -200685: return "DAQmxErrorCOWriteFreqOutOfRange"; break;
    case -200684: return "DAQmxErrorCOWriteDutyCycleOutOfRange"; break;
    case -200683: return "DAQmxErrorInvalidInstallation"; break;
    case -200682: return "DAQmxErrorRefTrigMasterSessionUnavailable"; break;
    case -200681: return "DAQmxErrorRouteFailedBecauseWatchdogExpired"; break;
    case -200680: return "DAQmxErrorDeviceShutDownDueToHighTemp"; break;
    case -200679: return "DAQmxErrorNoMemMapWhenHWTimedSinglePoint"; break;
    case -200678: return "DAQmxErrorWriteFailedBecauseWatchdogExpired"; break;
    case -200677: return "DAQmxErrorDifftInternalAIInputSrcs"; break;
    case -200676: return "DAQmxErrorDifftAIInputSrcInOneChanGroup"; break;
    case -200675: return "DAQmxErrorInternalAIInputSrcInMultChanGroups"; break;
    case -200674: return "DAQmxErrorSwitchOpFailedDueToPrevError"; break;
    case -200673: return "DAQmxErrorWroteMultiSampsUsingSingleSampWrite"; break;
    case -200672: return "DAQmxErrorMismatchedInputArraySizes"; break;
    case -200671: return "DAQmxErrorCantExceedRelayDriveLimit"; break;
    case -200670: return "DAQmxErrorDACRngLowNotEqualToMinusRefVal"; break;
    case -200669: return "DAQmxErrorCantAllowConnectDACToGnd"; break;
    case -200668: return "DAQmxErrorWatchdogTimeoutOutOfRangeAndNotSpecialVal"; break;
    case -200667: return "DAQmxErrorNoWatchdogOutputOnPortReservedForInput"; break;
    case -200666: return "DAQmxErrorNoInputOnPortCfgdForWatchdogOutput"; break;
    case -200665: return "DAQmxErrorWatchdogExpirationStateNotEqualForLinesInPort"; break;
    case -200664: return "DAQmxErrorCannotPerformOpWhenTaskNotReserved"; break;
    case -200663: return "DAQmxErrorPowerupStateNotSupported"; break;
    case -200662: return "DAQmxErrorWatchdogTimerNotSupported"; break;
    case -200661: return "DAQmxErrorOpNotSupportedWhenRefClkSrcNone"; break;
    case -200660: return "DAQmxErrorSampClkRateUnavailable"; break;
    case -200659: return "DAQmxErrorPrptyGetSpecdSingleActiveChanFailedDueToDifftVals"; break;
    case -200658: return "DAQmxErrorPrptyGetImpliedActiveChanFailedDueToDifftVals"; break;
    case -200657: return "DAQmxErrorPrptyGetSpecdActiveChanFailedDueToDifftVals"; break;
    case -200656: return "DAQmxErrorNoRegenWhenUsingBrdMem"; break;
    case -200655: return "DAQmxErrorNonbufferedReadMoreThanSampsPerChan"; break;
    case -200654: return "DAQmxErrorWatchdogExpirationTristateNotSpecdForEntirePort"; break;
    case -200653: return "DAQmxErrorPowerupTristateNotSpecdForEntirePort"; break;
    case -200652: return "DAQmxErrorPowerupStateNotSpecdForEntirePort"; break;
    case -200651: return "DAQmxErrorCantSetWatchdogExpirationOnDigInChan"; break;
    case -200650: return "DAQmxErrorCantSetPowerupStateOnDigInChan"; break;
    case -200649: return "DAQmxErrorPhysChanNotInTask"; break;
    case -200648: return "DAQmxErrorPhysChanDevNotInTask"; break;
    case -200647: return "DAQmxErrorDigInputNotSupported"; break;
    case -200646: return "DAQmxErrorDigFilterIntervalNotEqualForLines"; break;
    case -200645: return "DAQmxErrorDigFilterIntervalAlreadyCfgd"; break;
    case -200644: return "DAQmxErrorCantResetExpiredWatchdog"; break;
    case -200643: return "DAQmxErrorActiveChanTooManyLinesSpecdWhenGettingPrpty"; break;
    case -200642: return "DAQmxErrorActiveChanNotSpecdWhenGetting1LinePrpty"; break;
    case -200641: return "DAQmxErrorDigPrptyCannotBeSetPerLine"; break;
    case -200640: return "DAQmxErrorSendAdvCmpltAfterWaitForTrigInScanlist"; break;
    case -200639: return "DAQmxErrorDisconnectionRequiredInScanlist"; break;
    case -200638: return "DAQmxErrorTwoWaitForTrigsAfterConnectionInScanlist"; break;
    case -200637: return "DAQmxErrorActionSeparatorRequiredAfterBreakingConnectionInScanlist"; break;
    case -200636: return "DAQmxErrorConnectionInScanlistMustWaitForTrig"; break;
    case -200635: return "DAQmxErrorActionNotSupportedTaskNotWatchdog"; break;
    case -200634: return "DAQmxErrorWfmNameSameAsScriptName"; break;
    case -200633: return "DAQmxErrorScriptNameSameAsWfmName"; break;
    case -200632: return "DAQmxErrorDSFStopClock"; break;
    case -200631: return "DAQmxErrorDSFReadyForStartClock"; break;
    case -200630: return "DAQmxErrorWriteOffsetNotMultOfIncr"; break;
    case -200629: return "DAQmxErrorDifferentPrptyValsNotSupportedOnDev"; break;
    case -200628: return "DAQmxErrorRefAndPauseTrigConfigured"; break;
    case -200627: return "DAQmxErrorFailedToEnableHighSpeedInputClock"; break;
    case -200626: return "DAQmxErrorEmptyPhysChanInPowerUpStatesArray"; break;
    case -200625: return "DAQmxErrorActivePhysChanTooManyLinesSpecdWhenGettingPrpty"; break;
    case -200624: return "DAQmxErrorActivePhysChanNotSpecdWhenGetting1LinePrpty"; break;
    case -200623: return "DAQmxErrorPXIDevTempCausedShutDown"; break;
    case -200622: return "DAQmxErrorInvalidNumSampsToWrite"; break;
    case -200621: return "DAQmxErrorOutputFIFOUnderflow2"; break;
    case -200620: return "DAQmxErrorRepeatedAIPhysicalChan"; break;
    case -200619: return "DAQmxErrorMultScanOpsInOneChassis"; break;
    case -200618: return "DAQmxErrorInvalidAIChanOrder"; break;
    case -200617: return "DAQmxErrorReversePowerProtectionActivated"; break;
    case -200616: return "DAQmxErrorInvalidAsynOpHandle"; break;
    case -200615: return "DAQmxErrorFailedToEnableHighSpeedOutput"; break;
    case -200614: return "DAQmxErrorCannotReadPastEndOfRecord"; break;
    case -200613: return "DAQmxErrorAcqStoppedToPreventInputBufferOverwriteOneDataXferMech"; break;
    case -200612: return "DAQmxErrorZeroBasedChanIndexInvalid"; break;
    case -200611: return "DAQmxErrorNoChansOfGivenTypeInTask"; break;
    case -200610: return "DAQmxErrorSampClkSrcInvalidForOutputValidForInput"; break;
    case -200609: return "DAQmxErrorOutputBufSizeTooSmallToStartGen"; break;
    case -200608: return "DAQmxErrorInputBufSizeTooSmallToStartAcq"; break;
    case -200607: return "DAQmxErrorExportTwoSignalsOnSameTerminal"; break;
    case -200606: return "DAQmxErrorChanIndexInvalid"; break;
    case -200605: return "DAQmxErrorRangeSyntaxNumberTooBig"; break;
    case -200604: return "DAQmxErrorNULLPtr"; break;
    case -200603: return "DAQmxErrorScaledMinEqualMax"; break;
    case -200602: return "DAQmxErrorPreScaledMinEqualMax"; break;
    case -200601: return "DAQmxErrorPropertyNotSupportedForScaleType"; break;
    case -200600: return "DAQmxErrorChannelNameGenerationNumberTooBig"; break;
    case -200599: return "DAQmxErrorRepeatedNumberInScaledValues"; break;
    case -200598: return "DAQmxErrorRepeatedNumberInPreScaledValues"; break;
    case -200597: return "DAQmxErrorLinesAlreadyReservedForOutput"; break;
    case -200596: return "DAQmxErrorSwitchOperationChansSpanMultipleDevsInList"; break;
    case -200595: return "DAQmxErrorInvalidIDInListAtBeginningOfSwitchOperation"; break;
    case -200594: return "DAQmxErrorMStudioInvalidPolyDirection"; break;
    case -200593: return "DAQmxErrorMStudioPropertyGetWhileTaskNotVerified"; break;
    case -200592: return "DAQmxErrorRangeWithTooManyObjects"; break;
    case -200591: return "DAQmxErrorCppDotNetAPINegativeBufferSize"; break;
    case -200590: return "DAQmxErrorCppCantRemoveInvalidEventHandler"; break;
    case -200589: return "DAQmxErrorCppCantRemoveEventHandlerTwice"; break;
    case -200588: return "DAQmxErrorCppCantRemoveOtherObjectsEventHandler"; break;
    case -200587: return "DAQmxErrorDigLinesReservedOrUnavailable"; break;
    case -200586: return "DAQmxErrorDSFFailedToResetStream"; break;
    case -200585: return "DAQmxErrorDSFReadyForOutputNotAsserted"; break;
    case -200584: return "DAQmxErrorSampToWritePerChanNotMultipleOfIncr"; break;
    case -200583: return "DAQmxErrorAOPropertiesCauseVoltageBelowMin"; break;
    case -200582: return "DAQmxErrorAOPropertiesCauseVoltageOverMax"; break;
    case -200581: return "DAQmxErrorPropertyNotSupportedWhenRefClkSrcNone"; break;
    case -200580: return "DAQmxErrorAIMaxTooSmall"; break;
    case -200579: return "DAQmxErrorAIMaxTooLarge"; break;
    case -200578: return "DAQmxErrorAIMinTooSmall"; break;
    case -200577: return "DAQmxErrorAIMinTooLarge"; break;
    case -200576: return "DAQmxErrorBuiltInCJCSrcNotSupported"; break;
    case -200575: return "DAQmxErrorTooManyPostTrigSampsPerChan"; break;
    case -200574: return "DAQmxErrorTrigLineNotFoundSingleDevRoute"; break;
    case -200573: return "DAQmxErrorDifferentInternalAIInputSources"; break;
    case -200572: return "DAQmxErrorDifferentAIInputSrcInOneChanGroup"; break;
    case -200571: return "DAQmxErrorInternalAIInputSrcInMultipleChanGroups"; break;
    case -200570: return "DAQmxErrorCAPIChanIndexInvalid"; break;
    case -200569: return "DAQmxErrorCollectionDoesNotMatchChanType"; break;
    case -200568: return "DAQmxErrorOutputCantStartChangedRegenerationMode"; break;
    case -200567: return "DAQmxErrorOutputCantStartChangedBufferSize"; break;
    case -200566: return "DAQmxErrorChanSizeTooBigForU32PortWrite"; break;
    case -200565: return "DAQmxErrorChanSizeTooBigForU8PortWrite"; break;
    case -200564: return "DAQmxErrorChanSizeTooBigForU32PortRead"; break;
    case -200563: return "DAQmxErrorChanSizeTooBigForU8PortRead"; break;
    case -200562: return "DAQmxErrorInvalidDigDataWrite"; break;
    case -200561: return "DAQmxErrorInvalidAODataWrite"; break;
    case -200560: return "DAQmxErrorWaitUntilDoneDoesNotIndicateDone"; break;
    case -200559: return "DAQmxErrorMultiChanTypesInTask"; break;
    case -200558: return "DAQmxErrorMultiDevsInTask"; break;
    case -200557: return "DAQmxErrorCannotSetPropertyWhenTaskRunning"; break;
    case -200556: return "DAQmxErrorCannotGetPropertyWhenTaskNotCommittedOrRunning"; break;
    case -200555: return "DAQmxErrorLeadingUnderscoreInString"; break;
    case -200554: return "DAQmxErrorTrailingSpaceInString"; break;
    case -200553: return "DAQmxErrorLeadingSpaceInString"; break;
    case -200552: return "DAQmxErrorInvalidCharInString"; break;
    case -200551: return "DAQmxErrorDLLBecameUnlocked"; break;
    case -200550: return "DAQmxErrorDLLLock"; break;
    case -200549: return "DAQmxErrorSelfCalConstsInvalid"; break;
    case -200548: return "DAQmxErrorInvalidTrigCouplingExceptForExtTrigChan"; break;
    case -200547: return "DAQmxErrorWriteFailsBufferSizeAutoConfigured"; break;
    case -200546: return "DAQmxErrorExtCalAdjustExtRefVoltageFailed"; break;
    case -200545: return "DAQmxErrorSelfCalFailedExtNoiseOrRefVoltageOutOfCal"; break;
    case -200544: return "DAQmxErrorExtCalTemperatureNotDAQmx"; break;
    case -200543: return "DAQmxErrorExtCalDateTimeNotDAQmx"; break;
    case -200542: return "DAQmxErrorSelfCalTemperatureNotDAQmx"; break;
    case -200541: return "DAQmxErrorSelfCalDateTimeNotDAQmx"; break;
    case -200540: return "DAQmxErrorDACRefValNotSet"; break;
    case -200539: return "DAQmxErrorAnalogMultiSampWriteNotSupported"; break;
    case -200538: return "DAQmxErrorInvalidActionInControlTask"; break;
    case -200537: return "DAQmxErrorPolyCoeffsInconsistent"; break;
    case -200536: return "DAQmxErrorSensorValTooLow"; break;
    case -200535: return "DAQmxErrorSensorValTooHigh"; break;
    case -200534: return "DAQmxErrorWaveformNameTooLong"; break;
    case -200533: return "DAQmxErrorIdentifierTooLongInScript"; break;
    case -200532: return "DAQmxErrorUnexpectedIDFollowingSwitchChanName"; break;
    case -200531: return "DAQmxErrorRelayNameNotSpecifiedInList"; break;
    case -200530: return "DAQmxErrorUnexpectedIDFollowingRelayNameInList"; break;
    case -200529: return "DAQmxErrorUnexpectedIDFollowingSwitchOpInList"; break;
    case -200528: return "DAQmxErrorInvalidLineGrouping"; break;
    case -200527: return "DAQmxErrorCtrMinMax"; break;
    case -200526: return "DAQmxErrorWriteChanTypeMismatch"; break;
    case -200525: return "DAQmxErrorReadChanTypeMismatch"; break;
    case -200524: return "DAQmxErrorWriteNumChansMismatch"; break;
    case -200523: return "DAQmxErrorOneChanReadForMultiChanTask"; break;
    case -200522: return "DAQmxErrorCannotSelfCalDuringExtCal"; break;
    case -200521: return "DAQmxErrorMeasCalAdjustOscillatorPhaseDAC"; break;
    case -200520: return "DAQmxErrorInvalidCalConstCalADCAdjustment"; break;
    case -200519: return "DAQmxErrorInvalidCalConstOscillatorFreqDACValue"; break;
    case -200518: return "DAQmxErrorInvalidCalConstOscillatorPhaseDACValue"; break;
    case -200517: return "DAQmxErrorInvalidCalConstOffsetDACValue"; break;
    case -200516: return "DAQmxErrorInvalidCalConstGainDACValue"; break;
    case -200515: return "DAQmxErrorInvalidNumCalADCReadsToAverage"; break;
    case -200514: return "DAQmxErrorInvalidCfgCalAdjustDirectPathOutputImpedance"; break;
    case -200513: return "DAQmxErrorInvalidCfgCalAdjustMainPathOutputImpedance"; break;
    case -200512: return "DAQmxErrorInvalidCfgCalAdjustMainPathPostAmpGainAndOffset"; break;
    case -200511: return "DAQmxErrorInvalidCfgCalAdjustMainPathPreAmpGain"; break;
    case -200510: return "DAQmxErrorInvalidCfgCalAdjustMainPreAmpOffset"; break;
    case -200509: return "DAQmxErrorMeasCalAdjustCalADC"; break;
    case -200508: return "DAQmxErrorMeasCalAdjustOscillatorFrequency"; break;
    case -200507: return "DAQmxErrorMeasCalAdjustDirectPathOutputImpedance"; break;
    case -200506: return "DAQmxErrorMeasCalAdjustMainPathOutputImpedance"; break;
    case -200505: return "DAQmxErrorMeasCalAdjustDirectPathGain"; break;
    case -200504: return "DAQmxErrorMeasCalAdjustMainPathPostAmpGainAndOffset"; break;
    case -200503: return "DAQmxErrorMeasCalAdjustMainPathPreAmpGain"; break;
    case -200502: return "DAQmxErrorMeasCalAdjustMainPathPreAmpOffset"; break;
    case -200501: return "DAQmxErrorInvalidDateTimeInEEPROM"; break;
    case -200500: return "DAQmxErrorUnableToLocateErrorResources"; break;
    case -200499: return "DAQmxErrorDotNetAPINotUnsigned32BitNumber"; break;
    case -200498: return "DAQmxErrorInvalidRangeOfObjectsSyntaxInString"; break;
    case -200497: return "DAQmxErrorAttemptToEnableLineNotPreviouslyDisabled"; break;
    case -200496: return "DAQmxErrorInvalidCharInPattern"; break;
    case -200495: return "DAQmxErrorIntermediateBufferFull"; break;
    case -200494: return "DAQmxErrorLoadTaskFailsBecauseNoTimingOnDev"; break;
    case -200493: return "DAQmxErrorCAPIReservedParamNotNULLNorEmpty"; break;
    case -200492: return "DAQmxErrorCAPIReservedParamNotNULL"; break;
    case -200491: return "DAQmxErrorCAPIReservedParamNotZero"; break;
    case -200490: return "DAQmxErrorSampleValueOutOfRange"; break;
    case -200489: return "DAQmxErrorChanAlreadyInTask"; break;
    case -200488: return "DAQmxErrorVirtualChanDoesNotExist"; break;
    case -200486: return "DAQmxErrorChanNotInTask"; break;
    case -200485: return "DAQmxErrorTaskNotInDataNeighborhood"; break;
    case -200484: return "DAQmxErrorCantSaveTaskWithoutReplace"; break;
    case -200483: return "DAQmxErrorCantSaveChanWithoutReplace"; break;
    case -200482: return "DAQmxErrorDevNotInTask"; break;
    case -200481: return "DAQmxErrorDevAlreadyInTask"; break;
    case -200479: return "DAQmxErrorCanNotPerformOpWhileTaskRunning"; break;
    case -200478: return "DAQmxErrorCanNotPerformOpWhenNoChansInTask"; break;
    case -200477: return "DAQmxErrorCanNotPerformOpWhenNoDevInTask"; break;
    case -200475: return "DAQmxErrorCannotPerformOpWhenTaskNotRunning"; break;
    case -200474: return "DAQmxErrorOperationTimedOut"; break;
    case -200473: return "DAQmxErrorCannotReadWhenAutoStartFalseAndTaskNotRunningOrCommitted"; break;
    case -200472: return "DAQmxErrorCannotWriteWhenAutoStartFalseAndTaskNotRunningOrCommitted"; break;
    case -200470: return "DAQmxErrorTaskVersionNew"; break;
    case -200469: return "DAQmxErrorChanVersionNew"; break;
    case -200467: return "DAQmxErrorEmptyString"; break;
    case -200466: return "DAQmxErrorChannelSizeTooBigForPortReadType"; break;
    case -200465: return "DAQmxErrorChannelSizeTooBigForPortWriteType"; break;
    case -200464: return "DAQmxErrorExpectedNumberOfChannelsVerificationFailed"; break;
    case -200463: return "DAQmxErrorNumLinesMismatchInReadOrWrite"; break;
    case -200462: return "DAQmxErrorOutputBufferEmpty"; break;
    case -200461: return "DAQmxErrorInvalidChanName"; break;
    case -200460: return "DAQmxErrorReadNoInputChansInTask"; break;
    case -200459: return "DAQmxErrorWriteNoOutputChansInTask"; break;
    case -200457: return "DAQmxErrorPropertyNotSupportedNotInputTask"; break;
    case -200456: return "DAQmxErrorPropertyNotSupportedNotOutputTask"; break;
    case -200455: return "DAQmxErrorGetPropertyNotInputBufferedTask"; break;
    case -200454: return "DAQmxErrorGetPropertyNotOutputBufferedTask"; break;
    case -200453: return "DAQmxErrorInvalidTimeoutVal"; break;
    case -200452: return "DAQmxErrorAttributeNotSupportedInTaskContext"; break;
    case -200451: return "DAQmxErrorAttributeNotQueryableUnlessTaskIsCommitted"; break;
    case -200450: return "DAQmxErrorAttributeNotSettableWhenTaskIsRunning"; break;
    case -200449: return "DAQmxErrorDACRngLowNotMinusRefValNorZero"; break;
    case -200448: return "DAQmxErrorDACRngHighNotEqualRefVal"; break;
    case -200447: return "DAQmxErrorUnitsNotFromCustomScale"; break;
    case -200446: return "DAQmxErrorInvalidVoltageReadingDuringExtCal"; break;
    case -200445: return "DAQmxErrorCalFunctionNotSupported"; break;
    case -200444: return "DAQmxErrorInvalidPhysicalChanForCal"; break;
    case -200443: return "DAQmxErrorExtCalNotComplete"; break;
    case -200442: return "DAQmxErrorCantSyncToExtStimulusFreqDuringCal"; break;
    case -200441: return "DAQmxErrorUnableToDetectExtStimulusFreqDuringCal"; break;
    case -200440: return "DAQmxErrorInvalidCloseAction"; break;
    case -200439: return "DAQmxErrorExtCalFunctionOutsideExtCalSession"; break;
    case -200438: return "DAQmxErrorInvalidCalArea"; break;
    case -200437: return "DAQmxErrorExtCalConstsInvalid"; break;
    case -200436: return "DAQmxErrorStartTrigDelayWithExtSampClk"; break;
    case -200435: return "DAQmxErrorDelayFromSampClkWithExtConv"; break;
    case -200434: return "DAQmxErrorFewerThan2PreScaledVals"; break;
    case -200433: return "DAQmxErrorFewerThan2ScaledValues"; break;
    case -200432: return "DAQmxErrorPhysChanOutputType"; break;
    case -200431: return "DAQmxErrorPhysChanMeasType"; break;
    case -200430: return "DAQmxErrorInvalidPhysChanType"; break;
    case -200429: return "DAQmxErrorLabVIEWEmptyTaskOrChans"; break;
    case -200428: return "DAQmxErrorLabVIEWInvalidTaskOrChans"; break;
    case -200427: return "DAQmxErrorInvalidRefClkRate"; break;
    case -200426: return "DAQmxErrorInvalidExtTrigImpedance"; break;
    case -200425: return "DAQmxErrorHystTrigLevelAIMax"; break;
    case -200424: return "DAQmxErrorLineNumIncompatibleWithVideoSignalFormat"; break;
    case -200423: return "DAQmxErrorTrigWindowAIMinAIMaxCombo"; break;
    case -200422: return "DAQmxErrorTrigAIMinAIMax"; break;
    case -200421: return "DAQmxErrorHystTrigLevelAIMin"; break;
    case -200420: return "DAQmxErrorInvalidSampRateConsiderRIS"; break;
    case -200419: return "DAQmxErrorInvalidReadPosDuringRIS"; break;
    case -200418: return "DAQmxErrorImmedTrigDuringRISMode"; break;
    case -200417: return "DAQmxErrorTDCNotEnabledDuringRISMode"; break;
    case -200416: return "DAQmxErrorMultiRecWithRIS"; break;
    case -200415: return "DAQmxErrorInvalidRefClkSrc"; break;
    case -200414: return "DAQmxErrorInvalidSampClkSrc"; break;
    case -200413: return "DAQmxErrorInsufficientOnBoardMemForNumRecsAndSamps"; break;
    case -200412: return "DAQmxErrorInvalidAIAttenuation"; break;
    case -200411: return "DAQmxErrorACCouplingNotAllowedWith50OhmImpedance"; break;
    case -200410: return "DAQmxErrorInvalidRecordNum"; break;
    case -200409: return "DAQmxErrorZeroSlopeLinearScale"; break;
    case -200408: return "DAQmxErrorZeroReversePolyScaleCoeffs"; break;
    case -200407: return "DAQmxErrorZeroForwardPolyScaleCoeffs"; break;
    case -200406: return "DAQmxErrorNoReversePolyScaleCoeffs"; break;
    case -200405: return "DAQmxErrorNoForwardPolyScaleCoeffs"; break;
    case -200404: return "DAQmxErrorNoPolyScaleCoeffs"; break;
    case -200403: return "DAQmxErrorReversePolyOrderLessThanNumPtsToCompute"; break;
    case -200402: return "DAQmxErrorReversePolyOrderNotPositive"; break;
    case -200401: return "DAQmxErrorNumPtsToComputeNotPositive"; break;
    case -200400: return "DAQmxErrorWaveformLengthNotMultipleOfIncr"; break;
    case -200399: return "DAQmxErrorCAPINoExtendedErrorInfoAvailable"; break;
    case -200398: return "DAQmxErrorCVIFunctionNotFoundInDAQmxDLL"; break;
    case -200397: return "DAQmxErrorCVIFailedToLoadDAQmxDLL"; break;
    case -200396: return "DAQmxErrorNoCommonTrigLineForImmedRoute"; break;
    case -200395: return "DAQmxErrorNoCommonTrigLineForTaskRoute"; break;
    case -200394: return "DAQmxErrorF64PrptyValNotUnsignedInt"; break;
    case -200393: return "DAQmxErrorRegisterNotWritable"; break;
    case -200392: return "DAQmxErrorInvalidOutputVoltageAtSampClkRate"; break;
    case -200391: return "DAQmxErrorStrobePhaseShiftDCMBecameUnlocked"; break;
    case -200390: return "DAQmxErrorDrivePhaseShiftDCMBecameUnlocked"; break;
    case -200389: return "DAQmxErrorClkOutPhaseShiftDCMBecameUnlocked"; break;
    case -200388: return "DAQmxErrorOutputBoardClkDCMBecameUnlocked"; break;
    case -200387: return "DAQmxErrorInputBoardClkDCMBecameUnlocked"; break;
    case -200386: return "DAQmxErrorInternalClkDCMBecameUnlocked"; break;
    case -200385: return "DAQmxErrorDCMLock"; break;
    case -200384: return "DAQmxErrorDataLineReservedForDynamicOutput"; break;
    case -200383: return "DAQmxErrorInvalidRefClkSrcGivenSampClkSrc"; break;
    case -200382: return "DAQmxErrorNoPatternMatcherAvailable"; break;
    case -200381: return "DAQmxErrorInvalidDelaySampRateBelowPhaseShiftDCMThresh"; break;
    case -200380: return "DAQmxErrorStrainGageCalibration"; break;
    case -200379: return "DAQmxErrorInvalidExtClockFreqAndDivCombo"; break;
    case -200378: return "DAQmxErrorCustomScaleDoesNotExist"; break;
    case -200377: return "DAQmxErrorOnlyFrontEndChanOpsDuringScan"; break;
    case -200376: return "DAQmxErrorInvalidOptionForDigitalPortChannel"; break;
    case -200375: return "DAQmxErrorUnsupportedSignalTypeExportSignal"; break;
    case -200374: return "DAQmxErrorInvalidSignalTypeExportSignal"; break;
    case -200373: return "DAQmxErrorUnsupportedTrigTypeSendsSWTrig"; break;
    case -200372: return "DAQmxErrorInvalidTrigTypeSendsSWTrig"; break;
    case -200371: return "DAQmxErrorRepeatedPhysicalChan"; break;
    case -200370: return "DAQmxErrorResourcesInUseForRouteInTask"; break;
    case -200369: return "DAQmxErrorResourcesInUseForRoute"; break;
    case -200368: return "DAQmxErrorRouteNotSupportedByHW"; break;
    case -200367: return "DAQmxErrorResourcesInUseForExportSignalPolarity"; break;
    case -200366: return "DAQmxErrorResourcesInUseForInversionInTask"; break;
    case -200365: return "DAQmxErrorResourcesInUseForInversion"; break;
    case -200364: return "DAQmxErrorExportSignalPolarityNotSupportedByHW"; break;
    case -200363: return "DAQmxErrorInversionNotSupportedByHW"; break;
    case -200362: return "DAQmxErrorOverloadedChansExistNotRead"; break;
    case -200361: return "DAQmxErrorInputFIFOOverflow2"; break;
    case -200360: return "DAQmxErrorCJCChanNotSpecd"; break;
    case -200359: return "DAQmxErrorCtrExportSignalNotPossible"; break;
    case -200358: return "DAQmxErrorRefTrigWhenContinuous"; break;
    case -200357: return "DAQmxErrorIncompatibleSensorOutputAndDeviceInputRanges"; break;
    case -200356: return "DAQmxErrorCustomScaleNameUsed"; break;
    case -200355: return "DAQmxErrorPropertyValNotSupportedByHW"; break;
    case -200354: return "DAQmxErrorPropertyValNotValidTermName"; break;
    case -200353: return "DAQmxErrorResourcesInUseForProperty"; break;
    case -200352: return "DAQmxErrorCJCChanAlreadyUsed"; break;
    case -200351: return "DAQmxErrorForwardPolynomialCoefNotSpecd"; break;
    case -200350: return "DAQmxErrorTableScaleNumPreScaledAndScaledValsNotEqual"; break;
    case -200349: return "DAQmxErrorTableScalePreScaledValsNotSpecd"; break;
    case -200348: return "DAQmxErrorTableScaleScaledValsNotSpecd"; break;
    case -200347: return "DAQmxErrorIntermediateBufferSizeNotMultipleOfIncr"; break;
    case -200346: return "DAQmxErrorEventPulseWidthOutOfRange"; break;
    case -200345: return "DAQmxErrorEventDelayOutOfRange"; break;
    case -200344: return "DAQmxErrorSampPerChanNotMultipleOfIncr"; break;
    case -200343: return "DAQmxErrorCannotCalculateNumSampsTaskNotStarted"; break;
    case -200342: return "DAQmxErrorScriptNotInMem"; break;
    case -200341: return "DAQmxErrorOnboardMemTooSmall"; break;
    case -200340: return "DAQmxErrorReadAllAvailableDataWithoutBuffer"; break;
    case -200339: return "DAQmxErrorPulseActiveAtStart"; break;
    case -200338: return "DAQmxErrorCalTempNotSupported"; break;
    case -200337: return "DAQmxErrorDelayFromSampClkTooLong"; break;
    case -200336: return "DAQmxErrorDelayFromSampClkTooShort"; break;
    case -200335: return "DAQmxErrorAIConvRateTooHigh"; break;
    case -200334: return "DAQmxErrorDelayFromStartTrigTooLong"; break;
    case -200333: return "DAQmxErrorDelayFromStartTrigTooShort"; break;
    case -200332: return "DAQmxErrorSampRateTooHigh"; break;
    case -200331: return "DAQmxErrorSampRateTooLow"; break;
    case -200330: return "DAQmxErrorPFI0UsedForAnalogAndDigitalSrc"; break;
    case -200329: return "DAQmxErrorPrimingCfgFIFO"; break;
    case -200328: return "DAQmxErrorCannotOpenTopologyCfgFile"; break;
    case -200327: return "DAQmxErrorInvalidDTInsideWfmDataType"; break;
    case -200326: return "DAQmxErrorRouteSrcAndDestSame"; break;
    case -200325: return "DAQmxErrorReversePolynomialCoefNotSpecd"; break;
    case -200324: return "DAQmxErrorDevAbsentOrUnavailable"; break;
    case -200323: return "DAQmxErrorNoAdvTrigForMultiDevScan"; break;
    case -200322: return "DAQmxErrorInterruptsInsufficientDataXferMech"; break;
    case -200321: return "DAQmxErrorInvalidAttentuationBasedOnMinMax"; break;
    case -200320: return "DAQmxErrorCabledModuleCannotRouteSSH"; break;
    case -200319: return "DAQmxErrorCabledModuleCannotRouteConvClk"; break;
    case -200318: return "DAQmxErrorInvalidExcitValForScaling"; break;
    case -200317: return "DAQmxErrorNoDevMemForScript"; break;
    case -200316: return "DAQmxErrorScriptDataUnderflow"; break;
    case -200315: return "DAQmxErrorNoDevMemForWaveform"; break;
    case -200314: return "DAQmxErrorStreamDCMBecameUnlocked"; break;
    case -200313: return "DAQmxErrorStreamDCMLock"; break;
    case -200312: return "DAQmxErrorWaveformNotInMem"; break;
    case -200311: return "DAQmxErrorWaveformWriteOutOfBounds"; break;
    case -200310: return "DAQmxErrorWaveformPreviouslyAllocated"; break;
    case -200309: return "DAQmxErrorSampClkTbMasterTbDivNotAppropriateForSampTbSrc"; break;
    case -200308: return "DAQmxErrorSampTbRateSampTbSrcMismatch"; break;
    case -200307: return "DAQmxErrorMasterTbRateMasterTbSrcMismatch"; break;
    case -200306: return "DAQmxErrorSampsPerChanTooBig"; break;
    case -200305: return "DAQmxErrorFinitePulseTrainNotPossible"; break;
    case -200304: return "DAQmxErrorExtMasterTimebaseRateNotSpecified"; break;
    case -200303: return "DAQmxErrorExtSampClkSrcNotSpecified"; break;
    case -200302: return "DAQmxErrorInputSignalSlowerThanMeasTime"; break;
    case -200301: return "DAQmxErrorCannotUpdatePulseGenProperty"; break;
    case -200300: return "DAQmxErrorInvalidTimingType"; break;
    case -200297: return "DAQmxErrorPropertyUnavailWhenUsingOnboardMemory"; break;
    case -200295: return "DAQmxErrorCannotWriteAfterStartWithOnboardMemory"; break;
    case -200294: return "DAQmxErrorNotEnoughSampsWrittenForInitialXferRqstCondition"; break;
    case -200293: return "DAQmxErrorNoMoreSpace"; break;
    case -200292: return "DAQmxErrorSamplesCanNotYetBeWritten"; break;
    case -200291: return "DAQmxErrorGenStoppedToPreventIntermediateBufferRegenOfOldSamples"; break;
    case -200290: return "DAQmxErrorGenStoppedToPreventRegenOfOldSamples"; break;
    case -200289: return "DAQmxErrorSamplesNoLongerWriteable"; break;
    case -200288: return "DAQmxErrorSamplesWillNeverBeGenerated"; break;
    case -200287: return "DAQmxErrorNegativeWriteSampleNumber"; break;
    case -200286: return "DAQmxErrorNoAcqStarted"; break;
    case -200284: return "DAQmxErrorSamplesNotYetAvailable"; break;
    case -200283: return "DAQmxErrorAcqStoppedToPreventIntermediateBufferOverflow"; break;
    case -200282: return "DAQmxErrorNoRefTrigConfigured"; break;
    case -200281: return "DAQmxErrorCannotReadRelativeToRefTrigUntilDone"; break;
    case -200279: return "DAQmxErrorSamplesNoLongerAvailable"; break;
    case -200278: return "DAQmxErrorSamplesWillNeverBeAvailable"; break;
    case -200277: return "DAQmxErrorNegativeReadSampleNumber"; break;
    case -200276: return "DAQmxErrorExternalSampClkAndRefClkThruSameTerm"; break;
    case -200275: return "DAQmxErrorExtSampClkRateTooLowForClkIn"; break;
    case -200274: return "DAQmxErrorExtSampClkRateTooHighForBackplane"; break;
    case -200273: return "DAQmxErrorSampClkRateAndDivCombo"; break;
    case -200272: return "DAQmxErrorSampClkRateTooLowForDivDown"; break;
    case -200271: return "DAQmxErrorProductOfAOMinAndGainTooSmall"; break;
    case -200270: return "DAQmxErrorInterpolationRateNotPossible"; break;
    case -200269: return "DAQmxErrorOffsetTooLarge"; break;
    case -200268: return "DAQmxErrorOffsetTooSmall"; break;
    case -200267: return "DAQmxErrorProductOfAOMaxAndGainTooLarge"; break;
    case -200266: return "DAQmxErrorMinAndMaxNotSymmetric"; break;
    case -200265: return "DAQmxErrorInvalidAnalogTrigSrc"; break;
    case -200264: return "DAQmxErrorTooManyChansForAnalogRefTrig"; break;
    case -200263: return "DAQmxErrorTooManyChansForAnalogPauseTrig"; break;
    case -200262: return "DAQmxErrorTrigWhenOnDemandSampTiming"; break;
    case -200261: return "DAQmxErrorInconsistentAnalogTrigSettings"; break;
    case -200260: return "DAQmxErrorMemMapDataXferModeSampTimingCombo"; break;
    case -200259: return "DAQmxErrorInvalidJumperedAttr"; break;
    case -200258: return "DAQmxErrorInvalidGainBasedOnMinMax"; break;
    case -200257: return "DAQmxErrorInconsistentExcit"; break;
    case -200256: return "DAQmxErrorTopologyNotSupportedByCfgTermBlock"; break;
    case -200255: return "DAQmxErrorBuiltInTempSensorNotSupported"; break;
    case -200254: return "DAQmxErrorInvalidTerm"; break;
    case -200253: return "DAQmxErrorCannotTristateTerm"; break;
    case -200252: return "DAQmxErrorCannotTristateBusyTerm"; break;
    case -200251: return "DAQmxErrorNoDMAChansAvailable"; break;
    case -200250: return "DAQmxErrorInvalidWaveformLengthWithinLoopInScript"; break;
    case -200249: return "DAQmxErrorInvalidSubsetLengthWithinLoopInScript"; break;
    case -200248: return "DAQmxErrorMarkerPosInvalidForLoopInScript"; break;
    case -200247: return "DAQmxErrorIntegerExpectedInScript"; break;
    case -200246: return "DAQmxErrorPLLBecameUnlocked"; break;
    case -200245: return "DAQmxErrorPLLLock"; break;
    case -200244: return "DAQmxErrorDDCClkOutDCMBecameUnlocked"; break;
    case -200243: return "DAQmxErrorDDCClkOutDCMLock"; break;
    case -200242: return "DAQmxErrorClkDoublerDCMBecameUnlocked"; break;
    case -200241: return "DAQmxErrorClkDoublerDCMLock"; break;
    case -200240: return "DAQmxErrorSampClkDCMBecameUnlocked"; break;
    case -200239: return "DAQmxErrorSampClkDCMLock"; break;
    case -200238: return "DAQmxErrorSampClkTimebaseDCMBecameUnlocked"; break;
    case -200237: return "DAQmxErrorSampClkTimebaseDCMLock"; break;
    case -200236: return "DAQmxErrorAttrCannotBeReset"; break;
    case -200235: return "DAQmxErrorExplanationNotFound"; break;
    case -200234: return "DAQmxErrorWriteBufferTooSmall"; break;
    case -200233: return "DAQmxErrorSpecifiedAttrNotValid"; break;
    case -200232: return "DAQmxErrorAttrCannotBeRead"; break;
    case -200231: return "DAQmxErrorAttrCannotBeSet"; break;
    case -200230: return "DAQmxErrorNULLPtrForC_Api"; break;
    case -200229: return "DAQmxErrorReadBufferTooSmall"; break;
    case -200228: return "DAQmxErrorBufferTooSmallForString"; break;
    case -200227: return "DAQmxErrorNoAvailTrigLinesOnDevice"; break;
    case -200226: return "DAQmxErrorTrigBusLineNotAvail"; break;
    case -200225: return "DAQmxErrorCouldNotReserveRequestedTrigLine"; break;
    case -200224: return "DAQmxErrorTrigLineNotFound"; break;
    case -200223: return "DAQmxErrorSCXI1126ThreshHystCombination"; break;
    case -200222: return "DAQmxErrorAcqStoppedToPreventInputBufferOverwrite"; break;
    case -200221: return "DAQmxErrorTimeoutExceeded"; break;
    case -200220: return "DAQmxErrorInvalidDeviceID"; break;
    case -200219: return "DAQmxErrorInvalidAOChanOrder"; break;
    case -200218: return "DAQmxErrorSampleTimingTypeAndDataXferMode"; break;
    case -200217: return "DAQmxErrorBufferWithOnDemandSampTiming"; break;
    case -200216: return "DAQmxErrorBufferAndDataXferMode"; break;
    case -200215: return "DAQmxErrorMemMapAndBuffer"; break;
    case -200214: return "DAQmxErrorNoAnalogTrigHW"; break;
    case -200213: return "DAQmxErrorTooManyPretrigPlusMinPostTrigSamps"; break;
    case -200212: return "DAQmxErrorInconsistentUnitsSpecified"; break;
    case -200211: return "DAQmxErrorMultipleRelaysForSingleRelayOp"; break;
    case -200210: return "DAQmxErrorMultipleDevIDsPerChassisSpecifiedInList"; break;
    case -200209: return "DAQmxErrorDuplicateDevIDInList"; break;
    case -200208: return "DAQmxErrorInvalidRangeStatementCharInList"; break;
    case -200207: return "DAQmxErrorInvalidDeviceIDInList"; break;
    case -200206: return "DAQmxErrorTriggerPolarityConflict"; break;
    case -200205: return "DAQmxErrorCannotScanWithCurrentTopology"; break;
    case -200204: return "DAQmxErrorUnexpectedIdentifierInFullySpecifiedPathInList"; break;
    case -200203: return "DAQmxErrorSwitchCannotDriveMultipleTrigLines"; break;
    case -200202: return "DAQmxErrorInvalidRelayName"; break;
    case -200201: return "DAQmxErrorSwitchScanlistTooBig"; break;
    case -200200: return "DAQmxErrorSwitchChanInUse"; break;
    case -200199: return "DAQmxErrorSwitchNotResetBeforeScan"; break;
    case -200198: return "DAQmxErrorInvalidTopology"; break;
    case -200197: return "DAQmxErrorAttrNotSupported"; break;
    case -200196: return "DAQmxErrorUnexpectedEndOfActionsInList"; break;
    case -200195: return "DAQmxErrorPowerLimitExceeded"; break;
    case -200194: return "DAQmxErrorHWUnexpectedlyPoweredOffAndOn"; break;
    case -200193: return "DAQmxErrorSwitchOperationNotSupported"; break;
    case -200192: return "DAQmxErrorOnlyContinuousScanSupported"; break;
    case -200191: return "DAQmxErrorSwitchDifferentTopologyWhenScanning"; break;
    case -200190: return "DAQmxErrorDisconnectPathNotSameAsExistingPath"; break;
    case -200189: return "DAQmxErrorConnectionNotPermittedOnChanReservedForRouting"; break;
    case -200188: return "DAQmxErrorCannotConnectSrcChans"; break;
    case -200187: return "DAQmxErrorCannotConnectChannelToItself"; break;
    case -200186: return "DAQmxErrorChannelNotReservedForRouting"; break;
    case -200185: return "DAQmxErrorCannotConnectChansDirectly"; break;
    case -200184: return "DAQmxErrorChansAlreadyConnected"; break;
    case -200183: return "DAQmxErrorChanDuplicatedInPath"; break;
    case -200182: return "DAQmxErrorNoPathToDisconnect"; break;
    case -200181: return "DAQmxErrorInvalidSwitchChan"; break;
    case -200180: return "DAQmxErrorNoPathAvailableBetween2SwitchChans"; break;
    case -200179: return "DAQmxErrorExplicitConnectionExists"; break;
    case -200178: return "DAQmxErrorSwitchDifferentSettlingTimeWhenScanning"; break;
    case -200177: return "DAQmxErrorOperationOnlyPermittedWhileScanning"; break;
    case -200176: return "DAQmxErrorOperationNotPermittedWhileScanning"; break;
    case -200175: return "DAQmxErrorHardwareNotResponding"; break;
    case -200173: return "DAQmxErrorInvalidSampAndMasterTimebaseRateCombo"; break;
    case -200172: return "DAQmxErrorNonZeroBufferSizeInProgIOXfer"; break;
    case -200171: return "DAQmxErrorVirtualChanNameUsed"; break;
    case -200170: return "DAQmxErrorPhysicalChanDoesNotExist"; break;
    case -200169: return "DAQmxErrorMemMapOnlyForProgIOXfer"; break;
    case -200168: return "DAQmxErrorTooManyChans"; break;
    case -200167: return "DAQmxErrorCannotHaveCJTempWithOtherChans"; break;
    case -200166: return "DAQmxErrorOutputBufferUnderwrite"; break;
    case -200163: return "DAQmxErrorSensorInvalidCompletionResistance"; break;
    case -200162: return "DAQmxErrorVoltageExcitIncompatibleWith2WireCfg"; break;
    case -200161: return "DAQmxErrorIntExcitSrcNotAvailable"; break;
    case -200160: return "DAQmxErrorCannotCreateChannelAfterTaskVerified"; break;
    case -200159: return "DAQmxErrorLinesReservedForSCXIControl"; break;
    case -200158: return "DAQmxErrorCouldNotReserveLinesForSCXIControl"; break;
    case -200157: return "DAQmxErrorCalibrationFailed"; break;
    case -200156: return "DAQmxErrorReferenceFrequencyInvalid"; break;
    case -200155: return "DAQmxErrorReferenceResistanceInvalid"; break;
    case -200154: return "DAQmxErrorReferenceCurrentInvalid"; break;
    case -200153: return "DAQmxErrorReferenceVoltageInvalid"; break;
    case -200152: return "DAQmxErrorEEPROMDataInvalid"; break;
    case -200151: return "DAQmxErrorCabledModuleNotCapableOfRoutingAI"; break;
    case -200150: return "DAQmxErrorChannelNotAvailableInParallelMode"; break;
    case -200149: return "DAQmxErrorExternalTimebaseRateNotKnownForDelay"; break;
    case -200148: return "DAQmxErrorFREQOUTCannotProduceDesiredFrequency"; break;
    case -200147: return "DAQmxErrorMultipleCounterInputTask"; break;
    case -200146: return "DAQmxErrorCounterStartPauseTriggerConflict"; break;
    case -200145: return "DAQmxErrorCounterInputPauseTriggerAndSampleClockInvalid"; break;
    case -200144: return "DAQmxErrorCounterOutputPauseTriggerInvalid"; break;
    case -200143: return "DAQmxErrorCounterTimebaseRateNotSpecified"; break;
    case -200142: return "DAQmxErrorCounterTimebaseRateNotFound"; break;
    case -200141: return "DAQmxErrorCounterOverflow"; break;
    case -200140: return "DAQmxErrorCounterNoTimebaseEdgesBetweenGates"; break;
    case -200139: return "DAQmxErrorCounterMaxMinRangeFreq"; break;
    case -200138: return "DAQmxErrorCounterMaxMinRangeTime"; break;
    case -200137: return "DAQmxErrorSuitableTimebaseNotFoundTimeCombo"; break;
    case -200136: return "DAQmxErrorSuitableTimebaseNotFoundFrequencyCombo"; break;
    case -200135: return "DAQmxErrorInternalTimebaseSourceDivisorCombo"; break;
    case -200134: return "DAQmxErrorInternalTimebaseSourceRateCombo"; break;
    case -200133: return "DAQmxErrorInternalTimebaseRateDivisorSourceCombo"; break;
    case -200132: return "DAQmxErrorExternalTimebaseRateNotknownForRate"; break;
    case -200131: return "DAQmxErrorAnalogTrigChanNotFirstInScanList"; break;
    case -200130: return "DAQmxErrorNoDivisorForExternalSignal"; break;
    case -200128: return "DAQmxErrorAttributeInconsistentAcrossRepeatedPhysicalChannels"; break;
    case -200127: return "DAQmxErrorCannotHandshakeWithPort0"; break;
    case -200126: return "DAQmxErrorControlLineConflictOnPortC"; break;
    case -200125: return "DAQmxErrorLines4To7ConfiguredForOutput"; break;
    case -200124: return "DAQmxErrorLines4To7ConfiguredForInput"; break;
    case -200123: return "DAQmxErrorLines0To3ConfiguredForOutput"; break;
    case -200122: return "DAQmxErrorLines0To3ConfiguredForInput"; break;
    case -200121: return "DAQmxErrorPortConfiguredForOutput"; break;
    case -200120: return "DAQmxErrorPortConfiguredForInput"; break;
    case -200119: return "DAQmxErrorPortConfiguredForStaticDigitalOps"; break;
    case -200118: return "DAQmxErrorPortReservedForHandshaking"; break;
    case -200117: return "DAQmxErrorPortDoesNotSupportHandshakingDataIO"; break;
    case -200116: return "DAQmxErrorCannotTristate8255OutputLines"; break;
    case -200113: return "DAQmxErrorTemperatureOutOfRangeForCalibration"; break;
    case -200112: return "DAQmxErrorCalibrationHandleInvalid"; break;
    case -200111: return "DAQmxErrorPasswordRequired"; break;
    case -200110: return "DAQmxErrorIncorrectPassword"; break;
    case -200109: return "DAQmxErrorPasswordTooLong"; break;
    case -200108: return "DAQmxErrorCalibrationSessionAlreadyOpen"; break;
    case -200107: return "DAQmxErrorSCXIModuleIncorrect"; break;
    case -200106: return "DAQmxErrorAttributeInconsistentAcrossChannelsOnDevice"; break;
    case -200105: return "DAQmxErrorSCXI1122ResistanceChanNotSupportedForCfg"; break;
    case -200104: return "DAQmxErrorBracketPairingMismatchInList"; break;
    case -200103: return "DAQmxErrorInconsistentNumSamplesToWrite"; break;
    case -200102: return "DAQmxErrorIncorrectDigitalPattern"; break;
    case -200101: return "DAQmxErrorIncorrectNumChannelsToWrite"; break;
    case -200100: return "DAQmxErrorIncorrectReadFunction"; break;
    case -200099: return "DAQmxErrorPhysicalChannelNotSpecified"; break;
    case -200098: return "DAQmxErrorMoreThanOneTerminal"; break;
    case -200097: return "DAQmxErrorMoreThanOneActiveChannelSpecified"; break;
    case -200096: return "DAQmxErrorInvalidNumberSamplesToRead"; break;
    case -200095: return "DAQmxErrorAnalogWaveformExpected"; break;
    case -200094: return "DAQmxErrorDigitalWaveformExpected"; break;
    case -200093: return "DAQmxErrorActiveChannelNotSpecified"; break;
    case -200092: return "DAQmxErrorFunctionNotSupportedForDeviceTasks"; break;
    case -200091: return "DAQmxErrorFunctionNotInLibrary"; break;
    case -200090: return "DAQmxErrorLibraryNotPresent"; break;
    case -200089: return "DAQmxErrorDuplicateTask"; break;
    case -200088: return "DAQmxErrorInvalidTask"; break;
    case -200087: return "DAQmxErrorInvalidChannel"; break;
    case -200086: return "DAQmxErrorInvalidSyntaxForPhysicalChannelRange"; break;
    case -200082: return "DAQmxErrorMinNotLessThanMax"; break;
    case -200081: return "DAQmxErrorSampleRateNumChansConvertPeriodCombo"; break;
    case -200079: return "DAQmxErrorAODuringCounter1DMAConflict"; break;
    case -200078: return "DAQmxErrorAIDuringCounter0DMAConflict"; break;
    case -200077: return "DAQmxErrorInvalidAttributeValue"; break;
    case -200076: return "DAQmxErrorSuppliedCurrentDataOutsideSpecifiedRange"; break;
    case -200075: return "DAQmxErrorSuppliedVoltageDataOutsideSpecifiedRange"; break;
    case -200074: return "DAQmxErrorCannotStoreCalConst"; break;
    case -200073: return "DAQmxErrorSCXIModuleNotFound"; break;
    case -200072: return "DAQmxErrorDuplicatePhysicalChansNotSupported"; break;
    case -200071: return "DAQmxErrorTooManyPhysicalChansInList"; break;
    case -200070: return "DAQmxErrorInvalidAdvanceEventTriggerType"; break;
    case -200069: return "DAQmxErrorDeviceIsNotAValidSwitch"; break;
    case -200068: return "DAQmxErrorDeviceDoesNotSupportScanning"; break;
    case -200067: return "DAQmxErrorScanListCannotBeTimed"; break;
    case -200066: return "DAQmxErrorConnectOperatorInvalidAtPointInList"; break;
    case -200065: return "DAQmxErrorUnexpectedSwitchActionInList"; break;
    case -200064: return "DAQmxErrorUnexpectedSeparatorInList"; break;
    case -200063: return "DAQmxErrorExpectedTerminatorInList"; break;
    case -200062: return "DAQmxErrorExpectedConnectOperatorInList"; break;
    case -200061: return "DAQmxErrorExpectedSeparatorInList"; break;
    case -200060: return "DAQmxErrorFullySpecifiedPathInListContainsRange"; break;
    case -200059: return "DAQmxErrorConnectionSeparatorAtEndOfList"; break;
    case -200058: return "DAQmxErrorIdentifierInListTooLong"; break;
    case -200057: return "DAQmxErrorDuplicateDeviceIDInListWhenSettling"; break;
    case -200056: return "DAQmxErrorChannelNameNotSpecifiedInList"; break;
    case -200055: return "DAQmxErrorDeviceIDNotSpecifiedInList"; break;
    case -200054: return "DAQmxErrorSemicolonDoesNotFollowRangeInList"; break;
    case -200053: return "DAQmxErrorSwitchActionInListSpansMultipleDevices"; break;
    case -200052: return "DAQmxErrorRangeWithoutAConnectActionInList"; break;
    case -200051: return "DAQmxErrorInvalidIdentifierFollowingSeparatorInList"; break;
    case -200050: return "DAQmxErrorInvalidChannelNameInList"; break;
    case -200049: return "DAQmxErrorInvalidNumberInRepeatStatementInList"; break;
    case -200048: return "DAQmxErrorInvalidTriggerLineInList"; break;
    case -200047: return "DAQmxErrorInvalidIdentifierInListFollowingDeviceID"; break;
    case -200046: return "DAQmxErrorInvalidIdentifierInListAtEndOfSwitchAction"; break;
    case -200045: return "DAQmxErrorDeviceRemoved"; break;
    case -200044: return "DAQmxErrorRoutingPathNotAvailable"; break;
    case -200043: return "DAQmxErrorRoutingHardwareBusy"; break;
    case -200042: return "DAQmxErrorRequestedSignalInversionForRoutingNotPossible"; break;
    case -200041: return "DAQmxErrorInvalidRoutingDestinationTerminalName"; break;
    case -200040: return "DAQmxErrorInvalidRoutingSourceTerminalName"; break;
    case -200039: return "DAQmxErrorRoutingNotSupportedForDevice"; break;
    case -200038: return "DAQmxErrorWaitIsLastInstructionOfLoopInScript"; break;
    case -200037: return "DAQmxErrorClearIsLastInstructionOfLoopInScript"; break;
    case -200036: return "DAQmxErrorInvalidLoopIterationsInScript"; break;
    case -200035: return "DAQmxErrorRepeatLoopNestingTooDeepInScript"; break;
    case -200034: return "DAQmxErrorMarkerPositionOutsideSubsetInScript"; break;
    case -200033: return "DAQmxErrorSubsetStartOffsetNotAlignedInScript"; break;
    case -200032: return "DAQmxErrorInvalidSubsetLengthInScript"; break;
    case -200031: return "DAQmxErrorMarkerPositionNotAlignedInScript"; break;
    case -200030: return "DAQmxErrorSubsetOutsideWaveformInScript"; break;
    case -200029: return "DAQmxErrorMarkerOutsideWaveformInScript"; break;
    case -200028: return "DAQmxErrorWaveformInScriptNotInMem"; break;
    case -200027: return "DAQmxErrorKeywordExpectedInScript"; break;
    case -200026: return "DAQmxErrorBufferNameExpectedInScript"; break;
    case -200025: return "DAQmxErrorProcedureNameExpectedInScript"; break;
    case -200024: return "DAQmxErrorScriptHasInvalidIdentifier"; break;
    case -200023: return "DAQmxErrorScriptHasInvalidCharacter"; break;
    case -200022: return "DAQmxErrorResourceAlreadyReserved"; break;
    case -200020: return "DAQmxErrorSelfTestFailed"; break;
    case -200019: return "DAQmxErrorADCOverrun"; break;
    case -200018: return "DAQmxErrorDACUnderflow"; break;
    case -200017: return "DAQmxErrorInputFIFOUnderflow"; break;
    case -200016: return "DAQmxErrorOutputFIFOUnderflow"; break;
    case -200015: return "DAQmxErrorSCXISerialCommunication"; break;
    case -200014: return "DAQmxErrorDigitalTerminalSpecifiedMoreThanOnce"; break;
    case -200012: return "DAQmxErrorDigitalOutputNotSupported"; break;
    case -200011: return "DAQmxErrorInconsistentChannelDirections"; break;
    case -200010: return "DAQmxErrorInputFIFOOverflow"; break;
    case -200009: return "DAQmxErrorTimeStampOverwritten"; break;
    case -200008: return "DAQmxErrorStopTriggerHasNotOccurred"; break;
    case -200007: return "DAQmxErrorRecordNotAvailable"; break;
    case -200006: return "DAQmxErrorRecordOverwritten"; break;
    case -200005: return "DAQmxErrorDataNotAvailable"; break;
    case -200004: return "DAQmxErrorDataOverwrittenInDeviceMemory"; break;
    case -200003: return "DAQmxErrorDuplicatedChannel"; break;
    case 200003: return "DAQmxWarningTimestampCounterRolledOver"; break;
    case 200004: return "DAQmxWarningInputTerminationOverloaded"; break;
    case 200005: return "DAQmxWarningADCOverloaded"; break;
    case 200007: return "DAQmxWarningPLLUnlocked"; break;
    case 200008: return "DAQmxWarningCounter0DMADuringAIConflict"; break;
    case 200009: return "DAQmxWarningCounter1DMADuringAOConflict"; break;
    case 200010: return "DAQmxWarningStoppedBeforeDone"; break;
    case 200011: return "DAQmxWarningRateViolatesSettlingTime"; break;
    case 200012: return "DAQmxWarningRateViolatesMaxADCRate"; break;
    case 200013: return "DAQmxWarningUserDefInfoStringTooLong"; break;
    case 200014: return "DAQmxWarningTooManyInterruptsPerSecond"; break;
    case 200015: return "DAQmxWarningPotentialGlitchDuringWrite"; break;
    case 200016: return "DAQmxWarningDevNotSelfCalibratedWithDAQmx"; break;
    case 200017: return "DAQmxWarningAISampRateTooLow"; break;
    case 200018: return "DAQmxWarningAIConvRateTooLow"; break;
    case 200019: return "DAQmxWarningReadOffsetCoercion"; break;
    case 200020: return "DAQmxWarningPretrigCoercion"; break;
    case 200021: return "DAQmxWarningSampValCoercedToMax"; break;
    case 200022: return "DAQmxWarningSampValCoercedToMin"; break;
    case 200024: return "DAQmxWarningPropertyVersionNew"; break;
    case 200025: return "DAQmxWarningUserDefinedInfoTooLong"; break;
    case 200026: return "DAQmxWarningCAPIStringTruncatedToFitBuffer"; break;
    case 200027: return "DAQmxWarningSampClkRateTooLow"; break;
    case 200028: return "DAQmxWarningPossiblyInvalidCTRSampsInFiniteDMAAcq"; break;
    case 200029: return "DAQmxWarningRISAcqCompletedSomeBinsNotFilled"; break;
    case 200030: return "DAQmxWarningPXIDevTempExceedsMaxOpTemp"; break;
    case 200031: return "DAQmxWarningOutputGainTooLowForRFFreq"; break;
    case 200032: return "DAQmxWarningOutputGainTooHighForRFFreq"; break;
    case 200033: return "DAQmxWarningMultipleWritesBetweenSampClks"; break;
    case 200034: return "DAQmxWarningDeviceMayShutDownDueToHighTemp"; break;
    case 200035: return "DAQmxWarningRateViolatesMinADCRate"; break;
    case 200036: return "DAQmxWarningSampClkRateAboveDevSpecs"; break;
    case 200037: return "DAQmxWarningCOPrevDAQmxWriteSettingsOverwrittenForHWTimedSinglePoint"; break;
    case 200038: return "DAQmxWarningLowpassFilterSettlingTimeExceedsUserTimeBetween2ADCConversions"; break;
    case 200039: return "DAQmxWarningLowpassFilterSettlingTimeExceedsDriverTimeBetween2ADCConversions"; break;
    case 200040: return "DAQmxWarningSampClkRateViolatesSettlingTimeForGen"; break;
    case 200041: return "DAQmxWarningInvalidCalConstValueForAI"; break;
    case 200042: return "DAQmxWarningInvalidCalConstValueForAO"; break;
    case 200043: return "DAQmxWarningChanCalExpired"; break;
    case 200044: return "DAQmxWarningUnrecognizedEnumValueEncounteredInStorage"; break;
    case 200045: return "DAQmxWarningTableCRCNotCorrect"; break;
    case 200046: return "DAQmxWarningExternalCRCNotCorrect"; break;
    case 200047: return "DAQmxWarningSelfCalCRCNotCorrect"; break;
    case 200048: return "DAQmxWarningDeviceSpecExceeded"; break;
    case 200049: return "DAQmxWarningOnlyGainCalibrated"; break;
    case 200050: return "DAQmxWarningReversePowerProtectionActivated"; break;
    case 200051: return "DAQmxWarningOverVoltageProtectionActivated"; break;
    case 200052: return "DAQmxWarningBufferSizeNotMultipleOfSectorSize"; break;
    case 200053: return "DAQmxWarningSampleRateMayCauseAcqToFail"; break;
    case 200054: return "DAQmxWarningUserAreaCRCNotCorrect"; break;
    case 200055: return "DAQmxWarningPowerUpInfoCRCNotCorrect"; break;
    case 200056: return "DAQmxWarningConnectionCountHasExceededRecommendedLimit"; break;
    case 200057: return "DAQmxWarningNetworkDeviceAlreadyAdded"; break;
    case 200058: return "DAQmxWarningAccessoryConnectionCountIsInvalid"; break;
    case 200059: return "DAQmxWarningUnableToDisconnectPorts"; break;
    case 200060: return "DAQmxWarningReadRepeatedData"; break;
    case 200061: return "DAQmxWarningPXI5600_NotConfigured"; break;
    case 200062: return "DAQmxWarningPXI5661_IncorrectlyConfigured"; break;
    case 200063: return "DAQmxWarningPXIe5601_NotConfigured"; break;
    case 200064: return "DAQmxWarningPXIe5663_IncorrectlyConfigured"; break;
    case 200065: return "DAQmxWarningPXIe5663E_IncorrectlyConfigured"; break;
    case 200066: return "DAQmxWarningPXIe5603_NotConfigured"; break;
    case 200067: return "DAQmxWarningPXIe5665_5603_IncorrectlyConfigured"; break;
    case 200068: return "DAQmxWarningPXIe5667_5603_IncorrectlyConfigured"; break;
    case 200069: return "DAQmxWarningPXIe5605_NotConfigured"; break;
    case 200070: return "DAQmxWarningPXIe5665_5605_IncorrectlyConfigured"; break;
    case 200071: return "DAQmxWarningPXIe5667_5605_IncorrectlyConfigured"; break;
    case 200072: return "DAQmxWarningPXIe5606_NotConfigured"; break;
    case 200073: return "DAQmxWarningPXIe5665_5606_IncorrectlyConfigured"; break;
    case 200074: return "DAQmxWarningPXI5610_NotConfigured"; break;
    case 200075: return "DAQmxWarningPXI5610_IncorrectlyConfigured"; break;
    case 200076: return "DAQmxWarningPXIe5611_NotConfigured"; break;
    case 200077: return "DAQmxWarningPXIe5611_IncorrectlyConfigured"; break;
    case 200078: return "DAQmxWarningUSBHotfixForDAQ"; break;
    case 200079: return "DAQmxWarningNoChangeSupersededByIdleBehavior"; break;
    case 209800: return "DAQmxWarningReadNotCompleteBeforeSampClk"; break;
    case 209801: return "DAQmxWarningWriteNotCompleteBeforeSampClk"; break;
    case 209802: return "DAQmxWarningWaitForNextSampClkDetectedMissedSampClk"; break;
    case 209803: return "DAQmxWarningOutputDataTransferConditionNotSupported"; break;
    case -89167: return "DAQmxErrorRoutingDestTermPXIDStarXNotInSystemTimingSlot_Routing"; break;
    case -89166: return "DAQmxErrorRoutingSrcTermPXIDStarXNotInSystemTimingSlot_Routing"; break;
    case -89165: return "DAQmxErrorRoutingSrcTermPXIDStarInNonDStarTriggerSlot_Routing"; break;
    case -89164: return "DAQmxErrorRoutingDestTermPXIDStarInNonDStarTriggerSlot_Routing"; break;
    case -89162: return "DAQmxErrorRoutingDestTermPXIClk10InNotInStarTriggerSlot_Routing"; break;
    case -89161: return "DAQmxErrorRoutingDestTermPXIClk10InNotInSystemTimingSlot_Routing"; break;
    case -89160: return "DAQmxErrorRoutingDestTermPXIStarXNotInStarTriggerSlot_Routing"; break;
    case -89159: return "DAQmxErrorRoutingDestTermPXIStarXNotInSystemTimingSlot_Routing"; break;
    case -89158: return "DAQmxErrorRoutingSrcTermPXIStarXNotInStarTriggerSlot_Routing"; break;
    case -89157: return "DAQmxErrorRoutingSrcTermPXIStarXNotInSystemTimingSlot_Routing"; break;
    case -89156: return "DAQmxErrorRoutingSrcTermPXIStarInNonStarTriggerSlot_Routing"; break;
    case -89155: return "DAQmxErrorRoutingDestTermPXIStarInNonStarTriggerSlot_Routing"; break;
    case -89154: return "DAQmxErrorRoutingDestTermPXIStarInStarTriggerSlot_Routing"; break;
    case -89153: return "DAQmxErrorRoutingDestTermPXIStarInSystemTimingSlot_Routing"; break;
    case -89152: return "DAQmxErrorRoutingSrcTermPXIStarInStarTriggerSlot_Routing"; break;
    case -89151: return "DAQmxErrorRoutingSrcTermPXIStarInSystemTimingSlot_Routing"; break;
    case -89150: return "DAQmxErrorInvalidSignalModifier_Routing"; break;
    case -89149: return "DAQmxErrorRoutingDestTermPXIClk10InNotInSlot2_Routing"; break;
    case -89148: return "DAQmxErrorRoutingDestTermPXIStarXNotInSlot2_Routing"; break;
    case -89147: return "DAQmxErrorRoutingSrcTermPXIStarXNotInSlot2_Routing"; break;
    case -89146: return "DAQmxErrorRoutingSrcTermPXIStarInSlot16AndAbove_Routing"; break;
    case -89145: return "DAQmxErrorRoutingDestTermPXIStarInSlot16AndAbove_Routing"; break;
    case -89144: return "DAQmxErrorRoutingDestTermPXIStarInSlot2_Routing"; break;
    case -89143: return "DAQmxErrorRoutingSrcTermPXIStarInSlot2_Routing"; break;
    case -89142: return "DAQmxErrorRoutingDestTermPXIChassisNotIdentified_Routing"; break;
    case -89141: return "DAQmxErrorRoutingSrcTermPXIChassisNotIdentified_Routing"; break;
    case -89140: return "DAQmxErrorTrigLineNotFoundSingleDevRoute_Routing"; break;
    case -89139: return "DAQmxErrorNoCommonTrigLineForRoute_Routing"; break;
    case -89138: return "DAQmxErrorResourcesInUseForRouteInTask_Routing"; break;
    case -89137: return "DAQmxErrorResourcesInUseForRoute_Routing"; break;
    case -89136: return "DAQmxErrorRouteNotSupportedByHW_Routing"; break;
    case -89135: return "DAQmxErrorResourcesInUseForInversionInTask_Routing"; break;
    case -89134: return "DAQmxErrorResourcesInUseForInversion_Routing"; break;
    case -89133: return "DAQmxErrorInversionNotSupportedByHW_Routing"; break;
    case -89132: return "DAQmxErrorResourcesInUseForProperty_Routing"; break;
    case -89131: return "DAQmxErrorRouteSrcAndDestSame_Routing"; break;
    case -89130: return "DAQmxErrorDevAbsentOrUnavailable_Routing"; break;
    case -89129: return "DAQmxErrorInvalidTerm_Routing"; break;
    case -89128: return "DAQmxErrorCannotTristateTerm_Routing"; break;
    case -89127: return "DAQmxErrorCannotTristateBusyTerm_Routing"; break;
    case -89126: return "DAQmxErrorCouldNotReserveRequestedTrigLine_Routing"; break;
    case -89125: return "DAQmxErrorTrigLineNotFound_Routing"; break;
    case -89124: return "DAQmxErrorRoutingPathNotAvailable_Routing"; break;
    case -89123: return "DAQmxErrorRoutingHardwareBusy_Routing"; break;
    case -89122: return "DAQmxErrorRequestedSignalInversionForRoutingNotPossible_Routing"; break;
    case -89121: return "DAQmxErrorInvalidRoutingDestinationTerminalName_Routing"; break;
    case -89120: return "DAQmxErrorInvalidRoutingSourceTerminalName_Routing"; break;
    case -88907: return "DAQmxErrorServiceLocatorNotAvailable_Routing"; break;
    case -88900: return "DAQmxErrorCouldNotConnectToServer_Routing"; break;
    case -88720: return "DAQmxErrorDeviceNameContainsSpacesOrPunctuation_Routing"; break;
    case -88719: return "DAQmxErrorDeviceNameContainsNonprintableCharacters_Routing"; break;
    case -88718: return "DAQmxErrorDeviceNameIsEmpty_Routing"; break;
    case -88717: return "DAQmxErrorDeviceNameNotFound_Routing"; break;
    case -88716: return "DAQmxErrorLocalRemoteDriverVersionMismatch_Routing"; break;
    case -88715: return "DAQmxErrorDuplicateDeviceName_Routing"; break;
    case -88710: return "DAQmxErrorRuntimeAborting_Routing"; break;
    case -88709: return "DAQmxErrorRuntimeAborted_Routing"; break;
    case -88708: return "DAQmxErrorResourceNotInPool_Routing"; break;
    case -88705: return "DAQmxErrorDriverDeviceGUIDNotFound_Routing"; break;
    case -50808: return "DAQmxErrorPALUSBTransactionError"; break;
    case -50807: return "DAQmxErrorPALIsocStreamBufferError"; break;
    case -50806: return "DAQmxErrorPALInvalidAddressComponent"; break;
    case -50805: return "DAQmxErrorPALSharingViolation"; break;
    case -50804: return "DAQmxErrorPALInvalidDeviceState"; break;
    case -50803: return "DAQmxErrorPALConnectionReset"; break;
    case -50802: return "DAQmxErrorPALConnectionAborted"; break;
    case -50801: return "DAQmxErrorPALConnectionRefused"; break;
    case -50800: return "DAQmxErrorPALBusResetOccurred"; break;
    case -50700: return "DAQmxErrorPALWaitInterrupted"; break;
    case -50651: return "DAQmxErrorPALMessageUnderflow"; break;
    case -50650: return "DAQmxErrorPALMessageOverflow"; break;
    case -50604: return "DAQmxErrorPALThreadAlreadyDead"; break;
    case -50603: return "DAQmxErrorPALThreadStackSizeNotSupported"; break;
    case -50602: return "DAQmxErrorPALThreadControllerIsNotThreadCreator"; break;
    case -50601: return "DAQmxErrorPALThreadHasNoThreadObject"; break;
    case -50600: return "DAQmxErrorPALThreadCouldNotRun"; break;
    case -50551: return "DAQmxErrorPALSyncAbandoned"; break;
    case -50550: return "DAQmxErrorPALSyncTimedOut"; break;
    case -50503: return "DAQmxErrorPALReceiverSocketInvalid"; break;
    case -50502: return "DAQmxErrorPALSocketListenerInvalid"; break;
    case -50501: return "DAQmxErrorPALSocketListenerAlreadyRegistered"; break;
    case -50500: return "DAQmxErrorPALDispatcherAlreadyExported"; break;
    case -50450: return "DAQmxErrorPALDMALinkEventMissed"; break;
    case -50413: return "DAQmxErrorPALBusError"; break;
    case -50412: return "DAQmxErrorPALRetryLimitExceeded"; break;
    case -50411: return "DAQmxErrorPALTransferOverread"; break;
    case -50410: return "DAQmxErrorPALTransferOverwritten"; break;
    case -50409: return "DAQmxErrorPALPhysicalBufferFull"; break;
    case -50408: return "DAQmxErrorPALPhysicalBufferEmpty"; break;
    case -50407: return "DAQmxErrorPALLogicalBufferFull"; break;
    case -50406: return "DAQmxErrorPALLogicalBufferEmpty"; break;
    case -50405: return "DAQmxErrorPALTransferAborted"; break;
    case -50404: return "DAQmxErrorPALTransferStopped"; break;
    case -50403: return "DAQmxErrorPALTransferInProgress"; break;
    case -50402: return "DAQmxErrorPALTransferNotInProgress"; break;
    case -50401: return "DAQmxErrorPALCommunicationsFault"; break;
    case -50400: return "DAQmxErrorPALTransferTimedOut"; break;
    case -50355: return "DAQmxErrorPALMemoryHeapNotEmpty"; break;
    case -50354: return "DAQmxErrorPALMemoryBlockCheckFailed"; break;
    case -50353: return "DAQmxErrorPALMemoryPageLockFailed"; break;
    case -50352: return "DAQmxErrorPALMemoryFull"; break;
    case -50351: return "DAQmxErrorPALMemoryAlignmentFault"; break;
    case -50350: return "DAQmxErrorPALMemoryConfigurationFault"; break;
    case -50303: return "DAQmxErrorPALDeviceInitializationFault"; break;
    case -50302: return "DAQmxErrorPALDeviceNotSupported"; break;
    case -50301: return "DAQmxErrorPALDeviceUnknown"; break;
    case -50300: return "DAQmxErrorPALDeviceNotFound"; break;
    case -50265: return "DAQmxErrorPALFeatureDisabled"; break;
    case -50264: return "DAQmxErrorPALComponentBusy"; break;
    case -50263: return "DAQmxErrorPALComponentAlreadyInstalled"; break;
    case -50262: return "DAQmxErrorPALComponentNotUnloadable"; break;
    case -50261: return "DAQmxErrorPALComponentNeverLoaded"; break;
    case -50260: return "DAQmxErrorPALComponentAlreadyLoaded"; break;
    case -50259: return "DAQmxErrorPALComponentCircularDependency"; break;
    case -50258: return "DAQmxErrorPALComponentInitializationFault"; break;
    case -50257: return "DAQmxErrorPALComponentImageCorrupt"; break;
    case -50256: return "DAQmxErrorPALFeatureNotSupported"; break;
    case -50255: return "DAQmxErrorPALFunctionNotFound"; break;
    case -50254: return "DAQmxErrorPALFunctionObsolete"; break;
    case -50253: return "DAQmxErrorPALComponentTooNew"; break;
    case -50252: return "DAQmxErrorPALComponentTooOld"; break;
    case -50251: return "DAQmxErrorPALComponentNotFound"; break;
    case -50250: return "DAQmxErrorPALVersionMismatch"; break;
    case -50209: return "DAQmxErrorPALFileFault"; break;
    case -50208: return "DAQmxErrorPALFileWriteFault"; break;
    case -50207: return "DAQmxErrorPALFileReadFault"; break;
    case -50206: return "DAQmxErrorPALFileSeekFault"; break;
    case -50205: return "DAQmxErrorPALFileCloseFault"; break;
    case -50204: return "DAQmxErrorPALFileOpenFault"; break;
    case -50203: return "DAQmxErrorPALDiskFull"; break;
    case -50202: return "DAQmxErrorPALOSFault"; break;
    case -50201: return "DAQmxErrorPALOSInitializationFault"; break;
    case -50200: return "DAQmxErrorPALOSUnsupported"; break;
    case -50175: return "DAQmxErrorPALCalculationOverflow"; break;
    case -50152: return "DAQmxErrorPALHardwareFault"; break;
    case -50151: return "DAQmxErrorPALFirmwareFault"; break;
    case -50150: return "DAQmxErrorPALSoftwareFault"; break;
    case -50108: return "DAQmxErrorPALMessageQueueFull"; break;
    case -50107: return "DAQmxErrorPALResourceAmbiguous"; break;
    case -50106: return "DAQmxErrorPALResourceBusy"; break;
    case -50105: return "DAQmxErrorPALResourceInitialized"; break;
    case -50104: return "DAQmxErrorPALResourceNotInitialized"; break;
    case -50103: return "DAQmxErrorPALResourceReserved"; break;
    case -50102: return "DAQmxErrorPALResourceNotReserved"; break;
    case -50101: return "DAQmxErrorPALResourceNotAvailable"; break;
    case -50100: return "DAQmxErrorPALResourceOwnedBySystem"; break;
    case -50020: return "DAQmxErrorPALBadToken"; break;
    case -50019: return "DAQmxErrorPALBadThreadMultitask"; break;
    case -50018: return "DAQmxErrorPALBadLibrarySpecifier"; break;
    case -50017: return "DAQmxErrorPALBadAddressSpace"; break;
    case -50016: return "DAQmxErrorPALBadWindowType"; break;
    case -50015: return "DAQmxErrorPALBadAddressClass"; break;
    case -50014: return "DAQmxErrorPALBadWriteCount"; break;
    case -50013: return "DAQmxErrorPALBadWriteOffset"; break;
    case -50012: return "DAQmxErrorPALBadWriteMode"; break;
    case -50011: return "DAQmxErrorPALBadReadCount"; break;
    case -50010: return "DAQmxErrorPALBadReadOffset"; break;
    case -50009: return "DAQmxErrorPALBadReadMode"; break;
    case -50008: return "DAQmxErrorPALBadCount"; break;
    case -50007: return "DAQmxErrorPALBadOffset"; break;
    case -50006: return "DAQmxErrorPALBadMode"; break;
    case -50005: return "DAQmxErrorPALBadDataSize"; break;
    case -50004: return "DAQmxErrorPALBadPointer"; break;
    case -50003: return "DAQmxErrorPALBadSelector"; break;
    case -50002: return "DAQmxErrorPALBadDevice"; break;
    case -50001: return "DAQmxErrorPALIrrelevantAttribute"; break;
    case -50000: return "DAQmxErrorPALValueConflict"; break;
    case -26853: return "DAQmxErrorRetryCall"; break;
    case -26852: return "DAQmxErrorFileDoesNotExist"; break;
    case -26851: return "DAQmxErrorGenerationDisabled"; break;
    case -26850: return "DAQmxErrorAlreadyInitialized"; break;
    case -26809: return "DAQmxErrorPXISystemDescriptionParseError"; break;
    case -26808: return "DAQmxErrorPXISAConfigurationLocked"; break;
    case -26807: return "DAQmxErrorNotActiveResourceManager"; break;
    case -26806: return "DAQmxErrorInvalidAttribute"; break;
    case -26805: return "DAQmxErrorInvalidHandle"; break;
    case -26804: return "DAQmxErrorServiceNotRunning"; break;
    case -26803: return "DAQmxErrorRecursiveCall"; break;
    case -26802: return "DAQmxErrorTimeout"; break;
    case -26801: return "DAQmxErrorUnspecifiedError"; break;
    case -26657: return "DAQmxErrorHandlerNotFound"; break;
    case -26656: return "DAQmxErrorIncorrectDataType"; break;
    case -26655: return "DAQmxErrorInconsistentFileFault"; break;
    case -26654: return "DAQmxErrorChildNotFound"; break;
    case -26653: return "DAQmxErrorAttributeNotFound"; break;
    case -26652: return "DAQmxErrorIOError"; break;
    case -26600: return "DAQmxErrorPxiResmanMxsPxiSystemNotFound"; break;
    case -26550: return "DAQmxErrorPxiResmanPciDescriptionStringParseError"; break;
    case -26500: return "DAQmxErrorPxiResmanModuleParseError"; break;
    case -26403: return "DAQmxErrorPxiResmanSystemDescriptionSpecLimitation"; break;
    case -26402: return "DAQmxErrorPxiResmanSystemDescriptionWriteError"; break;
    case -26401: return "DAQmxErrorPxiResmanSystemDescriptionParseError"; break;
    case -26400: return "DAQmxErrorPxiResmanSystemDescriptionFileNotFound"; break;
    case -26302: return "DAQmxErrorPxiResmanControllerParseError"; break;
    case -26301: return "DAQmxErrorPxiResmanControllerTypeInvalid"; break;
    case -26300: return "DAQmxErrorPxiResmanControllerFileNotFound"; break;
    case -26206: return "DAQmxErrorPxiResmanChassisTriggerParseError"; break;
    case -26205: return "DAQmxErrorPxiResmanChassisBridgeParseError"; break;
    case -26204: return "DAQmxErrorPxiResmanChassisSlotParseError"; break;
    case -26203: return "DAQmxErrorPxiResmanChassisSegmentParseError"; break;
    case -26202: return "DAQmxErrorPxiResmanChassisParseError"; break;
    case -26201: return "DAQmxErrorPxiResmanChassisBridgeNotFound"; break;
    case -26200: return "DAQmxErrorPxiResmanChassisFileNotFound"; break;
    case -26113: return "DAQmxErrorInsufficientBuffer"; break;
    case -26112: return "DAQmxErrorDirCreateFault"; break;
    case -26111: return "DAQmxErrorAccessDenied"; break;
    case -26110: return "DAQmxErrorPathNotFound"; break;
    case -26109: return "DAQmxErrorFileExists"; break;
    case -26107: return "DAQmxErrorFileDeleteFault"; break;
    case -26106: return "DAQmxErrorFileCopyFault"; break;
    case -26105: return "DAQmxErrorFileCloseFault"; break;
    case -26103: return "DAQmxErrorFileNotFound"; break;
    case -26102: return "DAQmxErrorPxiResmanSystemNotInitialized"; break;
    case -26101: return "DAQmxErrorPxiResmanInvalidConfiguration"; break;
    case -26100: return "DAQmxErrorPxiResmanAllocationError"; break;
    case -26010: return "DAQmxErrorTrigmanDisconnected"; break;
    case -26009: return "DAQmxErrorTrigmanInvalidClient"; break;
    case -26008: return "DAQmxErrorTrigmanConflictingRoute"; break;
    case -26007: return "DAQmxErrorTrigmanConflictingRouteNoUnreserve"; break;
    case -26006: return "DAQmxErrorTrigmanConflictingRouteGeneric"; break;
    case -26005: return "DAQmxErrorTrigmanLineAlreadyReserved"; break;
    case -26004: return "DAQmxErrorTrigmanLineNotReservedUnroutable"; break;
    case -26003: return "DAQmxErrorTrigmanLineNotReserved"; break;
    case -26002: return "DAQmxErrorTrigmanInvalidParameter"; break;
    case -26001: return "DAQmxErrorTrigmanUnsupportedOper"; break;
    case -26000: return "DAQmxErrorTrigmanUnknownError"; break;
    case 26000: return "DAQmxWarningTrigmanUnknownError"; break;
    case 26001: return "DAQmxWarningTrigmanUnsupportedOper"; break;
    case 26002: return "DAQmxWarningTrigmanInvalidParameter"; break;
    case 26003: return "DAQmxWarningTrigmanLineNotReserved"; break;
    case 26004: return "DAQmxWarningTrigmanLineNotReservedUnroutable"; break;
    case 26005: return "DAQmxWarningTrigmanLineAlreadyReserved"; break;
    case 26006: return "DAQmxWarningTrigmanConflictingRouteGeneric"; break;
    case 26007: return "DAQmxWarningTrigmanConflictingRouteNoUnreserve"; break;
    case 26008: return "DAQmxWarningTrigmanConflictingRoute"; break;
    case 26009: return "DAQmxWarningTrigmanInvalidClient"; break;
    case 26010: return "DAQmxWarningTrigmanDisconnected"; break;
    case 26100: return "DAQmxWarningPxiResmanAllocationError"; break;
    case 26101: return "DAQmxWarningPxiResmanInvalidConfiguration"; break;
    case 26102: return "DAQmxWarningPxiResmanSystemNotInitialized"; break;
    case 26103: return "DAQmxWarningFileNotFound"; break;
    case 26105: return "DAQmxWarningFileCloseFault"; break;
    case 26106: return "DAQmxWarningFileCopyFault"; break;
    case 26107: return "DAQmxWarningFileDeleteFault"; break;
    case 26109: return "DAQmxWarningFileExists"; break;
    case 26110: return "DAQmxWarningPathNotFound"; break;
    case 26111: return "DAQmxWarningAccessDenied"; break;
    case 26112: return "DAQmxWarningDirCreateFault"; break;
    case 26113: return "DAQmxWarningInsufficientBuffer"; break;
    case 26200: return "DAQmxWarningPxiResmanChassisFileNotFound"; break;
    case 26201: return "DAQmxWarningPxiResmanChassisBridgeNotFound"; break;
    case 26202: return "DAQmxWarningPxiResmanChassisParseError"; break;
    case 26203: return "DAQmxWarningPxiResmanChassisSegmentParseError"; break;
    case 26204: return "DAQmxWarningPxiResmanChassisSlotParseError"; break;
    case 26205: return "DAQmxWarningPxiResmanChassisBridgeParseError"; break;
    case 26206: return "DAQmxWarningPxiResmanChassisTriggerParseError"; break;
    case 26300: return "DAQmxWarningPxiResmanControllerFileNotFound"; break;
    case 26301: return "DAQmxWarningPxiResmanControllerTypeInvalid"; break;
    case 26302: return "DAQmxWarningPxiResmanControllerParseError"; break;
    case 26400: return "DAQmxWarningPxiResmanSystemDescriptionFileNotFound"; break;
    case 26401: return "DAQmxWarningPxiResmanSystemDescriptionParseError"; break;
    case 26402: return "DAQmxWarningPxiResmanSystemDescriptionWriteError"; break;
    case 26403: return "DAQmxWarningPxiResmanSystemDescriptionSpecLimitation"; break;
    case 26500: return "DAQmxWarningPxiResmanModuleParseError"; break;
    case 26550: return "DAQmxWarningPxiResmanPciDescriptionStringParseError"; break;
    case 26600: return "DAQmxWarningPxiResmanMxsPxiSystemNotFound"; break;
    case 26652: return "DAQmxWarningIOError"; break;
    case 26653: return "DAQmxWarningAttributeNotFound"; break;
    case 26654: return "DAQmxWarningChildNotFound"; break;
    case 26655: return "DAQmxWarningInconsistentFileFault"; break;
    case 26656: return "DAQmxWarningIncorrectDataType"; break;
    case 26657: return "DAQmxWarningHandlerNotFound"; break;
    case 26801: return "DAQmxWarningUnspecifiedError"; break;
    case 26802: return "DAQmxWarningTimeout"; break;
    case 26803: return "DAQmxWarningRecursiveCall"; break;
    case 26804: return "DAQmxWarningServiceNotRunning"; break;
    case 26805: return "DAQmxWarningInvalidHandle"; break;
    case 26806: return "DAQmxWarningInvalidAttribute"; break;
    case 26807: return "DAQmxWarningNotActiveResourceManager"; break;
    case 26808: return "DAQmxWarningPXISAConfigurationLocked"; break;
    case 26809: return "DAQmxWarningPXISystemDescriptionParseError"; break;
    case 26850: return "DAQmxWarningAlreadyInitialized"; break;
    case 26851: return "DAQmxWarningGenerationDisabled"; break;
    case 26852: return "DAQmxWarningFileDoesNotExist"; break;
    case 26853: return "DAQmxWarningRetryCall"; break;
    case 50000: return "DAQmxWarningPALValueConflict"; break;
    case 50001: return "DAQmxWarningPALIrrelevantAttribute"; break;
    case 50002: return "DAQmxWarningPALBadDevice"; break;
    case 50003: return "DAQmxWarningPALBadSelector"; break;
    case 50004: return "DAQmxWarningPALBadPointer"; break;
    case 50005: return "DAQmxWarningPALBadDataSize"; break;
    case 50006: return "DAQmxWarningPALBadMode"; break;
    case 50007: return "DAQmxWarningPALBadOffset"; break;
    case 50008: return "DAQmxWarningPALBadCount"; break;
    case 50009: return "DAQmxWarningPALBadReadMode"; break;
    case 50010: return "DAQmxWarningPALBadReadOffset"; break;
    case 50011: return "DAQmxWarningPALBadReadCount"; break;
    case 50012: return "DAQmxWarningPALBadWriteMode"; break;
    case 50013: return "DAQmxWarningPALBadWriteOffset"; break;
    case 50014: return "DAQmxWarningPALBadWriteCount"; break;
    case 50015: return "DAQmxWarningPALBadAddressClass"; break;
    case 50016: return "DAQmxWarningPALBadWindowType"; break;
    case 50019: return "DAQmxWarningPALBadThreadMultitask"; break;
    case 50100: return "DAQmxWarningPALResourceOwnedBySystem"; break;
    case 50101: return "DAQmxWarningPALResourceNotAvailable"; break;
    case 50102: return "DAQmxWarningPALResourceNotReserved"; break;
    case 50103: return "DAQmxWarningPALResourceReserved"; break;
    case 50104: return "DAQmxWarningPALResourceNotInitialized"; break;
    case 50105: return "DAQmxWarningPALResourceInitialized"; break;
    case 50106: return "DAQmxWarningPALResourceBusy"; break;
    case 50107: return "DAQmxWarningPALResourceAmbiguous"; break;
    case 50151: return "DAQmxWarningPALFirmwareFault"; break;
    case 50152: return "DAQmxWarningPALHardwareFault"; break;
    case 50200: return "DAQmxWarningPALOSUnsupported"; break;
    case 50202: return "DAQmxWarningPALOSFault"; break;
    case 50254: return "DAQmxWarningPALFunctionObsolete"; break;
    case 50255: return "DAQmxWarningPALFunctionNotFound"; break;
    case 50256: return "DAQmxWarningPALFeatureNotSupported"; break;
    case 50258: return "DAQmxWarningPALComponentInitializationFault"; break;
    case 50260: return "DAQmxWarningPALComponentAlreadyLoaded"; break;
    case 50262: return "DAQmxWarningPALComponentNotUnloadable"; break;
    case 50351: return "DAQmxWarningPALMemoryAlignmentFault"; break;
    case 50355: return "DAQmxWarningPALMemoryHeapNotEmpty"; break;
    case 50402: return "DAQmxWarningPALTransferNotInProgress"; break;
    case 50403: return "DAQmxWarningPALTransferInProgress"; break;
    case 50404: return "DAQmxWarningPALTransferStopped"; break;
    case 50405: return "DAQmxWarningPALTransferAborted"; break;
    case 50406: return "DAQmxWarningPALLogicalBufferEmpty"; break;
    case 50407: return "DAQmxWarningPALLogicalBufferFull"; break;
    case 50408: return "DAQmxWarningPALPhysicalBufferEmpty"; break;
    case 50409: return "DAQmxWarningPALPhysicalBufferFull"; break;
    case 50410: return "DAQmxWarningPALTransferOverwritten"; break;
    case 50411: return "DAQmxWarningPALTransferOverread"; break;
    case 50500: return "DAQmxWarningPALDispatcherAlreadyExported"; break;
    case 50551: return "DAQmxWarningPALSyncAbandoned"; break;
    default: return "Undefined Error: " + error; break;
    }
}


