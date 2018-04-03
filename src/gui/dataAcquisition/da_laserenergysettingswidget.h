#ifndef DA_LASERENERGYSETTINGSWIDGET_H
#define DA_LASERENERGYSETTINGSWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>

#include "core.h"

#include "../utils/baseplotwidgetqwt.h"

class LaserEnergySettingsWidget : public QWidget
{
    Q_OBJECT
public:
    LaserEnergySettingsWidget(QWidget *parent = 0);

private:
    QComboBox *comboboxLUT;
    QComboBox *comboboxInput;
    QComboBox *comboboxOutput;

    QGridLayout *groupboxLayout;

    BasePlotWidgetQwt *basePlot;
    BasePlotCurve *setCurve;
    BasePlotCurve *posCurve;

public slots:
    void onDeviceListUpdate();
    void onLaserEnergyLUTUpdate();

    void onComboboxLUTCurrentIndexChanged();

    void onOK();
};


class LaserEnergyInfoWidget : public QWidget
{
    Q_OBJECT
public:
    LaserEnergyInfoWidget(QWidget *parent = 0);

private:
    enum State{
        SETPOINT,
        MANUAL
    };

    QHBoxLayout *layout;

    QLabel *labelFluence;
    QLabel *labelPosition;
    QComboBox *comboboxSetpoint;
    QLineEdit *lineeditFluence;

    State state;

};


class DA_LaserEnergySettingsWidget : public QWidget
{
    Q_OBJECT
public:
    DA_LaserEnergySettingsWidget(QWidget *parent = 0);

    LaserEnergySettingsWidget* getSettingsWidget();
    LaserEnergyInfoWidget* getInfoWidget();

private:
    QVBoxLayout *mainLayout;
    QHBoxLayout *lowerLayout;
    QTableWidget *tableWidget;

    QComboBox *comboboxSet;
    QLabel *labelActual;

    LaserEnergySettingsWidget *lesw;
    LaserEnergyInfoWidget *leiw;


private slots:
    void onCellChanged(int row, int column);

public slots:

};


class LaserEnergyControllerWidget : public QWidget
{
    Q_OBJECT
public:
    LaserEnergyControllerWidget(QWidget *parent = 0);

    double getLaserFluence();

    bool isIOEnabled();

    bool isSetpointValid(){return setpointValid;}
    double getSetpoint(){return lastSetpoint;}

private:
    QDoubleSpinBox *spinboxLaserFluence;
    QLabel *labelPositionVoltage;
    QToolButton *buttonSet;
    QToolButton *buttonEnableDisableIO;

    bool ioEnabled;

    bool setpointValid;
    double lastSetpoint;

private slots:
    void onButtonSetReleased();
    void onButtonEnableDisableIOReleased();
    void onCalculatedPositionValueChanged(double voltage, bool valid);
    void onIOEnabled(bool enabled);
    void onSpinBoxLaserFluenceValueChanged();

    void onGUISettingsChanged();

public slots:
    void setSpinBoxLaserFluenceSingleStep(double value);
};


class LaserEnergyMeasurementWidget : public QWidget
{
    Q_OBJECT
public:
    LaserEnergyMeasurementWidget(QWidget *parent = 0);

    bool isPositionValid(){return positionValid;}
    double getPosition(){return lastPosition;}

private:
    QLabel *labelMeasuredLaserFluence;

    bool positionValid;
    double lastPosition;

private slots:
    void onReadedPositionVoltage(double voltage, bool valid);
    void onReadedLaserFluence(double fluence, bool valid);
    void onReadedLaserFluenceAndPositionVoltage(double fluence, double position, bool valid);
};


#endif // DA_LASERENERGYSETTINGSWIDGET_H
