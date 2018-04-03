#ifndef DEVICEMANAGERWIDGET_H
#define DEVICEMANAGERWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListView>
#include <QStringListModel>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QComboBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QLineEdit>

#include "core.h"
#include "io/daqdevice.h"
#include "../utils/numberlineedit.h"
#include "../dataAcquisition/da_laserenergysettingswidget.h"

class DeviceManagerDialog : public QDialog
{
    Q_OBJECT

public:
    DeviceManagerDialog(QWidget *parent = 0);

private:
    QHBoxLayout *layoutMain;
    QVBoxLayout *layoutLeft;

    QListView *listView;

    QStringListModel *listModel;

    QPushButton *buttonDiscover;

    QVBoxLayout *layoutDeviceInfo;
    QLabel *labelIdentifier;
    QLabel *labelProductType;
    QLabel *labelSerialNumber;

    QListView *listViewAnalogIN;
    QStringListModel *listModelAnalogIN;

    QListView *listViewAnalogOUT;
    QStringListModel *listModelAnalogOUT;

    QListView *listViewDigitalIN;
    QStringListModel *listModelDigitalIN;

    QListView *listViewDigitalOUT;
    QStringListModel *listModelDigitalOUT;

    QVBoxLayout *layoutOutput;

    QList<DAQDevice> devList;

    QRadioButton *radioButtonDoNothing;
    QRadioButton *radioButtonRediscoverLatest;
    QRadioButton *radioButtonDiscover;

    QCheckBox *checkboxAutoDiscover;
    QDoubleSpinBox *spinboxAnalogOutStepSize;
    QDoubleSpinBox *spinboxAnalogOutLimitMin;
    QDoubleSpinBox *spinboxAnalogOutLimitMax;
    QCheckBox *checkboxAverageAnalogIn;
    QSpinBox *spinboxAverageSamples;

    QPushButton *ok;
    QPushButton *cancel;

    QComboBox *comboboxGainAChannelIN;
    QComboBox *comboboxGainBChannelIN;
    QComboBox *comboboxGainCChannelIN;
    QComboBox *comboboxGainDChannelIN;

    QLineEdit *lineeditDigitalOutput1;
    QLineEdit *lineeditDigitalOutput2;
    QLineEdit *lineeditDigitalOutput3;
    QLineEdit *lineeditDigitalOutput4;
    QLineEdit *lineeditDigitalOutput5;
    QLineEdit *lineeditDigitalOutput6;
    QLineEdit *lineeditDigitalOutput7;
    QLineEdit *lineeditDigitalOutput8;
    QLineEdit *lineeditDigitalOutput9;

    QComboBox *comboboxDigitalOutput1;
    QComboBox *comboboxDigitalOutput2;
    QComboBox *comboboxDigitalOutput3;
    QComboBox *comboboxDigitalOutput4;
    QComboBox *comboboxDigitalOutput5;
    QComboBox *comboboxDigitalOutput6;
    QComboBox *comboboxDigitalOutput7;
    QComboBox *comboboxDigitalOutput8;
    QComboBox *comboboxDigitalOutput9;

    QCheckBox *checkboxInvertDigitalOut1;
    QCheckBox *checkboxInvertDigitalOut2;
    QCheckBox *checkboxInvertDigitalOut3;
    QCheckBox *checkboxInvertDigitalOut4;
    QCheckBox *checkboxInvertDigitalOut5;
    QCheckBox *checkboxInvertDigitalOut6;
    QCheckBox *checkboxInvertDigitalOut7;
    QCheckBox *checkboxInvertDigitalOut8;
    QCheckBox *checkboxInvertDigitalOut9;

    LaserEnergySettingsWidget *laserEnergySettingsWidget;

signals:

public slots:
    void onDeviceListUpdate();

    void onDeviceListClicked(QModelIndex index);

private slots:
    void onButtonDiscoverReleased();

    void onAutoDiscoverStateChanged(int state);

    void onIOSettingsChanged();

    void onButtonOKReleased();
    void onButtonCancelReleased();

};

#endif // DEVICEMANAGERWIDGET_H
