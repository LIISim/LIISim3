#include "laserenergyposition.h"
#include "core.h"


LaserEnergyPosition::LaserEnergyPosition(QObject *parent) : QObject(parent)
{
    currentLUT = NULL;
    inputChannel = "";
    outputChannel = "";
    lastSetpoint = 0.0;

    laserFluenceSet = 0;
    laserFluenceRead = 0;
    positionValueSet = 0;
    positionValueRead = 0;

    ioShouldBeEnabled = false;

    connect(dynamic_cast<Core*>(parent)->ioSettings, SIGNAL(settingsChanged()), SLOT(onIOSettingsChanged()));

    connect(dynamic_cast<Core*>(parent)->devManager, SIGNAL(laserAnalogInStateChanged(bool)), SLOT(onLaserIOStateChanged(bool)));
    connect(dynamic_cast<Core*>(parent)->devManager, SIGNAL(laserAnalogOutStateChanged(bool)), SLOT(onLaserIOStateChanged(bool)));

    connect(dynamic_cast<Core*>(parent)->devManager, SIGNAL(laserAnalogValue(double)), SLOT(onLaserAnalogValue(double)));
}


LaserEnergy* LaserEnergyPosition::getCurrentLUT()
{
    return currentLUT;
}


QString LaserEnergyPosition::getCurrentInputChannelIdentifier()
{
    return inputChannel;
}


QString LaserEnergyPosition::getCurrentOutputChannelIdentifier()
{
    return outputChannel;
}


QString LaserEnergyPosition::getCurrentLUTName()
{
    if(currentLUT != NULL)
        return currentLUT->name;
    else
        return "";
}


void LaserEnergyPosition::setLaserEnergyLUT(LaserEnergy *laserEnergyLUT)
{
    currentLUT = laserEnergyLUT;
}

void LaserEnergyPosition::setLaserEnergyLUT(QString fileName)
{
    int lutIndex = Core::instance()->getDatabaseManager()->indexOfLaserEnergy(fileName);
    if(lutIndex != -1)
    {
        currentLUT = Core::instance()->getDatabaseManager()->getLaserEnergy(lutIndex);

        Core::instance()->ioSettings->blockSignals(true);
        Core::instance()->ioSettings->setValue("laserenergy", "lut", currentLUT->filename);
        Core::instance()->ioSettings->blockSignals(false);
    }
}


void LaserEnergyPosition::onIOSettingsChanged()
{
    if(Core::instance()->ioSettings->hasEntry("laserenergy", "lut"))
        setLaserEnergyLUT(Core::instance()->ioSettings->value("laserenergy", "lut").toString());

    if(Core::instance()->ioSettings->hasEntry("laserenergy", "inputChannel"))
        setAnalogInputChannel(Core::instance()->ioSettings->value("laserenergy", "inputChannel").toString());

    if(Core::instance()->ioSettings->hasEntry("laserenergy", "outputChannel"))
        setAnalogOutputChannel(Core::instance()->ioSettings->value("laserenergy", "outputChannel").toString());
}


void LaserEnergyPosition::setLaserFluence(double fluence)
{
    if(currentLUT != NULL)
    {
        if(currentLUT->table.firstKey() > fluence || currentLUT->table.lastKey() < fluence)
        {
            laserFluenceSet = 0.0;
            positionValueSet = 0.0;
            emit calculatedPositionValueChanged(0.0, false);
            return;
        }

        laserFluenceSet = fluence;

        for(auto it = currentLUT->table.begin(); it != currentLUT->table.end(); ++it)
        {
            if(it.key() == fluence)
            {
                positionValueSet = it.value().first;
                emit calculatedPositionValueChanged(positionValueSet, true);
                Core::instance()->devManager->setLaserOutputVoltage(positionValueSet);
                return;
            }
        }

        double biggerPosSet = 0;
        double biggerEnergy = 0;
        double smallerPosSet = 0;
        double smallerEnergy = 0;

        bool found = false;
        for(auto it = currentLUT->table.begin(); it != currentLUT->table.end(); ++it)
        {
            if(it.key() > fluence && !found)
            {
                biggerPosSet = it.value().first;
                biggerEnergy = it.key();
                found = true;
            }
        }

        for(auto it = currentLUT->table.begin(); it != currentLUT->table.end(); ++it)
        {
            if(it.key() < fluence && it.key() > smallerEnergy)
            {
                smallerPosSet = it.value().first;
                smallerEnergy = it.key();
            }
        }

        positionValueSet = smallerPosSet + ((biggerPosSet - smallerPosSet) / (biggerEnergy - smallerEnergy)
                                           * (fluence - smallerEnergy));

        emit calculatedPositionValueChanged(positionValueSet, true);

        Core::instance()->devManager->setLaserOutputVoltage(positionValueSet);
    }
}


void LaserEnergyPosition::setAnalogInputChannel(QString channel)
{
    Core::instance()->ioSettings->blockSignals(true);
    Core::instance()->ioSettings->setValue("laserenergy", "inputChannel", channel);
    Core::instance()->ioSettings->blockSignals(false);

    inputChannel = channel;

    Core::instance()->devManager->setLaserInputChannel(channel);
}


void LaserEnergyPosition::setAnalogOutputChannel(QString channel)
{
    Core::instance()->ioSettings->blockSignals(true);
    Core::instance()->ioSettings->setValue("laserenergy", "outputChannel", channel);
    Core::instance()->ioSettings->blockSignals(false);

    outputChannel = channel;

    Core::instance()->devManager->setLaserOutputChannel(channel, lastSetpoint);
}


void LaserEnergyPosition::enableIO(bool enable)
{
    ioShouldBeEnabled = enable;
    if(enable)
    {
        Core::instance()->devManager->startLaserAnalogIN();
        Core::instance()->devManager->startLaserAnalogOUT();
    }
    else
    {
        Core::instance()->devManager->stopLaserAnalogIN();
        Core::instance()->devManager->stopLaserAnalogOUT();
    }
}

void LaserEnergyPosition::onLaserIOStateChanged(bool enabled)
{
    emit ioEnabled(enabled);
}


void LaserEnergyPosition::onLaserAnalogValue(double value)
{
    if(currentLUT != NULL)
    {
        if(currentLUT->table.first().second > value || currentLUT->table.last().second < value)
        {
            //emit readedPositionVoltage(value, false);
            //emit readedLaserFluence(0.0, false);
            emit readedLaserFluenceAndPositionVoltage(0.0, value, false);
            return;
        }

        positionValueRead = value;

        for(auto it = currentLUT->table.begin(); it != currentLUT->table.end(); ++it)
        {
            if(it.value().second == value)
            {
                laserFluenceRead = it.key();
                //emit readedPositionVoltage(value, true);
                //emit readedLaserFluence(it.key(), true);
                emit readedLaserFluenceAndPositionVoltage(it.key(), value, true);
                return;
            }
        }

        double biggerPos = 0;
        double biggerEnergy = 0;
        double smallerPos = 0;
        double smallerEnergy = 0;

        bool found = false;
        for(auto it = currentLUT->table.begin(); it != currentLUT->table.end(); ++it)
        {
            if(it.value().second > value && !found)
            {
                biggerPos = it.value().second;
                biggerEnergy = it.key();
                found = true;
            }
        }

        for(auto it = currentLUT->table.begin(); it != currentLUT->table.end(); ++it)
        {
            if(it.value().second < value && it.value().second > smallerPos)
            {
                smallerPos = it.value().second;
                smallerEnergy = it.key();
            }
        }

        laserFluenceRead = smallerEnergy + ((biggerEnergy - smallerEnergy) / (biggerPos - smallerPos)
                                            * (value - smallerPos));

        //emit readedPositionVoltage(value, true);
        //emit readedLaserFluence(laserFluenceRead, true);
        emit readedLaserFluenceAndPositionVoltage(laserFluenceRead, value, true);
    }
}


