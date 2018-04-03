#include "picoscopesettingswidget.h"

#include <QHeaderView>

#include <QDebug>
#include <QLayout>
#include <QClipboard>

#include "../../core.h"

PicoScopeSettingsWidget::PicoScopeSettingsWidget(PicoScopeSettings *psSettings,PicoScope* device, QWidget *parent)
    : QTableWidget(parent),
      picoscope(device),
      psSettings(psSettings)
{
    offsetBoundsInitialUpdated = false;

    setRowCount(15);
    setColumnCount(7);

    setShowGrid(false);

    for(int i = 0; i < rowCount(); i++)
        setRowHeight(i,17);

    setColumnWidth(0, 70);
    setColumnWidth(1, 90);
    setColumnWidth(2, 80);
    setColumnWidth(3, 80);
    setColumnWidth(5, 90);
    setColumnWidth(6, 80);
    setMinimumHeight(300);

    verticalHeader()->setVisible(false);
    QStringList mt_horizontalHeaders;
    mt_horizontalHeaders << "Plot" << "Channel" << "Range" << "Offset (V)" << " Gain AO Channel" << "Gain (V)" << "Measured (V)";
    setHorizontalHeaderLabels(mt_horizontalHeaders);


    //setItem(0,1,new QTableWidgetItem("Channel Enabled"));
    setItem(4,5,new QTableWidgetItem("Gain Voltage [V]"));
    item(4,5)->setIcon(QIcon(Core::rootDir + "resources/icons/arrow_refresh.png"));
    //setItem(0, 2, new QTableWidgetItem("Range"));
    //setItem(0, 3, new QTableWidgetItem("Offset (V)"));

    /*
    QFont boldfont;
    boldfont.setBold(true);
    item(0,0)->setFont(boldfont);
    item(0,1)->setFont(boldfont);
    */

    //item(0,1)->setFlags(Qt::ItemIsEnabled);
    //item(0,2)->setFlags(Qt::ItemIsEnabled);
    //item(0,3)->setFlags(Qt::ItemIsEnabled);
    item(4,5)->setFlags(Qt::ItemIsEnabled);

    wvcChannelA = new ChannelVisualControlWidget(this);
    wvcChannelB = new ChannelVisualControlWidget(this);
    wvcChannelC = new ChannelVisualControlWidget(this);
    wvcChannelD = new ChannelVisualControlWidget(this);

    setCellWidget(0, 0, wvcChannelA);
    setCellWidget(1, 0, wvcChannelB);
    setCellWidget(2, 0, wvcChannelC);
    setCellWidget(3, 0, wvcChannelD);

    channelA = new QCheckBox("Channel A");
    channelB = new QCheckBox("Channel B");
    channelC = new QCheckBox("Channel C");
    channelD = new QCheckBox("Channel D");

    setCellWidget(0,1,channelA);
    setCellWidget(1,1,channelB);
    setCellWidget(2,1,channelC);
    setCellWidget(3,1,channelD);

    /*gainChannelA = new NumberLineEdit(NumberLineEdit::DOUBLE,"Gain Value for Channel A");
    gainChannelB = new NumberLineEdit(NumberLineEdit::DOUBLE,"Gain Value for Channel A");
    gainChannelC = new NumberLineEdit(NumberLineEdit::DOUBLE,"Gain Value for Channel A");
    gainChannelD = new NumberLineEdit(NumberLineEdit::DOUBLE,"Gain Value for Channel A");

    gainChannelA->setValue(0);
    gainChannelB->setValue(0);
    gainChannelC->setValue(0);
    gainChannelD->setValue(0);*/

    /*
    gainChannelA->setStyleSheet(this->styleSheet() + "QLineEdit { border: 0px }");
    gainChannelB->setStyleSheet(this->styleSheet() + "QLineEdit { border: 0px }");
    gainChannelC->setStyleSheet(this->styleSheet() + "QLineEdit { border: 0px }");
    gainChannelD->setStyleSheet(this->styleSheet() + "QLineEdit { border: 0px }");
    */

    /*setCellWidget(1,2,gainChannelA);
    setCellWidget(2,2,gainChannelB);
    setCellWidget(3,2,gainChannelC);
    setCellWidget(4,2,gainChannelD);*/

    cbRangeChannelA = new QComboBox(this);
    cbRangeChannelB = new QComboBox(this);
    cbRangeChannelC = new QComboBox(this);
    cbRangeChannelD = new QComboBox(this);

    setCellWidget(0, 2, cbRangeChannelA);
    setCellWidget(1, 2, cbRangeChannelB);
    setCellWidget(2, 2, cbRangeChannelC);
    setCellWidget(3, 2, cbRangeChannelD);

    linkChannelSettings = new QCheckBox("Link range", this);
    setCellWidget(4, 2, linkChannelSettings);

    /*nleOffsetChannelA = new NumberLineEdit(NumberLineEdit::DOUBLE, "Offset value for channel A in volt", this);
    nleOffsetChannelB = new NumberLineEdit(NumberLineEdit::DOUBLE, "Offset value for channel B in volt", this);
    nleOffsetChannelC = new NumberLineEdit(NumberLineEdit::DOUBLE, "Offset value for channel C in volt", this);
    nleOffsetChannelD = new NumberLineEdit(NumberLineEdit::DOUBLE, "Offset value for channel D in volt", this);*/

    /*setCellWidget(1, 4, nleOffsetChannelA);
    setCellWidget(2, 4, nleOffsetChannelB);
    setCellWidget(3, 4, nleOffsetChannelC);
    setCellWidget(4, 4, nleOffsetChannelD);*/

    spinboxOffsetChannelA = new QDoubleSpinBox(this);
    spinboxOffsetChannelB = new QDoubleSpinBox(this);
    spinboxOffsetChannelC = new QDoubleSpinBox(this);
    spinboxOffsetChannelD = new QDoubleSpinBox(this);
    spinboxOffsetChannelA->setSingleStep(0.0001);
    spinboxOffsetChannelB->setSingleStep(0.0001);
    spinboxOffsetChannelC->setSingleStep(0.0001);
    spinboxOffsetChannelD->setSingleStep(0.0001);
    spinboxOffsetChannelA->setDecimals(4);
    spinboxOffsetChannelB->setDecimals(4);
    spinboxOffsetChannelC->setDecimals(4);
    spinboxOffsetChannelD->setDecimals(4);
    setCellWidget(0, 3, spinboxOffsetChannelA);
    setCellWidget(1, 3, spinboxOffsetChannelB);
    setCellWidget(2, 3, spinboxOffsetChannelC);
    setCellWidget(3, 3, spinboxOffsetChannelD);

#ifdef LIISIM_NIDAQMX
    comboboxGainOUTChannelA = new QComboBox(this);
    comboboxGainOUTChannelA->addItem("Manual");
    comboboxGainOUTChannelB = new QComboBox(this);
    comboboxGainOUTChannelB->addItem("Manual");
    comboboxGainOUTChannelC = new QComboBox(this);
    comboboxGainOUTChannelC->addItem("Manual");
    comboboxGainOUTChannelD = new QComboBox(this);
    comboboxGainOUTChannelD->addItem("Manual");

    setCellWidget(0, 4, comboboxGainOUTChannelA);
    setCellWidget(1, 4, comboboxGainOUTChannelB);
    setCellWidget(2, 4, comboboxGainOUTChannelC);
    setCellWidget(3, 4, comboboxGainOUTChannelD);

    spinboxGainChannelA = new QDoubleSpinBox(this);
    spinboxGainChannelB = new QDoubleSpinBox(this);
    spinboxGainChannelC = new QDoubleSpinBox(this);
    spinboxGainChannelD = new QDoubleSpinBox(this);


    spinboxGainChannelA->setRange(Core::instance()->devManager->getAnalogOutLimitMin(), Core::instance()->devManager->getAnalogOutLimitMax());
    spinboxGainChannelA->setDecimals(3);
    spinboxGainChannelA->setSingleStep(0.01);
    spinboxGainChannelB->setRange(Core::instance()->devManager->getAnalogOutLimitMin(), Core::instance()->devManager->getAnalogOutLimitMax());
    spinboxGainChannelB->setDecimals(3);
    spinboxGainChannelB->setSingleStep(0.01);
    spinboxGainChannelC->setRange(Core::instance()->devManager->getAnalogOutLimitMin(), Core::instance()->devManager->getAnalogOutLimitMax());
    spinboxGainChannelC->setDecimals(3);
    spinboxGainChannelC->setSingleStep(0.01);
    spinboxGainChannelD->setRange(Core::instance()->devManager->getAnalogOutLimitMin(), Core::instance()->devManager->getAnalogOutLimitMax());
    spinboxGainChannelD->setDecimals(3);
    spinboxGainChannelD->setSingleStep(0.01);

    setCellWidget(0, 5, spinboxGainChannelA);
    setCellWidget(1, 5, spinboxGainChannelB);
    setCellWidget(2, 5, spinboxGainChannelC);
    setCellWidget(3, 5, spinboxGainChannelD);

    QString curAnalogOutStepSize = Core::instance()->ioSettings->value("deviceManager", "StepSizeAnalogOut").toString();

    setItem(5,5,new QTableWidgetItem("Step: " + curAnalogOutStepSize));
    item(5,5)->setIcon(QIcon(Core::rootDir + "resources/icons/arrow_refresh.png"));
    item(5,5)->setFlags(Qt::ItemIsEnabled);
    stepSizeSwitch = item(5, 5);

    setSpan(8,5,1,2);
    setItem(8,5,new QTableWidgetItem("Automatic Sequence:"));
    item(8,5)->setFlags(Qt::ItemIsEnabled);

    setItem(9,5,new QTableWidgetItem("Blocks"));
    item(9,5)->setFlags(Qt::ItemIsEnabled);

    comboboxBlockSequenceBlockNumber= new QComboBox(this);
    comboboxBlockSequenceBlockNumber->addItem("1");
    comboboxBlockSequenceBlockNumber->addItem("2");
    comboboxBlockSequenceBlockNumber->addItem("3");
    setCellWidget(9, 6, comboboxBlockSequenceBlockNumber);

    setItem(10,5,new QTableWidgetItem("Decrement (V)"));
    item(10,5)->setFlags(Qt::ItemIsEnabled);

    spinboxBlockSequenceDecrement = new QDoubleSpinBox(this);
    spinboxBlockSequenceDecrement->setDecimals(3);
    spinboxBlockSequenceDecrement->setSingleStep(0.001);
    spinboxBlockSequenceDecrement->setMinimum(0.001);
    spinboxBlockSequenceDecrement->setValue(0.01);
    setCellWidget(10, 6, spinboxBlockSequenceDecrement);

    setItem(11,5,new QTableWidgetItem("Delay (s)"));
    item(11,5)->setFlags(Qt::ItemIsEnabled);

    spinboxBlockSequenceDelay = new QDoubleSpinBox(this);
    spinboxBlockSequenceDelay->setDecimals(1);
    spinboxBlockSequenceDelay->setSingleStep(0.1);
    spinboxBlockSequenceDelay->setMinimum(0.1);
    spinboxBlockSequenceDelay->setValue(5.0);
    setCellWidget(11, 6, spinboxBlockSequenceDelay);

    setItem(12,5,new QTableWidgetItem("Time left"));
    item(12,5)->setFlags(Qt::ItemIsEnabled);

    labelBlockSequenceTimeLeft = new QLabel("-", this);
    setCellWidget(12, 6, labelBlockSequenceTimeLeft);

    setItem(13,5,new QTableWidgetItem("Samples left"));
    item(13,5)->setFlags(Qt::ItemIsEnabled);

    labelBlockSequenceSamplesLeft = new QLabel("-", this);
    setCellWidget(13, 6, labelBlockSequenceSamplesLeft);

    labelGainINChannelA = new QLabel("-", this);
    labelGainINChannelB = new QLabel("-", this);
    labelGainINChannelC = new QLabel("-", this);
    labelGainINChannelD = new QLabel("-", this);
    labelGainINChannelA->setContentsMargins(5, 0, 0, 0);
    labelGainINChannelB->setContentsMargins(5, 0, 0, 0);
    labelGainINChannelC->setContentsMargins(5, 0, 0, 0);
    labelGainINChannelD->setContentsMargins(5, 0, 0, 0);

    //setCellWidget(0, 6, new QLabel("Measured Voltage", this));
    setCellWidget(0, 6, labelGainINChannelA);
    setCellWidget(1, 6, labelGainINChannelB);
    setCellWidget(2, 6, labelGainINChannelC);
    setCellWidget(3, 6, labelGainINChannelD);

    connect(Core::instance()->devManager, SIGNAL(listUpdated()), SLOT(onDeviceListUpdated()));

    connect(comboboxGainOUTChannelA, SIGNAL(currentIndexChanged(int)), SLOT(onComboboxGainChannelChanged()));
    connect(comboboxGainOUTChannelB, SIGNAL(currentIndexChanged(int)), SLOT(onComboboxGainChannelChanged()));
    connect(comboboxGainOUTChannelC, SIGNAL(currentIndexChanged(int)), SLOT(onComboboxGainChannelChanged()));
    connect(comboboxGainOUTChannelD, SIGNAL(currentIndexChanged(int)), SLOT(onComboboxGainChannelChanged()));

    connect(spinboxGainChannelA, SIGNAL(valueChanged(double)), SLOT(onSpinboxGainValueChanged()));
    connect(spinboxGainChannelB, SIGNAL(valueChanged(double)), SLOT(onSpinboxGainValueChanged()));
    connect(spinboxGainChannelC, SIGNAL(valueChanged(double)), SLOT(onSpinboxGainValueChanged()));
    connect(spinboxGainChannelD, SIGNAL(valueChanged(double)), SLOT(onSpinboxGainValueChanged()));

    connect(Core::instance()->devManager, SIGNAL(analogOutputLimitChanged()), SLOT(onAnalogOutLimitChanged()));

    Core::instance()->devManager->setOutputVoltage(1, spinboxGainChannelA->value());
    Core::instance()->devManager->setOutputVoltage(2, spinboxGainChannelB->value());
    Core::instance()->devManager->setOutputVoltage(3, spinboxGainChannelC->value());
    Core::instance()->devManager->setOutputVoltage(4, spinboxGainChannelD->value());

    connect(Core::instance()->devManager, SIGNAL(analogInputReset()), SLOT(onAnalogInputReset()));
#endif

    setRowHeight(6,10);
    setSpan(6,0,1,4);
    setItem(6,0,new QTableWidgetItem(""));
    item(6,0)->setFlags(Qt::ItemIsEnabled);


    setItem(7,0,new QTableWidgetItem("Coupling"));
    item(7,0)->setFlags(Qt::ItemIsEnabled);
    setSpan(7, 0, 1, 2);
    cbcoupling = new QComboBox;
    cbcoupling->setMaximumWidth(90);
    setCellWidget(7,2,cbcoupling);
    setSpan(7, 2, 1, 2);
    //cbcoupling->setStyleSheet(this->styleSheet() + "QComboBox { margin: 1px; padding: 1px }");

    setItem(8, 0, new QTableWidgetItem("Collection Time ( / Div)"));
    item(8, 0)->setFlags(Qt::ItemIsEnabled);
    setSpan(8, 0, 1, 2);
    lecollectiontime = new QLineEdit();
    //setCellWidget(8, 2, lecollectiontime);

    cbcollectiontimemagnitude = new QComboBox;
    //cbcollectiontimemagnitude->setMaximumWidth(40);
    cbcollectiontimemagnitude->addItem("ms", 3);
    cbcollectiontimemagnitude->addItem("µs", 2);
    cbcollectiontimemagnitude->addItem("ns", 1);
    //setCellWidget(8, 3, cbcollectiontimemagnitude);

    QWidget *widgetCT = new QWidget(this);
    widgetCT->setLayout(new QHBoxLayout);
    widgetCT->layout()->setMargin(0);
    widgetCT->layout()->addWidget(lecollectiontime);
    widgetCT->layout()->addWidget(cbcollectiontimemagnitude);
    widgetCT->setMaximumWidth(90);

    setCellWidget(8, 2, widgetCT);
    setSpan(8, 2, 1, 2);

    setItem(9, 0, new QTableWidgetItem("Sample Intervall"));
    item(9, 0)->setFlags(Qt::ItemIsEnabled);
    setSpan(9, 0, 1, 2);
    cbsampleintervall = new QComboBox;
    cbsampleintervall->setMaximumWidth(90);
    setCellWidget(9, 2, cbsampleintervall);
    setSpan(9, 2, 1, 2);
    /*
     * Adding timebase descriptions / values to combo box
     * To add further timebases: calculate timebase according to
     * Picoscope Programmer's Guide, Section 3.7: Timebases
     */
    cbsampleintervall->addItem("200 ps", 0);
    cbsampleintervall->addItem("400 ps", 1);
    cbsampleintervall->addItem("800 ps", 2);
    cbsampleintervall->addItem("1.6 ns", 3);
    cbsampleintervall->addItem("3.2 ns", 4);
    cbsampleintervall->addItem("6.4 ns", 5);
    cbsampleintervall->addItem("12.8 ns", 6);
    cbsampleintervall->addItem("19.2 ns", 7);
    cbsampleintervall->addItem("25.6 ns", 8);
    cbsampleintervall->addItem("32 ns", 9);
    cbsampleintervall->addItem("38.4 ns", 10);


    setItem(10, 0, new QTableWidgetItem("Captures"));
    item(10, 0)->setFlags(Qt::ItemIsEnabled);
    setSpan(10, 0, 1, 2);
    sbcaptures = new QSpinBox;
    sbcaptures->setMinimumWidth(90);
    sbcaptures->setMinimum(1);
    sbcaptures->setMaximum(10000);
    //setCellWidget(10,2,sbcaptures);
    checkboxAverageCaptures = new QCheckBox("Average", this);
    //setCellWidget(10, 3, checkboxAverageCaptures);

    QWidget *widgetCaptures = new QWidget(this);
    widgetCaptures->setLayout(new QHBoxLayout);
    widgetCaptures->layout()->setMargin(0);
    widgetCaptures->layout()->addWidget(sbcaptures);
    widgetCaptures->layout()->addWidget(checkboxAverageCaptures);
    //widgetCaptures->setMaximumWidth(130);

    setCellWidget(10, 2, widgetCaptures);
    setSpan(10, 2, 1, 2);


    setItem(11, 0, new QTableWidgetItem("Presample (%)"));
    item(11, 0)->setFlags(Qt::ItemIsEnabled);
    setSpan(11, 0, 1, 2);
    sbpresample = new QSpinBox;
    sbpresample->setMaximumWidth(90);
    sbpresample->setMinimum(0);
    sbpresample->setMaximum(100);
    setCellWidget(11, 2, sbpresample);
    setSpan(11, 2, 1, 2);



    // CONNECTIONS

    connect(wvcChannelA->buttonVisible, SIGNAL(released()), SLOT(onChannelVisibilityChanged()));
    connect(wvcChannelB->buttonVisible, SIGNAL(released()), SLOT(onChannelVisibilityChanged()));
    connect(wvcChannelC->buttonVisible, SIGNAL(released()), SLOT(onChannelVisibilityChanged()));
    connect(wvcChannelD->buttonVisible, SIGNAL(released()), SLOT(onChannelVisibilityChanged()));

    connect(wvcChannelA->buttonLayerUp, SIGNAL(released()), SLOT(onChannelLayerUp()));
    connect(wvcChannelB->buttonLayerUp, SIGNAL(released()), SLOT(onChannelLayerUp()));
    connect(wvcChannelC->buttonLayerUp, SIGNAL(released()), SLOT(onChannelLayerUp()));
    connect(wvcChannelD->buttonLayerUp, SIGNAL(released()), SLOT(onChannelLayerUp()));

    connect(wvcChannelA->buttonLayerDown, SIGNAL(released()), SLOT(onChannelLayerDown()));
    connect(wvcChannelB->buttonLayerDown, SIGNAL(released()), SLOT(onChannelLayerDown()));
    connect(wvcChannelC->buttonLayerDown, SIGNAL(released()), SLOT(onChannelLayerDown()));
    connect(wvcChannelD->buttonLayerDown, SIGNAL(released()), SLOT(onChannelLayerDown()));

    connect(channelA, SIGNAL(stateChanged(int)), this, SLOT(channelAchecked(int)));
    connect(channelB, SIGNAL(stateChanged(int)), this, SLOT(channelBchecked(int)));
    connect(channelC, SIGNAL(stateChanged(int)), this, SLOT(channelCchecked(int)));
    connect(channelD, SIGNAL(stateChanged(int)), this, SLOT(channelDchecked(int)));

    connect(this, SIGNAL(cellClicked(int,int)), SLOT(onCellClicked(int,int)));

    //connect(cbrange, SIGNAL(currentIndexChanged(int)), this, SLOT(rangeChanged(int)));
    connect(cbcoupling, SIGNAL(currentIndexChanged(int)), this, SLOT(couplingChanged(int)));
    //connect(leoffset, SIGNAL(editingFinished()),SLOT(offsetChanged()));

    connect(lecollectiontime, SIGNAL(textEdited(QString)), this, SLOT(collectionTimeChanged(QString)));
    connect(cbcollectiontimemagnitude, SIGNAL(currentIndexChanged(QString)), this, SLOT(collectionTimeChanged(QString)));
    connect(cbsampleintervall, SIGNAL(currentIndexChanged(int)), this, SLOT(sampleIntervallChanged(int)));

    connect(sbcaptures, SIGNAL(valueChanged(int)), this, SLOT(capturesChanged(int)));

    // clear streaming average when captures are changed
    connect(sbcaptures, SIGNAL(valueChanged(int)), picoscope, SLOT(clearStreamingAvg()));

    connect(sbpresample, SIGNAL(valueChanged(int)), this, SLOT(presampleChanged(int)));

    connect(psSettings, SIGNAL(settingsChanged()), SLOT(updateGUI()));
    //connect(psSettings,SIGNAL(settingsChangedDriver()), SLOT(updateGUI()));
    //connect(psSettings,SIGNAL(settingsChangedUI()), SLOT(updateGUI()));
    connect(cbRangeChannelA, SIGNAL(currentIndexChanged(int)), this, SLOT(rangeChanged2(int)));
    connect(cbRangeChannelB, SIGNAL(currentIndexChanged(int)), this, SLOT(rangeChanged2(int)));
    connect(cbRangeChannelC, SIGNAL(currentIndexChanged(int)), this, SLOT(rangeChanged2(int)));
    connect(cbRangeChannelD, SIGNAL(currentIndexChanged(int)), this, SLOT(rangeChanged2(int)));

    /*connect(nleOffsetChannelA, SIGNAL(textEdited(QString)), SLOT(onOffsetEdited(QString)));
    connect(nleOffsetChannelB, SIGNAL(textEdited(QString)), SLOT(onOffsetEdited(QString)));
    connect(nleOffsetChannelC, SIGNAL(textEdited(QString)), SLOT(onOffsetEdited(QString)));
    connect(nleOffsetChannelD, SIGNAL(textEdited(QString)), SLOT(onOffsetEdited(QString)));*/

    connect(spinboxOffsetChannelA, SIGNAL(valueChanged(double)), SLOT(onOffsetValueChanged(double)));
    connect(spinboxOffsetChannelB, SIGNAL(valueChanged(double)), SLOT(onOffsetValueChanged(double)));
    connect(spinboxOffsetChannelC, SIGNAL(valueChanged(double)), SLOT(onOffsetValueChanged(double)));
    connect(spinboxOffsetChannelD, SIGNAL(valueChanged(double)), SLOT(onOffsetValueChanged(double)));

    connect(linkChannelSettings, SIGNAL(stateChanged(int)), SLOT(onLinkSettingsStateChanged(int)));
    connect(checkboxAverageCaptures, SIGNAL(stateChanged(int)), SLOT(onAverageCapturesStateChanged()));

    connect(comboboxBlockSequenceBlockNumber, SIGNAL(currentIndexChanged(int)), SLOT(onBlockSequenceBlockNumber(int)));
    connect(spinboxBlockSequenceDecrement, SIGNAL(valueChanged(double)), SLOT(onBlockSequenceDecrementChanged(double)));
    connect(spinboxBlockSequenceDelay, SIGNAL(valueChanged(double)), SLOT(onBlockSequenceDelayChanged(double)));

    connect(psSettings, SIGNAL(rangeChanged(PSChannel)), SLOT(onSettingsRangeChanged(PSChannel)));
    connect(Core::instance()->ioSettings, SIGNAL(settingsChanged()), SLOT(onIOSettingsChanged()));
    connect(Core::instance()->guiSettings, SIGNAL(settingsChanged()), SLOT(onGUISettingsChanged()));

    onIOSettingsChanged();
}


PicoScopeSettingsWidget::~PicoScopeSettingsWidget()
{

}

// -------
// GETTERS
// -------


/**
 * @brief PicoScopeSettingsWidget::gainA get gain
 * value of channel A from GUI
 * @return gain value
 */
float PicoScopeSettingsWidget::gainA()
{
    //return (float)gainChannelA->getValue();
    return spinboxGainChannelA->value();
}

/**
 * @brief PicoScopeSettingsWidget::gainB get gain
 * value of channel B from GUI
 * @return gain value
 */
float PicoScopeSettingsWidget::gainB()
{
    //return (float)gainChannelB->getValue();
    return spinboxGainChannelB->value();
}

/**
 * @brief PicoScopeSettingsWidget::gainC get gain
 * value of channel C from GUI
 * @return gain value
 */
float PicoScopeSettingsWidget::gainC()
{
    //return (float)gainChannelC->getValue();
    return spinboxGainChannelC->value();
}

/**
 * @brief PicoScopeSettingsWidget::gainD get gain
 * value of channel D from GUI
 * @return gain value
 */
float PicoScopeSettingsWidget::gainD()
{
    //return (float)gainChannelD->getValue();
    return spinboxGainChannelD->value();
}

/**
 * @brief PicoScopeSettingsWidget::getGainList returns list of gain values
 * @return
 */
QList<double> PicoScopeSettingsWidget::getGainList()
{
    QList<double> list;

    qDebug() << "PicoScopeSettingsWidget::getGainList(): " << spinboxGainChannelA->value() << spinboxGainChannelB->value() << spinboxGainChannelC->value() << spinboxGainChannelD->value();

    list << spinboxGainChannelA->value()
        << spinboxGainChannelB->value()
        << spinboxGainChannelC->value()
        << spinboxGainChannelD->value();

    return list;
}


QList<double> PicoScopeSettingsWidget::getGainReferenceList()
{
    QList<double> list;

    list << labelGainINChannelA->text().toDouble()
        << labelGainINChannelB->text().toDouble()
        << labelGainINChannelC->text().toDouble()
        << labelGainINChannelD->text().toDouble();

    return list;
}


int PicoScopeSettingsWidget::getBlockSequenceBlockNumber()
{
    return comboboxBlockSequenceBlockNumber->currentText().toInt();
}


float PicoScopeSettingsWidget::getBlockSequenceVoltageDecrement()
{
    return spinboxBlockSequenceDecrement->value();
}


float PicoScopeSettingsWidget::getBlockSequenceDelaySec()
{
    return spinboxBlockSequenceDelay->value();
}


// ---------------------------------------------
// SLOTS TO UPDATE SETTINGS BASED ON GUI CHANGES
// ---------------------------------------------


/**
 * @brief PicoScopeSettingsWidget::channelAchecked Called when the state of the channel A
 * checkbox is changed and adjusts the settings
 * @param state
 */
void PicoScopeSettingsWidget::channelAchecked(int state)
{
    if(state == Qt::Checked)
        psSettings->setChannel(PSChannel::A, true, PSSignalSource::UI);
    else if(state == Qt::Unchecked)
        psSettings->setChannel(PSChannel::A, false, PSSignalSource::UI);
}


/**
 * @brief PicoScopeSettingsWidget::channelBchecked Called when the state of the channel B
 * checkbox is changed and adjusts the settings
 * @param state
 */
void PicoScopeSettingsWidget::channelBchecked(int state)
{
    if(state == Qt::Checked)
        psSettings->setChannel(PSChannel::B, true, PSSignalSource::UI);
    else if(state == Qt::Unchecked)
        psSettings->setChannel(PSChannel::B, false, PSSignalSource::UI);
}


/**
 * @brief PicoScopeSettingsWidget::channelCchecked Called when the state of the channel C
 * checkbox is changed and adjusts the settings
 * @param state
 */
void PicoScopeSettingsWidget::channelCchecked(int state)
{
    if(state == Qt::Checked)
        psSettings->setChannel(PSChannel::C, true, PSSignalSource::UI);
    else if(state == Qt::Unchecked)
        psSettings->setChannel(PSChannel::C, false, PSSignalSource::UI);
}


/**
 * @brief PicoScopeSettingsWidget::channelDchecked Called when the state of the channel D
 * checkbox is changed and adjusts the settings
 * @param state
 */
void PicoScopeSettingsWidget::channelDchecked(int state)
{
    if(state == Qt::Checked)
        psSettings->setChannel(PSChannel::D, true, PSSignalSource::UI);
    else if(state == Qt::Unchecked)
        psSettings->setChannel(PSChannel::D, false, PSSignalSource::UI);
}


/**
 * @brief PicoScopeSettingsWidget::rangeChanged Executed when the value in the range combo box
 * is changed;
 * @param index
 */
/*void PicoScopeSettingsWidget::rangeChanged(int index)
{
    psSettings->setRange(psSettings->intToPSRange(index), PSSignalSource::UI);
    updateCouplingCB(psSettings->availableCoupling(), (int)psSettings->coupling());
}*/


void PicoScopeSettingsWidget::rangeChanged2(int index)
{
    if(linkChannelSettings->isChecked())
    {
        psSettings->setRange(PSChannel::A, psSettings->intToPSRange(index), PSSignalSource::UI);
        psSettings->setRange(PSChannel::B, psSettings->intToPSRange(index), PSSignalSource::UI);
        psSettings->setRange(PSChannel::C, psSettings->intToPSRange(index), PSSignalSource::UI);
        psSettings->setRange(PSChannel::D, psSettings->intToPSRange(index), PSSignalSource::UI);
        updateRange(psSettings->availableRange());
    }
    else
    {
        if(QObject::sender() == cbRangeChannelA)
            psSettings->setRange(PSChannel::A, psSettings->intToPSRange(index), PSSignalSource::UI);
        if(QObject::sender() == cbRangeChannelB)
            psSettings->setRange(PSChannel::B, psSettings->intToPSRange(index), PSSignalSource::UI);
        if(QObject::sender() == cbRangeChannelC)
            psSettings->setRange(PSChannel::C, psSettings->intToPSRange(index), PSSignalSource::UI);
        if(QObject::sender() == cbRangeChannelD)
            psSettings->setRange(PSChannel::D, psSettings->intToPSRange(index), PSSignalSource::UI);
    }
}


/**
 * @brief PicoScopeSettingsWidget::couplingChanged Executed when the value in the coupling combo
 * box is changed
 * @param index
 */
void PicoScopeSettingsWidget::couplingChanged(int index)
{
    psSettings->setCoupling(psSettings->intToPSCoupling(index), PSSignalSource::UI);
    updateRange(psSettings->availableRange());
}


/*void PicoScopeSettingsWidget::offsetChanged()
{
    float offset = (float) leoffset->getValue();
    float min;
    float max;
    picoscope->offsetMaxMin(&max, &min);

    if(offset > max || offset < min)
    {
        QString msg = QString("[PicoScope] Offset not in range, max ").append(QString::number(max)).append(", min ").append(QString::number(min));
        MSG_STATUS(msg);
        qDebug() << msg;
    }
    else
    {
        psSettings->setOffset(offset, PSSignalSource::UI);
    }
}*/


void PicoScopeSettingsWidget::collectionTimeChanged(QString text)
{
    bool ok;

    double collectionTime = lecollectiontime->text().toDouble(&ok);

    if(ok)
    {
        if(collectionTime <= 0)
        {
            lecollectiontime->setStyleSheet("QLineEdit { background: #FF8C8C }");
        }
        else
        {
            switch(cbcollectiontimemagnitude->currentData().toInt())
            {
            case 1:
                collectionTime *= pow(10, -9);
                break;
            case 2:
                collectionTime *= pow(10, -6);
                break;
            case 3:
                collectionTime *= pow(10, -3);
                break;
            default:
                qDebug() << "Fatal error scaling collection time";
                return;
                break;
            }
            lecollectiontime->setStyleSheet("QLineEdit { background: white }");
            psSettings->setCollectionTime(collectionTime, PSSignalSource::UI);
        }
    }
    else
    {
        lecollectiontime->setStyleSheet("QLineEdit { background: #FF8C8C }");
    }
}


void PicoScopeSettingsWidget::sampleIntervallChanged(int index)
{
    psSettings->setTimebase(cbsampleintervall->itemData(index).toInt(), PSSignalSource::UI);
}


void PicoScopeSettingsWidget::capturesChanged(int value)
{    
    psSettings->setCaptures(value, PSSignalSource::UI);
}


void PicoScopeSettingsWidget::presampleChanged(int value)
{
    psSettings->setPresamplePercentage(value, PSSignalSource::UI);
}


void PicoScopeSettingsWidget::onLinkSettingsStateChanged(int state)
{
    if(state == Qt::Checked)
    {
        psSettings->setLinkSettings(true);
        psSettings->setRange(PSChannel::A, psSettings->intToPSRange(cbRangeChannelA->currentIndex()), PSSignalSource::UI);
        psSettings->setRange(PSChannel::B, psSettings->intToPSRange(cbRangeChannelA->currentIndex()), PSSignalSource::UI);
        psSettings->setRange(PSChannel::C, psSettings->intToPSRange(cbRangeChannelA->currentIndex()), PSSignalSource::UI);
        psSettings->setRange(PSChannel::D, psSettings->intToPSRange(cbRangeChannelA->currentIndex()), PSSignalSource::UI);
        updateRange(psSettings->availableRange());
    }
    else if(state == Qt::Unchecked)
        psSettings->setLinkSettings(false);
}


void PicoScopeSettingsWidget::onOffsetEdited(QString text)
{
    /*float max;
    float min;

    PSChannel channel;
    if(QObject::sender() == nleOffsetChannelA)
        channel = PSChannel::A;
    else if(QObject::sender() == nleOffsetChannelB)
        channel = PSChannel::B;
    else if(QObject::sender() == nleOffsetChannelC)
        channel = PSChannel::C;
    else if(QObject::sender() == nleOffsetChannelD)
        channel = PSChannel::D;

    picoscope->offsetBounds(channel, &max, &min);
    float offset = text.toFloat();

    if(offset > max)
    {
        (dynamic_cast<NumberLineEdit*>(QObject::sender()))->setText(QString::number(max));
        psSettings->setOffset(channel, max);
    }
    else if(offset < min)
    {
        (dynamic_cast<NumberLineEdit*>(QObject::sender()))->setText(QString::number(min));
        psSettings->setOffset(channel, min);
    }
    else
        psSettings->setOffset(channel, offset);*/
}


void PicoScopeSettingsWidget::onOffsetValueChanged(double value)
{
    qDebug() << "onOffsetValueChanged()";

    if(!offsetBoundsInitialUpdated)
    {
        if(picoscope->open())
        {
            updateOffsetBounds(PSChannel::A);
            updateOffsetBounds(PSChannel::B);
            updateOffsetBounds(PSChannel::C);
            updateOffsetBounds(PSChannel::D);
            offsetBoundsInitialUpdated = true;
        }
        else
        {
            qDebug() << "Could not open picoscope to get offsets, setting offset anyways";
        }
    }

    if(QObject::sender() == spinboxOffsetChannelA)
        psSettings->setOffset(PSChannel::A, spinboxOffsetChannelA->value(), PSSignalSource::UI);
    else if(QObject::sender() == spinboxOffsetChannelB)
        psSettings->setOffset(PSChannel::B, spinboxOffsetChannelB->value(), PSSignalSource::UI);
    else if(QObject::sender() == spinboxOffsetChannelC)
        psSettings->setOffset(PSChannel::C, spinboxOffsetChannelC->value(), PSSignalSource::UI);
    else if(QObject::sender() == spinboxOffsetChannelD)
        psSettings->setOffset(PSChannel::D, spinboxOffsetChannelD->value(), PSSignalSource::UI);
}


// -------------------------------------
// SLOTS TO UPDATE GUI BASED ON SETTINGS
// -------------------------------------


/**
 * @brief PicoScopeSettingsWidget::updateGUI updates all window elements based
 * on current PicoScopeSettings values
 */
void PicoScopeSettingsWidget::updateGUI()
{
    //qDebug() << "reloadSettings() called";

    linkChannelSettings->blockSignals(true);
    linkChannelSettings->setChecked(psSettings->linkSettings());
    linkChannelSettings->blockSignals(false);

    channelA->blockSignals(true);
    channelB->blockSignals(true);
    channelC->blockSignals(true);
    channelD->blockSignals(true);

    if(psSettings->channel(PSChannel::A))
        channelA->setCheckState(Qt::Checked);
    else
        channelA->setCheckState(Qt::Unchecked);

    if(psSettings->channel(PSChannel::B))
        channelB->setCheckState(Qt::Checked);
    else
        channelB->setCheckState(Qt::Unchecked);

    if(psSettings->channel(PSChannel::C))
        channelC->setCheckState(Qt::Checked);
    else
        channelC->setCheckState(Qt::Unchecked);

    if(psSettings->channel(PSChannel::D))
        channelD->setCheckState(Qt::Checked);
    else
        channelD->setCheckState(Qt::Unchecked);

    channelA->blockSignals(false);
    channelB->blockSignals(false);
    channelC->blockSignals(false);
    channelD->blockSignals(false);

    //updateRangeCB(psSettings->availableRange(), (int)psSettings->range());

    updateRange(psSettings->availableRange());

    updateCouplingCB(psSettings->availableCoupling(), (int)psSettings->coupling());

    //updateOffset(psSettings->offset());

    updateOffsets();

    updateCaptures(psSettings->captures());

    updatePresample(psSettings->presamplePercentage());

    updateCollectionTime(psSettings->collectionTime());

    updateSampleIntervall(psSettings->timebase());
}


void PicoScopeSettingsWidget::updateRange(QList<QString> rangeList)
{
    for(int i = 1; i < 5; i++)
    {
        QComboBox *cb;
        int index;
        switch(i)
        {
        case 1: cb = cbRangeChannelA; index = (int)psSettings->range(PSChannel::A); break;
        case 2: cb = cbRangeChannelB; index = (int)psSettings->range(PSChannel::B); break;
        case 3: cb = cbRangeChannelC; index = (int)psSettings->range(PSChannel::C); break;
        case 4: cb = cbRangeChannelD; index = (int)psSettings->range(PSChannel::D); break;
        }
        cb->blockSignals(true);
        cb->clear();
        cb->addItems(rangeList);
        cb->setCurrentIndex(index);
        cb->blockSignals(false);
    }
}


/**
 * @brief PicoScopeSettingsWidget::updateCouplingCB Updates the content of the coupling combo box.
 * Blocks Combobox signals during update
 * @param couplingList QList<QString> containing the texts
 * @param index Item index which should be displayed
 */
void PicoScopeSettingsWidget::updateCouplingCB(QList<QString> couplingList, int index)
{
   cbcoupling->blockSignals(true);
   cbcoupling->clear();
   cbcoupling->addItems(couplingList);
   cbcoupling->setCurrentIndex(index);
   cbcoupling->blockSignals(false);
}


void PicoScopeSettingsWidget::updateOffsets()
{
    /*nleOffsetChannelA->blockSignals(true);
    nleOffsetChannelA->setValue(psSettings->offset(PSChannel::A));
    nleOffsetChannelA->blockSignals(false);

    nleOffsetChannelB->blockSignals(true);
    nleOffsetChannelB->setValue(psSettings->offset(PSChannel::B));
    nleOffsetChannelB->blockSignals(false);

    nleOffsetChannelC->blockSignals(true);
    nleOffsetChannelC->setValue(psSettings->offset(PSChannel::C));
    nleOffsetChannelC->blockSignals(false);

    nleOffsetChannelD->blockSignals(true);
    nleOffsetChannelD->setValue(psSettings->offset(PSChannel::D));
    nleOffsetChannelD->blockSignals(false);*/

    spinboxOffsetChannelA->blockSignals(true);
    spinboxOffsetChannelA->setValue(psSettings->offset(PSChannel::A));
    spinboxOffsetChannelA->blockSignals(false);
    spinboxOffsetChannelB->blockSignals(true);
    spinboxOffsetChannelB->setValue(psSettings->offset(PSChannel::B));
    spinboxOffsetChannelB->blockSignals(false);
    spinboxOffsetChannelC->blockSignals(true);
    spinboxOffsetChannelC->setValue(psSettings->offset(PSChannel::C));
    spinboxOffsetChannelC->blockSignals(false);
    spinboxOffsetChannelD->blockSignals(true);
    spinboxOffsetChannelD->setValue(psSettings->offset(PSChannel::D));
    spinboxOffsetChannelD->blockSignals(false);
}


void PicoScopeSettingsWidget::updateCollectionTime(double ct)
{
    lecollectiontime->blockSignals(true);
    cbcollectiontimemagnitude->blockSignals(true);

    if(ct >= 1.0)
    {
        lecollectiontime->setText(QString::number(ct * 1000));
        cbcollectiontimemagnitude->setCurrentIndex(0);
    }
    else if(ct / pow(10, -3) >= 1.0)
    {
        lecollectiontime->setText(QString::number(ct / pow(10, -3)));
        cbcollectiontimemagnitude->setCurrentIndex(0);
    }
    else if(ct / pow(10, -6) >= 1.0)
    {
        lecollectiontime->setText(QString::number(ct / pow(10, -6)));
        cbcollectiontimemagnitude->setCurrentIndex(1);
    }
    else
    {
        lecollectiontime->setText(QString::number(ct / pow(10, -9)));
        cbcollectiontimemagnitude->setCurrentIndex(2);
    }

    lecollectiontime->setStyleSheet("QLineEdit { background : white }");

    lecollectiontime->blockSignals(false);
    cbcollectiontimemagnitude->blockSignals(false);
}


void PicoScopeSettingsWidget::updateSampleIntervall(unsigned int si)
{
    int index = cbsampleintervall->findData(si);
    if(index >= 0)
    {
        cbsampleintervall->blockSignals(true);
        cbsampleintervall->setCurrentIndex(index);
        cbsampleintervall->blockSignals(false);
    }
    else
    {
        qDebug() << "Could not set index of sample intervall";
    }
}


void PicoScopeSettingsWidget::updateCaptures(unsigned int captures)
{
    sbcaptures->blockSignals(true);
    sbcaptures->setValue(captures);
    sbcaptures->blockSignals(false);
}


void PicoScopeSettingsWidget::updatePresample(unsigned int percentage)
{
    sbpresample->blockSignals(true);
    sbpresample->setValue(percentage);
    sbpresample->blockSignals(false);
}


void PicoScopeSettingsWidget::onCellClicked(int r,int c)
{
    if(r == 4 && c == 5)
    {
        pasteClipBoard();
    }
    else if(r == 5 && c == 5)
    {
        changeGainIncrement();
    }
}


/**
 * @brief PicoScopeSettingsWidget::pasteClipBoard inserts gain voltage from system clipboard
 */
void PicoScopeSettingsWidget::pasteClipBoard()
{ 
    // get string from operating system clipboard
    // for example "1;2;3;4"
    // insert data in input fields
    QClipboard *clipboard = QApplication::clipboard();
    QString originalText = clipboard->text();

    QStringList vlist = originalText.split( ";" );

    if(vlist.size() == 4)
    {
        spinboxGainChannelA->setValue(vlist.at(0).toDouble());
        spinboxGainChannelB->setValue(vlist.at(1).toDouble());
        spinboxGainChannelC->setValue(vlist.at(2).toDouble());
        spinboxGainChannelD->setValue(vlist.at(3).toDouble());
    }
}


/**
 * @brief PicoScopeSettingsWidget::changeGainIncrement changed increment variable for cursor gain control
 */
void PicoScopeSettingsWidget::changeGainIncrement()
{
    QString currentValue;

    QList<QString> incList;

    incList << "0.001" << "0.005" << "0.01" << "0.02" << "0.05" << "0.1";

    currentValue = Core::instance()->ioSettings->value("deviceManager", "StepSizeAnalogOut").toString();

    int curIndex = incList.indexOf(currentValue);

    if(curIndex == 5)
        curIndex = 0;
    else
        curIndex++;

    currentValue = incList.at(curIndex);

    Core::instance()->ioSettings->setValue("deviceManager", "StepSizeAnalogOut", currentValue);

    // update table widget cell
    //setItem(5,5,new QTableWidgetItem("Step: " + currentValue));
    //item(5,5)->setIcon(QIcon(Core::rootDir + "resources/icons/arrow_refresh.png"));
    //item(5,5)->setFlags(Qt::ItemIsEnabled);
    stepSizeSwitch->setText(QString("Step: %0").arg(currentValue));

    // update io settings
    onIOSettingsChanged();

    emit stepSizeChanged(currentValue.toDouble());
}


void PicoScopeSettingsWidget::updateOffsetBounds(PSChannel channel)
{
    qDebug() << "updateOffsetBounds";

    float max, min;
    QString strChannel("");

    picoscope->offsetBounds(channel, &max, &min);

    if(psSettings->linkSettings())
    {
        spinboxOffsetChannelA->setRange(min, max);
        spinboxOffsetChannelB->setRange(min, max);
        spinboxOffsetChannelC->setRange(min, max);
        spinboxOffsetChannelD->setRange(min, max);
    }
    else
    {
        switch(channel)
        {
        case PSChannel::A: spinboxOffsetChannelA->setRange(min, max); strChannel = "A"; break;
        case PSChannel::B: spinboxOffsetChannelB->setRange(min, max); strChannel = "B"; break;
        case PSChannel::C: spinboxOffsetChannelC->setRange(min, max); strChannel = "C"; break;
        case PSChannel::D: spinboxOffsetChannelD->setRange(min, max); strChannel = "D"; break;
        }
    }
}


void PicoScopeSettingsWidget::onSettingsRangeChanged(PSChannel channel)
{
    if(psSettings->linkSettings())
    {
        if(channel == PSChannel::A)
            updateOffsetBounds(channel);
    }
    else
        updateOffsetBounds(channel);

    /*float max;
    float min;

    NumberLineEdit *nle;

    switch(channel)
    {
    case PSChannel::A: nle = nleOffsetChannelA; break;
    case PSChannel::B: nle = nleOffsetChannelB; break;
    case PSChannel::C: nle = nleOffsetChannelC; break;
    case PSChannel::D: nle = nleOffsetChannelD; break;
    }

    picoscope->offsetBounds(channel, &max, &min);

    if(nle->text().toFloat() > max)
    {
        nle->setText(QString::number(max));
        psSettings->setOffset(channel, max);
    }
    else if(nle->text().toFloat() < min)
    {
        nle->setText(QString::number(min));
        psSettings->setOffset(channel, min);
    }*/
}

void PicoScopeSettingsWidget::updateGainList(QList<double> list)
{
    spinboxGainChannelA->setValue(list.at(0));
    spinboxGainChannelB->setValue(list.at(1));
    spinboxGainChannelC->setValue(list.at(2));
    spinboxGainChannelD->setValue(list.at(3));
}


#ifdef LIISIM_NIDAQMX
void PicoScopeSettingsWidget::onDeviceListUpdated()
{
    comboboxGainOUTChannelA->blockSignals(true);
    comboboxGainOUTChannelB->blockSignals(true);
    comboboxGainOUTChannelC->blockSignals(true);
    comboboxGainOUTChannelD->blockSignals(true);

    /*QString channelALastText = comboboxGainOUTChannelA->currentText();
    int channelALastIndex = comboboxGainOUTChannelA->currentIndex();
    QString channelBLastText = comboboxGainOUTChannelB->currentText();
    int channelBLastIndex = comboboxGainOUTChannelB->currentIndex();
    QString channelCLastText = comboboxGainOUTChannelC->currentText();
    int channelCLastIndex = comboboxGainOUTChannelC->currentIndex();
    QString channelDLastText = comboboxGainOUTChannelD->currentText();
    int channelDLastIndex = comboboxGainOUTChannelD->currentIndex();*/

    QList<DAQDevice> devList = Core::instance()->devManager->devices();

    comboboxGainOUTChannelA->clear();
    comboboxGainOUTChannelB->clear();
    comboboxGainOUTChannelC->clear();
    comboboxGainOUTChannelD->clear();
    comboboxGainOUTChannelA->addItem("Manual");
    comboboxGainOUTChannelB->addItem("Manual");
    comboboxGainOUTChannelC->addItem("Manual");
    comboboxGainOUTChannelD->addItem("Manual");

    for(int i = 0; i < devList.size(); i++)
    {
        for(int j = 0; j < devList.at(i).analogOut.size(); j++)
        {
            comboboxGainOUTChannelA->addItem(devList.at(i).analogOut.value(j));
            comboboxGainOUTChannelB->addItem(devList.at(i).analogOut.value(j));
            comboboxGainOUTChannelC->addItem(devList.at(i).analogOut.value(j));
            comboboxGainOUTChannelD->addItem(devList.at(i).analogOut.value(j));
        }
    }

    /*if(comboboxGainOUTChannelA->findText(channelALastText) == channelALastIndex)
        comboboxGainOUTChannelA->setCurrentIndex(channelALastIndex);
    else
    {
        comboboxGainOUTChannelA->blockSignals(false);
        comboboxGainOUTChannelA->currentIndexChanged(comboboxGainOUTChannelA->currentIndex());
    }
    if(comboboxGainOUTChannelB->findText(channelBLastText) == channelBLastIndex)
        comboboxGainOUTChannelB->setCurrentIndex(channelBLastIndex);
    else
    {
        comboboxGainOUTChannelB->blockSignals(false);
        comboboxGainOUTChannelB->currentIndexChanged(comboboxGainOUTChannelB->currentIndex());
    }
    if(comboboxGainOUTChannelC->findText(channelCLastText) == channelCLastIndex)
        comboboxGainOUTChannelC->setCurrentIndex(channelCLastIndex);
    else
    {
        comboboxGainOUTChannelC->blockSignals(false);
        comboboxGainOUTChannelC->currentIndexChanged(comboboxGainOUTChannelC->currentIndex());
    }
    if(comboboxGainOUTChannelD->findText(channelDLastText) == channelDLastIndex)
        comboboxGainOUTChannelD->setCurrentIndex(channelDLastIndex);
    else
    {
        comboboxGainOUTChannelD->blockSignals(false);
        comboboxGainOUTChannelD->currentIndexChanged(comboboxGainOUTChannelD->currentIndex());
    }*/

    int index;
    index = comboboxGainOUTChannelA->findText(Core::instance()->devManager->getAnalogOutChannelIdentifier(1));
    if(index != -1)
        comboboxGainOUTChannelA->setCurrentIndex(index);

    index = comboboxGainOUTChannelB->findText(Core::instance()->devManager->getAnalogOutChannelIdentifier(2));
    if(index != -1)
        comboboxGainOUTChannelB->setCurrentIndex(index);

    index = comboboxGainOUTChannelC->findText(Core::instance()->devManager->getAnalogOutChannelIdentifier(3));
    if(index != -1)
        comboboxGainOUTChannelC->setCurrentIndex(index);

    index = comboboxGainOUTChannelD->findText(Core::instance()->devManager->getAnalogOutChannelIdentifier(4));
    if(index != -1)
        comboboxGainOUTChannelD->setCurrentIndex(index);

    comboboxGainOUTChannelA->blockSignals(false);
    comboboxGainOUTChannelB->blockSignals(false);
    comboboxGainOUTChannelC->blockSignals(false);
    comboboxGainOUTChannelD->blockSignals(false);
}


void PicoScopeSettingsWidget::onSpinboxGainValueChanged()
{
    if(QObject::sender() == spinboxGainChannelA)
        if(comboboxGainOUTChannelA->currentText() != "Manual")
            Core::instance()->devManager->setOutputVoltage(1, spinboxGainChannelA->value());

    if(QObject::sender() == spinboxGainChannelB)
        if(comboboxGainOUTChannelB->currentText() != "Manual")
            Core::instance()->devManager->setOutputVoltage(2, spinboxGainChannelB->value());

    if(QObject::sender() == spinboxGainChannelC)
        if(comboboxGainOUTChannelC->currentText() != "Manual")
            Core::instance()->devManager->setOutputVoltage(3, spinboxGainChannelC->value());

    if(QObject::sender() == spinboxGainChannelD)
        if(comboboxGainOUTChannelD->currentText() != "Manual")
            Core::instance()->devManager->setOutputVoltage(4, spinboxGainChannelD->value());
}


void PicoScopeSettingsWidget::onAnalogOutLimitChanged()
{
    spinboxGainChannelA->setRange(Core::instance()->devManager->getAnalogOutLimitMin(), Core::instance()->devManager->getAnalogOutLimitMax());
    spinboxGainChannelB->setRange(Core::instance()->devManager->getAnalogOutLimitMin(), Core::instance()->devManager->getAnalogOutLimitMax());
    spinboxGainChannelC->setRange(Core::instance()->devManager->getAnalogOutLimitMin(), Core::instance()->devManager->getAnalogOutLimitMax());
    spinboxGainChannelD->setRange(Core::instance()->devManager->getAnalogOutLimitMin(), Core::instance()->devManager->getAnalogOutLimitMax());
}


void PicoScopeSettingsWidget::onComboboxGainChannelChanged()
{
    if(QObject::sender() == comboboxGainOUTChannelA)
    {
        if(comboboxGainOUTChannelA->currentText() == "Manual")
            //Core::instance()->devManager->setGainOutputEnabled(1, false);
            Core::instance()->devManager->setOutputChannel(1, QString(""), 0);
        else
        {
            Core::instance()->devManager->setOutputChannel(1, comboboxGainOUTChannelA->currentText(), spinboxGainChannelA->value());
            //Core::instance()->devManager->setOutputChannel(1, comboboxGainOUTChannelA->currentText());
            //Core::instance()->devManager->setGainOutputEnabled(1, true);
        }
    }
    if(QObject::sender() == comboboxGainOUTChannelB)
    {
        if(comboboxGainOUTChannelB->currentText() == "Manual")
            //Core::instance()->devManager->setGainOutputEnabled(2, false);
            Core::instance()->devManager->setOutputChannel(2, QString(""), 0);
        else
        {
            Core::instance()->devManager->setOutputChannel(2, comboboxGainOUTChannelB->currentText(), spinboxGainChannelB->value());
            //Core::instance()->devManager->setOutputChannel(2, comboboxGainOUTChannelB->currentText());
            //Core::instance()->devManager->setGainOutputEnabled(2, true);
        }
    }
    if(QObject::sender() == comboboxGainOUTChannelC)
    {
        if(comboboxGainOUTChannelC->currentText() == "Manual")
            //Core::instance()->devManager->setGainOutputEnabled(3, false);
            Core::instance()->devManager->setOutputChannel(3, QString(""), 0);
        else
        {
            Core::instance()->devManager->setOutputChannel(3, comboboxGainOUTChannelC->currentText(), spinboxGainChannelC->value());
            //Core::instance()->devManager->setOutputChannel(3, comboboxGainOUTChannelC->currentText());
            //Core::instance()->devManager->setGainOutputEnabled(3, true);
        }
    }
    if(QObject::sender() == comboboxGainOUTChannelD)
    {
        if(comboboxGainOUTChannelD->currentText() == "Manual")
            //Core::instance()->devManager->setGainOutputEnabled(4, false);
            Core::instance()->devManager->setOutputChannel(4, QString(""), 0);
        else
        {
            Core::instance()->devManager->setOutputChannel(4, comboboxGainOUTChannelD->currentText(), spinboxGainChannelD->value());
            //Core::instance()->devManager->setOutputChannel(4, comboboxGainOUTChannelD->currentText());
            //Core::instance()->devManager->setGainOutputEnabled(4, true);
        }
    }
}
#endif



void PicoScopeSettingsWidget::onBlockSequenceBlockNumber(int idx)
{
    Core::instance()->ioSettings->setValue("blockSequence", "blocks", comboboxBlockSequenceBlockNumber->currentText().toInt());
}

void PicoScopeSettingsWidget::onBlockSequenceDecrementChanged(double value)
{
    Core::instance()->ioSettings->setValue("blockSequence", "decrement", value);
}


void PicoScopeSettingsWidget::onBlockSequenceDelayChanged(double value)
{
    Core::instance()->ioSettings->setValue("blockSequence", "delay", value);
}


void PicoScopeSettingsWidget::onIOSettingsChanged()
{
    if(Core::instance()->ioSettings->hasEntry("deviceManager", "StepSizeAnalogOut"))
    {
        double stepSize = Core::instance()->ioSettings->value("deviceManager", "StepSizeAnalogOut").toDouble();
        spinboxGainChannelA->setSingleStep(stepSize);
        spinboxGainChannelB->setSingleStep(stepSize);
        spinboxGainChannelC->setSingleStep(stepSize);
        spinboxGainChannelD->setSingleStep(stepSize);

        stepSizeSwitch->setText(QString("Step: %0").arg(this->locale().toString(stepSize)));

        emit stepSizeChanged(stepSize);
    }

    if(Core::instance()->ioSettings->hasEntry("blockSequence", "blocks"))
    {
        int idx = Core::instance()->ioSettings->value("blockSequence", "blocks").toInt()-1;

        if(idx >= comboboxBlockSequenceBlockNumber->count())
            idx = 0;

        if(idx < 0)
            idx = 0;

        comboboxBlockSequenceBlockNumber->setCurrentIndex(idx);
    }

    if(Core::instance()->ioSettings->hasEntry("blockSequence", "decrement"))
        spinboxBlockSequenceDecrement->setValue(Core::instance()->ioSettings->value("blockSequence", "decrement").toDouble());

    if(Core::instance()->ioSettings->hasEntry("blockSequence", "delay"))
        spinboxBlockSequenceDelay->setValue(Core::instance()->ioSettings->value("blockSequence", "delay").toDouble());
}


void PicoScopeSettingsWidget::updateGainReferenceVoltage(float gainA, float gainB, float gainC, float gainD)
{
    labelGainINChannelA->setText(this->locale().toString(gainA));
    labelGainINChannelB->setText(this->locale().toString(gainB));
    labelGainINChannelC->setText(this->locale().toString(gainC));
    labelGainINChannelD->setText(this->locale().toString(gainD));
}


void PicoScopeSettingsWidget::updateGainReferenceVoltage(int channel, float value)
{
    //qDebug() << "updating gain reference" << channel << value;
    switch(channel)
    {
    case 1: labelGainINChannelA->setText(this->locale().toString(value)); break;
    case 2: labelGainINChannelB->setText(this->locale().toString(value)); break;
    case 3: labelGainINChannelC->setText(this->locale().toString(value)); break;
    case 4: labelGainINChannelD->setText(this->locale().toString(value)); break;
    }
}


void PicoScopeSettingsWidget::onAnalogInputReset()
{
    labelGainINChannelA->setText("-");
    labelGainINChannelB->setText("-");
    labelGainINChannelC->setText("-");
    labelGainINChannelD->setText("-");
}


bool PicoScopeSettingsWidget::focusNextPrevChild(bool next)
{
    if(tabKeyNavigation())
    {
        // Qt::Key_Down instead of Qt::Key_Tab and Qt::Key_Up instead of Qt::Key_Backtab
        QKeyEvent event(QEvent::KeyPress, next ? Qt::Key_Down : Qt::Key_Up, Qt::NoModifier);
        keyPressEvent(&event);
        if(event.isAccepted())
            return true;
    }
    return QTableWidget::focusNextPrevChild(next);
}


void PicoScopeSettingsWidget::onChannelVisibilityChanged()
{
    if(QObject::sender() == wvcChannelA->buttonVisible)
    {
        if(Core::instance()->guiSettings->value("picoscopewidget", "channelAvisible").toBool())
        {
            Core::instance()->guiSettings->setValue("picoscopewidget", "channelAvisible", false);
            wvcChannelA->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1gray_32.png"));
        }
        else
        {
            Core::instance()->guiSettings->setValue("picoscopewidget", "channelAvisible", true);
            wvcChannelA->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1_32.png"));
        }
    }

    if(QObject::sender() == wvcChannelB->buttonVisible)
    {
        if(Core::instance()->guiSettings->value("picoscopewidget", "channelBvisible").toBool())
        {
            Core::instance()->guiSettings->setValue("picoscopewidget", "channelBvisible", false);
            wvcChannelB->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1gray_32.png"));
        }
        else
        {
            Core::instance()->guiSettings->setValue("picoscopewidget", "channelBvisible", true);
            wvcChannelB->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1_32.png"));
        }
    }

    if(QObject::sender() == wvcChannelC->buttonVisible)
    {
        if(Core::instance()->guiSettings->value("picoscopewidget", "channelCvisible").toBool())
        {
            Core::instance()->guiSettings->setValue("picoscopewidget", "channelCvisible", false);
            wvcChannelC->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1gray_32.png"));
        }
        else
        {
            Core::instance()->guiSettings->setValue("picoscopewidget", "channelCvisible", true);
            wvcChannelC->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1_32.png"));
        }
    }

    if(QObject::sender() == wvcChannelD->buttonVisible)
    {
        if(Core::instance()->guiSettings->value("picoscopewidget", "channelDvisible").toBool())
        {
            Core::instance()->guiSettings->setValue("picoscopewidget", "channelDvisible", false);
            wvcChannelD->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1gray_32.png"));
        }
        else
        {
            Core::instance()->guiSettings->setValue("picoscopewidget", "channelDvisible", true);
            wvcChannelD->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1_32.png"));
        }
    }
}


void PicoScopeSettingsWidget::onChannelLayerUp()
{
    unsigned int curve = 0;
    if(QObject::sender() == wvcChannelA->buttonLayerUp)
        curve = 1;
    else if(QObject::sender() == wvcChannelB->buttonLayerUp)
        curve = 2;
    else if(QObject::sender() == wvcChannelC->buttonLayerUp)
        curve = 3;
    else if(QObject::sender() == wvcChannelD->buttonLayerUp)
        curve = 4;

    emit switchStreamSignalLayer(curve, true);
}


void PicoScopeSettingsWidget::onChannelLayerDown()
{
    unsigned int curve = 0;
    if(QObject::sender() == wvcChannelA->buttonLayerDown)
        curve = 1;
    else if(QObject::sender() == wvcChannelB->buttonLayerDown)
        curve = 2;
    else if(QObject::sender() == wvcChannelC->buttonLayerDown)
        curve = 3;
    else if(QObject::sender() == wvcChannelD->buttonLayerDown)
        curve = 4;

    emit switchStreamSignalLayer(curve, false);
}


void PicoScopeSettingsWidget::onGUISettingsChanged()
{
    if(Core::instance()->guiSettings->hasEntry("picoscopewidget", "channelAvisible"))
    {
        if(Core::instance()->guiSettings->value("picoscopewidget", "channelAvisible").toBool())
            wvcChannelA->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1_32.png"));
        else
            wvcChannelA->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1gray_32.png"));
    }
    else
        Core::instance()->guiSettings->setValue("picoscopewidget", "channelAvisible", true);

    if(Core::instance()->guiSettings->hasEntry("picoscopewidget", "channelBvisible"))
    {
        if(Core::instance()->guiSettings->value("picoscopewidget", "channelBvisible").toBool())
            wvcChannelB->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1_32.png"));
        else
            wvcChannelB->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1gray_32.png"));
    }
    else
        Core::instance()->guiSettings->setValue("picoscopewidget", "channelBvisible", true);

    if(Core::instance()->guiSettings->hasEntry("picoscopewidget", "channelCvisible"))
    {
        if(Core::instance()->guiSettings->value("picoscopewidget", "channelCvisible").toBool())
            wvcChannelC->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1_32.png"));
        else
            wvcChannelC->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1gray_32.png"));
    }
    else
        Core::instance()->guiSettings->setValue("picoscopewidget", "channelCvisible", true);

    if(Core::instance()->guiSettings->hasEntry("picoscopewidget", "channelDvisible"))
    {
        if(Core::instance()->guiSettings->value("picoscopewidget", "channelDvisible").toBool())
            wvcChannelD->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1_32.png"));
        else
            wvcChannelD->buttonVisible->setIcon(QIcon(Core::rootDir + "resources/icons/visible1gray_32.png"));
    }
    else
        Core::instance()->guiSettings->setValue("picoscopewidget", "channelDvisible", true);

    if(Core::instance()->guiSettings->hasEntry("picoscopewidget", "averagecaptures"))
        checkboxAverageCaptures->setChecked(Core::instance()->guiSettings->value("picoscopewidget", "averagecaptures").toBool());
}


bool PicoScopeSettingsWidget::averageCaptures()
{
    return checkboxAverageCaptures->isChecked();
}


void PicoScopeSettingsWidget::onAverageCapturesStateChanged()
{
    Core::instance()->guiSettings->setValue("picoscopewidget", "averagecaptures", checkboxAverageCaptures->isChecked());
}


int PicoScopeSettingsWidget::getNoCaptures()
{
    return sbcaptures->value();
}
