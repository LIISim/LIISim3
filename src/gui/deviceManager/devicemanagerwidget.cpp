#include "devicemanagerwidget.h"
#include "core.h"

#include <QGroupBox>
#include <QRadioButton>
#include <QGridLayout>
#include <QSpacerItem>
#include <QCheckBox>
#include "../utils/numberlineedit.h"

#include "../dataAcquisition/dataacquisitionwindow.h"

DeviceManagerDialog::DeviceManagerDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("DAQmx Device Manager"));
    setModal(true);

    layoutMain = new QHBoxLayout(this);
    setLayout(layoutMain);

    layoutLeft = new QVBoxLayout();
    layoutMain->addLayout(layoutLeft);

    listView = new QListView(this);
    listView->setMaximumWidth(200);
    layoutLeft->addWidget(listView);

    listModel = new QStringListModel(this);
    listView->setModel(listModel);
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    buttonDiscover = new QPushButton("Discover", this);
    /*if(Core::instance()->devManager->isDiscovering())
    {
        buttonDiscover->setEnabled(false);
        buttonDiscover->setText("Discovering...");
    }*/
    layoutLeft->addWidget(buttonDiscover);

    layoutMain->setAlignment(layoutLeft, Qt::AlignLeft);

    connect(Core::instance()->devManager, SIGNAL(listUpdated()), SLOT(onDeviceListUpdate()));
    connect(listView, SIGNAL(clicked(QModelIndex)), SLOT(onDeviceListClicked(QModelIndex)));
    connect(buttonDiscover, SIGNAL(released()), SLOT(onButtonDiscoverReleased()));

    layoutDeviceInfo = new QVBoxLayout();
    labelIdentifier = new QLabel("", this);
    layoutDeviceInfo->addWidget(labelIdentifier);
    labelProductType = new QLabel("Product Type:", this);
    layoutDeviceInfo->addWidget(labelProductType);
    labelSerialNumber = new QLabel("Serial Number:", this);
    layoutDeviceInfo->addWidget(labelSerialNumber);

    QLabel *labelAnalogIN = new QLabel("Analog IN:", this);
    listViewAnalogIN = new QListView(this);
    listModelAnalogIN = new QStringListModel(this);
    listViewAnalogIN->setModel(listModelAnalogIN);
    listViewAnalogIN->setMaximumWidth(300);
    layoutDeviceInfo->addWidget(labelAnalogIN);
    layoutDeviceInfo->addWidget(listViewAnalogIN);

    QLabel *labelAnalogOUT = new QLabel("Analog OUT:", this);
    listViewAnalogOUT = new QListView(this);
    listModelAnalogOUT = new QStringListModel(this);
    listViewAnalogOUT->setModel(listModelAnalogOUT);
    listViewAnalogOUT->setMaximumWidth(300);
    layoutDeviceInfo->addWidget(labelAnalogOUT);
    layoutDeviceInfo->addWidget(listViewAnalogOUT);

    QLabel *labelDigitalIN = new QLabel("Digital IN:", this);
    listViewDigitalIN = new QListView(this);
    listModelDigitalIN = new QStringListModel(this);
    listViewDigitalIN->setModel(listModelDigitalIN);
    listViewDigitalIN->setMaximumWidth(300);
    layoutDeviceInfo->addWidget(labelDigitalIN);
    layoutDeviceInfo->addWidget(listViewDigitalIN);

    QLabel *labelDigitalOUT = new QLabel("Digital OUT:", this);
    listViewDigitalOUT = new QListView(this);
    listModelDigitalOUT = new QStringListModel(this);
    listViewDigitalOUT->setModel(listModelDigitalOUT);
    listViewDigitalOUT->setMaximumWidth(300);
    layoutDeviceInfo->addWidget(labelDigitalOUT);
    layoutDeviceInfo->addWidget(listViewDigitalOUT);

    layoutMain->addLayout(layoutDeviceInfo);
    layoutMain->setAlignment(layoutDeviceInfo, Qt::AlignLeft);

    QGroupBox *groupBoxSettings = new QGroupBox("Device Manager", this);
    //groupBoxSettings->setMaximumWidth(300);
    //groupBoxSettings->setMaximumHeight(150);
    QGridLayout *gridLayoutSettings = new QGridLayout();

    checkboxAutoDiscover = new QCheckBox("Auto discover at startup", this);

    QLabel *labelAOStepSize = new QLabel("Analog out field step size", this);
    spinboxAnalogOutStepSize = new QDoubleSpinBox(this);
    spinboxAnalogOutStepSize->setRange(0.001, 0.1);
    spinboxAnalogOutStepSize->setSingleStep(0.001);
    spinboxAnalogOutStepSize->setDecimals(3);

    if(!Core::instance()->ioSettings->hasEntry("deviceManager", "StepSizeAnalogOut"))
    {
        Core::instance()->ioSettings->setValue("deviceManager", "StepSizeAnalogOut", "0.001");
    }

    spinboxAnalogOutStepSize->setValue(Core::instance()->ioSettings->value("deviceManager", "StepSizeAnalogOut").toDouble());
    spinboxAnalogOutStepSize->setMinimumWidth(100);
    spinboxAnalogOutStepSize->setMaximumWidth(100);

    QLabel *labelAOLimitMin = new QLabel("Output limit (minimum, in volt)", this);
    spinboxAnalogOutLimitMin = new QDoubleSpinBox(this);
    spinboxAnalogOutLimitMin->setRange(0, 5.0);
    spinboxAnalogOutLimitMin->setSingleStep(0.01);
    spinboxAnalogOutLimitMin->setValue(Core::instance()->devManager->getAnalogOutLimitMin());
    spinboxAnalogOutLimitMin->setMinimumWidth(100);
    spinboxAnalogOutLimitMin->setMaximumWidth(100);

    QLabel *labelAOLimitMax = new QLabel("Output limit (maximum, in volt)", this);
    spinboxAnalogOutLimitMax = new QDoubleSpinBox(this);
    spinboxAnalogOutLimitMax->setRange(0, 5.0);
    spinboxAnalogOutLimitMax->setSingleStep(0.01);
    spinboxAnalogOutLimitMax->setValue(Core::instance()->devManager->getAnalogOutLimitMax());
    spinboxAnalogOutLimitMax->setMinimumWidth(100);
    spinboxAnalogOutLimitMax->setMaximumWidth(100);

    checkboxAverageAnalogIn = new QCheckBox("Average analog input (at ~10 Hz) | Sample buffer size", this);
    checkboxAverageAnalogIn->setChecked(Core::instance()->devManager->getAverageAnalogIn());
    checkboxAverageAnalogIn->setToolTip("Analog input value shown will be an averaged value, sampled at ~10 Hz");

    spinboxAverageSamples = new QSpinBox(this);
    spinboxAverageSamples->setMinimum(1);
    spinboxAverageSamples->setValue(Core::instance()->devManager->getAnalogInAverageSampleSize());

    gridLayoutSettings->addWidget(checkboxAutoDiscover, 0, 0);
    gridLayoutSettings->addWidget(labelAOStepSize, 1, 0);
    gridLayoutSettings->addWidget(spinboxAnalogOutStepSize, 1, 1);
    gridLayoutSettings->addWidget(labelAOLimitMin, 2, 0);
    gridLayoutSettings->addWidget(spinboxAnalogOutLimitMin, 2, 1);
    gridLayoutSettings->addWidget(labelAOLimitMax, 3, 0);
    gridLayoutSettings->addWidget(spinboxAnalogOutLimitMax, 3, 1);
    gridLayoutSettings->addWidget(checkboxAverageAnalogIn, 4, 0);
    gridLayoutSettings->addWidget(spinboxAverageSamples, 4, 1);
    groupBoxSettings->setLayout(gridLayoutSettings);

    //layoutDeviceInfo->addWidget(groupBoxSettings);
    //layoutDeviceInfo->setAlignment(groupBoxSettings, Qt::AlignLeft);

    connect(checkboxAutoDiscover, SIGNAL(stateChanged(int)), SLOT(onAutoDiscoverStateChanged(int)));
    connect(Core::instance()->ioSettings, SIGNAL(settingsChanged()), SLOT(onIOSettingsChanged()));

    QGroupBox *groupBoxInput = new QGroupBox("Gain Input Channel");
    groupBoxInput->setMinimumWidth(200);
    //groupBoxInput->setMaximumWidth(300);
    //groupBoxInput->setMaximumHeight(150);
    QGridLayout *layoutInputGroupBox = new QGridLayout();

    comboboxGainAChannelIN = new QComboBox(this);
    comboboxGainBChannelIN = new QComboBox(this);
    comboboxGainCChannelIN = new QComboBox(this);
    comboboxGainDChannelIN = new QComboBox(this);
    comboboxGainAChannelIN->setMinimumWidth(100);
    comboboxGainBChannelIN->setMinimumWidth(100);
    comboboxGainCChannelIN->setMinimumWidth(100);
    comboboxGainDChannelIN->setMinimumWidth(100);
    comboboxGainAChannelIN->addItem("None");
    comboboxGainBChannelIN->addItem("None");
    comboboxGainCChannelIN->addItem("None");
    comboboxGainDChannelIN->addItem("None");

    layoutInputGroupBox->addWidget(new QLabel("Gain A", this), 0, 0);
    layoutInputGroupBox->addWidget(comboboxGainAChannelIN, 0, 1);
    layoutInputGroupBox->addWidget(new QLabel("Gain B", this), 1, 0);
    layoutInputGroupBox->addWidget(comboboxGainBChannelIN, 1, 1);
    layoutInputGroupBox->addWidget(new QLabel("Gain C", this), 2, 0);
    layoutInputGroupBox->addWidget(comboboxGainCChannelIN, 2, 1);
    layoutInputGroupBox->addWidget(new QLabel("Gain D", this), 3, 0);
    layoutInputGroupBox->addWidget(comboboxGainDChannelIN, 3, 1);

    groupBoxInput->setLayout(layoutInputGroupBox);
    //layoutMain->addWidget(groupBoxInput);
    //layoutMain->setAlignment(groupBoxInput, Qt::AlignTop | Qt::AlignRight);

    QGroupBox *groupBoxDigitalOutput = new QGroupBox("Digital Output", this);
    groupBoxDigitalOutput->setMinimumWidth(300);
    //groupBoxDigitalOutput->setMaximumWidth(300);
    //groupBoxDigitalOutput->setMaximumHeight(300);
    QGridLayout *layoutDigitalOutputBox = new QGridLayout();

    lineeditDigitalOutput1 = new QLineEdit(Core::instance()->guiSettings->value("acquisition", "buttonDO1text", "DO 1").toString(), this);
    lineeditDigitalOutput2 = new QLineEdit(Core::instance()->guiSettings->value("acquisition", "buttonDO2text", "DO 2").toString(), this);
    lineeditDigitalOutput3 = new QLineEdit(Core::instance()->guiSettings->value("acquisition", "buttonDO3text", "DO 3").toString(), this);
    lineeditDigitalOutput4 = new QLineEdit(Core::instance()->guiSettings->value("acquisition", "buttonDO4text", "DO 4").toString(), this);
    lineeditDigitalOutput5 = new QLineEdit(Core::instance()->guiSettings->value("acquisition", "buttonDO5text", "DO 5").toString(), this);
    lineeditDigitalOutput6 = new QLineEdit(Core::instance()->guiSettings->value("acquisition", "buttonDO6text", "DO 6").toString(), this);
    lineeditDigitalOutput7 = new QLineEdit(Core::instance()->guiSettings->value("acquisition", "buttonDO7text", "DO 7").toString(), this);
    lineeditDigitalOutput8 = new QLineEdit(Core::instance()->guiSettings->value("acquisition", "buttonDO8text", "DO 8").toString(), this);
    lineeditDigitalOutput9 = new QLineEdit(Core::instance()->guiSettings->value("acquisition", "buttonDO9text", "DO 9").toString(), this);
    comboboxDigitalOutput1 = new QComboBox(this);
    comboboxDigitalOutput2 = new QComboBox(this);
    comboboxDigitalOutput3 = new QComboBox(this);
    comboboxDigitalOutput4 = new QComboBox(this);
    comboboxDigitalOutput5 = new QComboBox(this);
    comboboxDigitalOutput6 = new QComboBox(this);
    comboboxDigitalOutput7 = new QComboBox(this);
    comboboxDigitalOutput8 = new QComboBox(this);
    comboboxDigitalOutput9 = new QComboBox(this);
    comboboxDigitalOutput1->setMinimumWidth(150);
    comboboxDigitalOutput2->setMinimumWidth(150);
    comboboxDigitalOutput3->setMinimumWidth(150);
    comboboxDigitalOutput4->setMinimumWidth(150);
    comboboxDigitalOutput5->setMinimumWidth(150);
    comboboxDigitalOutput6->setMinimumWidth(150);
    comboboxDigitalOutput7->setMinimumWidth(150);
    comboboxDigitalOutput8->setMinimumWidth(150);
    comboboxDigitalOutput9->setMinimumWidth(150);
    comboboxDigitalOutput1->addItem("None");
    comboboxDigitalOutput2->addItem("None");
    comboboxDigitalOutput3->addItem("None");
    comboboxDigitalOutput4->addItem("None");
    comboboxDigitalOutput5->addItem("None");
    comboboxDigitalOutput6->addItem("None");
    comboboxDigitalOutput7->addItem("None");
    comboboxDigitalOutput8->addItem("None");
    comboboxDigitalOutput9->addItem("None");
    checkboxInvertDigitalOut1 = new QCheckBox("Invert", this);
    checkboxInvertDigitalOut2 = new QCheckBox("Invert", this);
    checkboxInvertDigitalOut3 = new QCheckBox("Invert", this);
    checkboxInvertDigitalOut4 = new QCheckBox("Invert", this);
    checkboxInvertDigitalOut5 = new QCheckBox("Invert", this);
    checkboxInvertDigitalOut6 = new QCheckBox("Invert", this);
    checkboxInvertDigitalOut7 = new QCheckBox("Invert", this);
    checkboxInvertDigitalOut8 = new QCheckBox("Invert", this);
    checkboxInvertDigitalOut9 = new QCheckBox("Invert", this);
    layoutDigitalOutputBox->addWidget(lineeditDigitalOutput1, 0, 0);
    layoutDigitalOutputBox->addWidget(comboboxDigitalOutput1, 0, 1);
    layoutDigitalOutputBox->addWidget(checkboxInvertDigitalOut1, 0, 2);
    layoutDigitalOutputBox->addWidget(lineeditDigitalOutput2, 1, 0);
    layoutDigitalOutputBox->addWidget(comboboxDigitalOutput2, 1, 1);
    layoutDigitalOutputBox->addWidget(checkboxInvertDigitalOut2, 1, 2);
    layoutDigitalOutputBox->addWidget(lineeditDigitalOutput3, 2, 0);
    layoutDigitalOutputBox->addWidget(comboboxDigitalOutput3, 2, 1);
    layoutDigitalOutputBox->addWidget(checkboxInvertDigitalOut3, 2, 2);
    layoutDigitalOutputBox->addWidget(lineeditDigitalOutput4, 3, 0);
    layoutDigitalOutputBox->addWidget(comboboxDigitalOutput4, 3, 1);
    layoutDigitalOutputBox->addWidget(checkboxInvertDigitalOut4, 3, 2);
    layoutDigitalOutputBox->addWidget(lineeditDigitalOutput5, 4, 0);
    layoutDigitalOutputBox->addWidget(comboboxDigitalOutput5, 4, 1);
    layoutDigitalOutputBox->addWidget(checkboxInvertDigitalOut5, 4, 2);
    layoutDigitalOutputBox->addWidget(lineeditDigitalOutput6, 5, 0);
    layoutDigitalOutputBox->addWidget(comboboxDigitalOutput6, 5, 1);
    layoutDigitalOutputBox->addWidget(checkboxInvertDigitalOut6, 5, 2);
    layoutDigitalOutputBox->addWidget(lineeditDigitalOutput7, 6, 0);
    layoutDigitalOutputBox->addWidget(comboboxDigitalOutput7, 6, 1);
    layoutDigitalOutputBox->addWidget(checkboxInvertDigitalOut7, 6, 2);
    layoutDigitalOutputBox->addWidget(lineeditDigitalOutput8, 7, 0);
    layoutDigitalOutputBox->addWidget(comboboxDigitalOutput8, 7, 1);
    layoutDigitalOutputBox->addWidget(checkboxInvertDigitalOut8, 7, 2);
    layoutDigitalOutputBox->addWidget(lineeditDigitalOutput9, 8, 0);
    layoutDigitalOutputBox->addWidget(comboboxDigitalOutput9, 8, 1);
    layoutDigitalOutputBox->addWidget(checkboxInvertDigitalOut9, 8, 2);
    groupBoxDigitalOutput->setLayout(layoutDigitalOutputBox);

    //layoutMain->addWidget(groupBoxDigitalOutput);
    //layoutMain->setAlignment(groupBoxDigitalOutput, Qt::AlignTop | Qt::AlignRight);

    laserEnergySettingsWidget = new LaserEnergySettingsWidget(this);

    QVBoxLayout *layoutRight = new QVBoxLayout;


    //layoutMain->addWidget(laserEnergySettingsWidget);
    //layoutMain->setAlignment(laserEnergySettingsWidget, Qt::AlignTop);

    ok = new QPushButton("OK", this);
    cancel = new QPushButton("Cancel", this);

    QWidget *blub = new QWidget(this);
    QHBoxLayout *layoutOKCancel = new QHBoxLayout(blub);
    //layoutOKCancel->setMargin(0);
    layoutOKCancel->addWidget(cancel);
    layoutOKCancel->addWidget(ok);

    //layoutDeviceInfo->addWidget(blub);
    //layoutDeviceInfo->setAlignment(blub, Qt::AlignBottom | Qt::AlignRight);

    layoutRight->addWidget(groupBoxSettings);
    layoutRight->addWidget(groupBoxInput);
    layoutRight->addWidget(groupBoxDigitalOutput);
    layoutRight->addWidget(laserEnergySettingsWidget);
    layoutRight->addWidget(blub);
    layoutRight->setAlignment(blub, Qt::AlignBottom | Qt::AlignRight);
    layoutMain->addLayout(layoutRight);

    connect(ok, SIGNAL(released()), SLOT(onButtonOKReleased()));
    connect(cancel, SIGNAL(released()), SLOT(onButtonCancelReleased()));

    if(Core::instance()->ioSettings->hasEntry("deviceManager", "discoverAtStartup"))
            checkboxAutoDiscover->setChecked(Core::instance()->ioSettings->value("deviceManager", "discoverAtStartup").toBool());

    if(Core::instance()->devManager->isDiscovering())
    {
        buttonDiscover->setEnabled(false);
        buttonDiscover->setText("Discovering...");
    }

    if(Core::instance()->devManager->devices().size() > 0)
        onDeviceListUpdate();
}


void DeviceManagerDialog::onDeviceListUpdate()
{
    devList = Core::instance()->devManager->devices();

    QStringList temp;
    for(int i = 0; i < devList.size(); i++)
    {
        temp << devList.at(i).identifier;
    }
    listModel->setStringList(temp);

    comboboxGainAChannelIN->blockSignals(true);
    comboboxGainBChannelIN->blockSignals(true);
    comboboxGainCChannelIN->blockSignals(true);
    comboboxGainDChannelIN->blockSignals(true);

    comboboxGainAChannelIN->clear();
    comboboxGainBChannelIN->clear();
    comboboxGainCChannelIN->clear();
    comboboxGainDChannelIN->clear();

    comboboxGainAChannelIN->addItem("None");
    comboboxGainBChannelIN->addItem("None");
    comboboxGainCChannelIN->addItem("None");
    comboboxGainDChannelIN->addItem("None");

    for(int i = 0; i < devList.size(); i++)
    {
        for(int j = 0; j < devList.at(i).analogIn.size(); j++)
        {
            comboboxGainAChannelIN->addItem(devList.at(i).analogIn.value(j));
            comboboxGainBChannelIN->addItem(devList.at(i).analogIn.value(j));
            comboboxGainCChannelIN->addItem(devList.at(i).analogIn.value(j));
            comboboxGainDChannelIN->addItem(devList.at(i).analogIn.value(j));
        }
    }

    comboboxDigitalOutput1->clear();
    comboboxDigitalOutput2->clear();
    comboboxDigitalOutput3->clear();
    comboboxDigitalOutput4->clear();
    comboboxDigitalOutput5->clear();
    comboboxDigitalOutput6->clear();
    comboboxDigitalOutput7->clear();
    comboboxDigitalOutput8->clear();
    comboboxDigitalOutput9->clear();

    comboboxDigitalOutput1->addItem("None");
    comboboxDigitalOutput2->addItem("None");
    comboboxDigitalOutput3->addItem("None");
    comboboxDigitalOutput4->addItem("None");
    comboboxDigitalOutput5->addItem("None");
    comboboxDigitalOutput6->addItem("None");
    comboboxDigitalOutput7->addItem("None");
    comboboxDigitalOutput8->addItem("None");
    comboboxDigitalOutput9->addItem("None");

    for(int i = 0; i < devList.size(); i++)
    {
        for(int j = 0; j < devList.at(i).digitalOut.size(); j++)
        {
            comboboxDigitalOutput1->addItem(devList.at(i).digitalOut.value(j));
            comboboxDigitalOutput2->addItem(devList.at(i).digitalOut.value(j));
            comboboxDigitalOutput3->addItem(devList.at(i).digitalOut.value(j));
            comboboxDigitalOutput4->addItem(devList.at(i).digitalOut.value(j));
            comboboxDigitalOutput5->addItem(devList.at(i).digitalOut.value(j));
            comboboxDigitalOutput6->addItem(devList.at(i).digitalOut.value(j));
            comboboxDigitalOutput7->addItem(devList.at(i).digitalOut.value(j));
            comboboxDigitalOutput8->addItem(devList.at(i).digitalOut.value(j));
            comboboxDigitalOutput9->addItem(devList.at(i).digitalOut.value(j));
        }
    }

    int index;
    //if(Core::instance()->devManager->getGainInputChannelEnabled(1))
    {
        index = comboboxGainAChannelIN->findText(Core::instance()->devManager->getAnalogInChannelIdentifier(1));
        if(index != -1)
            comboboxGainAChannelIN->setCurrentIndex(index);
    }

    //if(Core::instance()->devManager->getGainInputChannelEnabled(2))
    {
        index = comboboxGainBChannelIN->findText(Core::instance()->devManager->getAnalogInChannelIdentifier(2));
        if(index != -1)
            comboboxGainBChannelIN->setCurrentIndex(index);
    }

    //if(Core::instance()->devManager->getGainInputChannelEnabled(3))
    {
        index = comboboxGainCChannelIN->findText(Core::instance()->devManager->getAnalogInChannelIdentifier(3));
        if(index != -1)
            comboboxGainCChannelIN->setCurrentIndex(index);
    }

    //if(Core::instance()->devManager->getGainInputChannelEnabled(4))
    {
        index = comboboxGainDChannelIN->findText(Core::instance()->devManager->getAnalogInChannelIdentifier(4));
        if(index != -1)
            comboboxGainDChannelIN->setCurrentIndex(index);
    }

    index = comboboxDigitalOutput1->findText(Core::instance()->devManager->getDigitalOutputChannel(1));
    if(index != -1)
        comboboxDigitalOutput1->setCurrentIndex(index);
    index = comboboxDigitalOutput2->findText(Core::instance()->devManager->getDigitalOutputChannel(2));
    if(index != -1)
        comboboxDigitalOutput2->setCurrentIndex(index);
    index = comboboxDigitalOutput3->findText(Core::instance()->devManager->getDigitalOutputChannel(3));
    if(index != -1)
        comboboxDigitalOutput3->setCurrentIndex(index);
    index = comboboxDigitalOutput4->findText(Core::instance()->devManager->getDigitalOutputChannel(4));
    if(index != -1)
        comboboxDigitalOutput4->setCurrentIndex(index);
    index = comboboxDigitalOutput5->findText(Core::instance()->devManager->getDigitalOutputChannel(5));
    if(index != -1)
        comboboxDigitalOutput5->setCurrentIndex(index);
    index = comboboxDigitalOutput6->findText(Core::instance()->devManager->getDigitalOutputChannel(6));
    if(index != -1)
        comboboxDigitalOutput6->setCurrentIndex(index);
    index = comboboxDigitalOutput7->findText(Core::instance()->devManager->getDigitalOutputChannel(7));
    if(index != -1)
        comboboxDigitalOutput7->setCurrentIndex(index);
    index = comboboxDigitalOutput8->findText(Core::instance()->devManager->getDigitalOutputChannel(8));
    if(index != -1)
        comboboxDigitalOutput8->setCurrentIndex(index);
    index = comboboxDigitalOutput9->findText(Core::instance()->devManager->getDigitalOutputChannel(9));
    if(index != -1)
        comboboxDigitalOutput9->setCurrentIndex(index);

    checkboxInvertDigitalOut1->setChecked(Core::instance()->devManager->getDigitalOutputInverted(1));
    checkboxInvertDigitalOut2->setChecked(Core::instance()->devManager->getDigitalOutputInverted(2));
    checkboxInvertDigitalOut3->setChecked(Core::instance()->devManager->getDigitalOutputInverted(3));
    checkboxInvertDigitalOut4->setChecked(Core::instance()->devManager->getDigitalOutputInverted(4));
    checkboxInvertDigitalOut5->setChecked(Core::instance()->devManager->getDigitalOutputInverted(5));
    checkboxInvertDigitalOut6->setChecked(Core::instance()->devManager->getDigitalOutputInverted(6));
    checkboxInvertDigitalOut7->setChecked(Core::instance()->devManager->getDigitalOutputInverted(7));
    checkboxInvertDigitalOut8->setChecked(Core::instance()->devManager->getDigitalOutputInverted(8));
    checkboxInvertDigitalOut9->setChecked(Core::instance()->devManager->getDigitalOutputInverted(9));

    comboboxGainAChannelIN->blockSignals(false);
    comboboxGainBChannelIN->blockSignals(false);
    comboboxGainCChannelIN->blockSignals(false);
    comboboxGainDChannelIN->blockSignals(false);

    buttonDiscover->setText("Discover");
    buttonDiscover->setEnabled(true);

    laserEnergySettingsWidget->onDeviceListUpdate();
}


void DeviceManagerDialog::onDeviceListClicked(QModelIndex index)
{
    int position;

    for(int i = 0; i < devList.size(); i++)
    {
        if(index.data().toString() == devList.at(i).identifier)
            position = i;
    }

    labelIdentifier->setText(devList.at(position).identifier);
    labelProductType->setText(QString("Product Type: %0").arg(devList.at(position).type));
    labelSerialNumber->setText(QString("Serial Number: %0").arg(devList.at(position).serialNumber));

    QStringList temp1;
    for(int i = 0; i < devList.at(position).analogIn.size(); i++)
        temp1 << devList.at(position).analogIn.value(i);
    listModelAnalogIN->setStringList(temp1);

    QStringList temp2;
    for(int i = 0; i < devList.at(position).analogOut.size(); i++)
        temp2 << devList.at(position).analogOut.value(i);
    listModelAnalogOUT->setStringList(temp2);

    QStringList temp3;
    for(int i = 0; i < devList.at(position).digitalIn.size(); i++)
        temp3 << devList.at(position).digitalIn.value(i);
    listModelDigitalIN->setStringList(temp3);

    QStringList temp4;
    for(int i = 0; i < devList.at(position).digitalOut.size(); i++)
        temp4 << devList.at(position).digitalOut.value(i);
    listModelDigitalOUT->setStringList(temp4);
}


void DeviceManagerDialog::onButtonDiscoverReleased()
{
    buttonDiscover->setEnabled(false);
    buttonDiscover->setText("Discovering...");

    QStringList temp;
    listModel->setStringList(temp);
    listModelAnalogIN->setStringList(temp);
    listModelAnalogOUT->setStringList(temp);
    listModelDigitalIN->setStringList(temp);
    listModelDigitalOUT->setStringList(temp);
    labelIdentifier->setText("");
    labelProductType->setText("Product Type:");
    labelSerialNumber->setText("Serial Number:");

    Core::instance()->devManager->discover();
}


void DeviceManagerDialog::onAutoDiscoverStateChanged(int state)
{
    if(state == Qt::Checked)
        Core::instance()->ioSettings->setValue("deviceManager", "discoverAtStartup", true);
    else if(state == Qt::Unchecked)
        Core::instance()->ioSettings->setValue("deviceManager", "discoverAtStartup", false);
}


void DeviceManagerDialog::onIOSettingsChanged()
{   
}


void DeviceManagerDialog::onButtonOKReleased()
{
    this->blockSignals(true);

    Core::instance()->ioSettings->blockSignals(true);

    if(comboboxGainAChannelIN->currentText() == "None")
        Core::instance()->devManager->setInputChannel(1, QString());
    else
        Core::instance()->devManager->setInputChannel(1, comboboxGainAChannelIN->currentText());

    if(comboboxGainBChannelIN->currentText() == "None")
        Core::instance()->devManager->setInputChannel(2, QString());
    else
        Core::instance()->devManager->setInputChannel(2, comboboxGainBChannelIN->currentText());

    if(comboboxGainCChannelIN->currentText() == "None")
        Core::instance()->devManager->setInputChannel(3, QString());
    else
        Core::instance()->devManager->setInputChannel(3, comboboxGainCChannelIN->currentText());

    if(comboboxGainDChannelIN->currentText() == "None")
        Core::instance()->devManager->setInputChannel(4, QString());
    else
        Core::instance()->devManager->setInputChannel(4, comboboxGainDChannelIN->currentText());

    if(comboboxDigitalOutput1->currentText() == "None")
        Core::instance()->devManager->setDigitalOutputChannel(1, QString(), false);
    else
        Core::instance()->devManager->setDigitalOutputChannel(1, comboboxDigitalOutput1->currentText(), checkboxInvertDigitalOut1->isChecked());

    if(comboboxDigitalOutput2->currentText() == "None")
        Core::instance()->devManager->setDigitalOutputChannel(2, QString(), false);
    else
        Core::instance()->devManager->setDigitalOutputChannel(2, comboboxDigitalOutput2->currentText(), checkboxInvertDigitalOut2->isChecked());

    if(comboboxDigitalOutput3->currentText() == "None")
        Core::instance()->devManager->setDigitalOutputChannel(3, QString(), false);
    else
        Core::instance()->devManager->setDigitalOutputChannel(3, comboboxDigitalOutput3->currentText(), checkboxInvertDigitalOut3->isChecked());

    if(comboboxDigitalOutput4->currentText() == "None")
        Core::instance()->devManager->setDigitalOutputChannel(4, QString(), false);
    else
        Core::instance()->devManager->setDigitalOutputChannel(4, comboboxDigitalOutput4->currentText(), checkboxInvertDigitalOut4->isChecked());

    if(comboboxDigitalOutput5->currentText() == "None")
        Core::instance()->devManager->setDigitalOutputChannel(5, QString(), false);
    else
        Core::instance()->devManager->setDigitalOutputChannel(5, comboboxDigitalOutput5->currentText(), checkboxInvertDigitalOut5->isChecked());

    if(comboboxDigitalOutput6->currentText() == "None")
        Core::instance()->devManager->setDigitalOutputChannel(6, QString(), false);
    else
        Core::instance()->devManager->setDigitalOutputChannel(6, comboboxDigitalOutput6->currentText(), checkboxInvertDigitalOut6->isChecked());

    if(comboboxDigitalOutput7->currentText() == "None")
        Core::instance()->devManager->setDigitalOutputChannel(7, QString(), false);
    else
        Core::instance()->devManager->setDigitalOutputChannel(7, comboboxDigitalOutput7->currentText(), checkboxInvertDigitalOut7->isChecked());

    if(comboboxDigitalOutput8->currentText() == "None")
        Core::instance()->devManager->setDigitalOutputChannel(8, QString(), false);
    else
        Core::instance()->devManager->setDigitalOutputChannel(8, comboboxDigitalOutput8->currentText(), checkboxInvertDigitalOut8->isChecked());

    if(comboboxDigitalOutput9->currentText() == "None")
        Core::instance()->devManager->setDigitalOutputChannel(9, QString(), false);
    else
        Core::instance()->devManager->setDigitalOutputChannel(9, comboboxDigitalOutput9->currentText(), checkboxInvertDigitalOut9->isChecked());

    Core::instance()->ioSettings->setValue("deviceManager", "StepSizeAnalogOut", spinboxAnalogOutStepSize->value());

    Core::instance()->ioSettings->setValue("deviceManager", "AnalogOutLimitMin", spinboxAnalogOutLimitMin->value());
    Core::instance()->ioSettings->setValue("deviceManager", "AnalogOutLimitMax", spinboxAnalogOutLimitMax->value());

    Core::instance()->devManager->setAnalogOutLimit(spinboxAnalogOutLimitMin->value(), spinboxAnalogOutLimitMax->value());

    Core::instance()->devManager->setAverageAnalogIn(checkboxAverageAnalogIn->isChecked());

    Core::instance()->guiSettings->setValue("acquisition", "buttonDO1text", lineeditDigitalOutput1->text().trimmed());
    Core::instance()->guiSettings->setValue("acquisition", "buttonDO2text", lineeditDigitalOutput2->text().trimmed());
    Core::instance()->guiSettings->setValue("acquisition", "buttonDO3text", lineeditDigitalOutput3->text().trimmed());
    Core::instance()->guiSettings->setValue("acquisition", "buttonDO4text", lineeditDigitalOutput4->text().trimmed());
    Core::instance()->guiSettings->setValue("acquisition", "buttonDO5text", lineeditDigitalOutput5->text().trimmed());
    Core::instance()->guiSettings->setValue("acquisition", "buttonDO6text", lineeditDigitalOutput6->text().trimmed());
    Core::instance()->guiSettings->setValue("acquisition", "buttonDO7text", lineeditDigitalOutput7->text().trimmed());
    Core::instance()->guiSettings->setValue("acquisition", "buttonDO8text", lineeditDigitalOutput8->text().trimmed());
    Core::instance()->guiSettings->setValue("acquisition", "buttonDO9text", lineeditDigitalOutput9->text().trimmed());

    Core::instance()->devManager->setAnalogInAverageSampleSize(spinboxAverageSamples->value());

    laserEnergySettingsWidget->onOK();

    static_cast<DataAcquisitionWindow*>(parentWidget())->onGuiSettingsChanged();

    Core::instance()->ioSettings->blockSignals(false);

    this->close();
}


void DeviceManagerDialog::onButtonCancelReleased()
{
    this->close();
}
