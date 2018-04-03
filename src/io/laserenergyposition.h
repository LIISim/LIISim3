#ifndef LASERENERGYPOSITION_H
#define LASERENERGYPOSITION_H

#include <QObject>

#include "../database/structure/laserenergy.h"

class LaserEnergyPosition : public QObject
{
    Q_OBJECT

public:
    LaserEnergyPosition(QObject *parent = 0);
    LaserEnergy* getCurrentLUT();
    QString getCurrentLUTName();
    QString getCurrentInputChannelIdentifier();
    QString getCurrentOutputChannelIdentifier();

private:
    LaserEnergy *currentLUT;
    QString inputChannel;
    QString outputChannel;
    double lastSetpoint;

    double laserFluenceSet;
    double laserFluenceRead;
    double positionValueSet;
    double positionValueRead;

    bool ioShouldBeEnabled;

signals:
    void positionValue(double value);
    void laserEnergyValue(double value);

    //---

    void calculatedPositionValueChanged(double voltage, bool valid);
    void positionValueChanged(double voltage);

    void readedPositionValueChanged(double voltage);

    void ioEnabled(bool enabled);

    void readedPositionVoltage(double voltage, bool valid);
    void readedLaserFluence(double fluence, bool valid);
    void readedLaserFluenceAndPositionVoltage(double fluence, double position, bool valid);

public slots:
    void setLaserEnergyLUT(LaserEnergy *laserEnergyLUT);
    void setLaserEnergyLUT(QString fileName);

    void onIOSettingsChanged();

    //---

    void setLaserFluence(double fluence);

    void setAnalogInputChannel(QString channel);
    void setAnalogOutputChannel(QString channel);

    void enableIO(bool enable);

    void onLaserIOStateChanged(bool enabled);

    void onLaserAnalogValue(double value);

};

#endif // LASERENERGYPOSITION_H
