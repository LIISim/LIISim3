#include "da_laserenergysettingswidget.h"

#include "qtoolbutton.h"
#include <QGroupBox>
#include "../../io/devicemanager.h"

#include <qwt_symbol.h>

DA_LaserEnergySettingsWidget::DA_LaserEnergySettingsWidget(QWidget *parent) : QWidget(parent)
{
    lesw = new LaserEnergySettingsWidget(this);
    //leiw = new LaserEnergyInfoWidget(this);

    /*mainLayout = new QVBoxLayout();
    lowerLayout = new QHBoxLayout();

    tableWidget = new QTableWidget(this);
    tableWidget->setColumnCount(3);
    tableWidget->setRowCount(5);
    QStringList header;
    header << "SET" << "IST" << "ENERGY";
    tableWidget->setHorizontalHeaderLabels(header);

    comboboxSet = new QComboBox(this);
    labelActual = new QLabel(this);
    lowerLayout->addWidget(comboboxSet);
    lowerLayout->addWidget(labelActual);

    mainLayout->addWidget(tableWidget);
    mainLayout->addLayout(lowerLayout);

    setLayout(mainLayout);

    connect(tableWidget, SIGNAL(cellChanged(int,int)), SLOT(onCellChanged(int,int)));*/
}


LaserEnergySettingsWidget* DA_LaserEnergySettingsWidget::getSettingsWidget()
{
    return lesw;
}

LaserEnergyInfoWidget* DA_LaserEnergySettingsWidget::getInfoWidget()
{
    return new LaserEnergyInfoWidget;
}


void DA_LaserEnergySettingsWidget::onCellChanged(int row, int column)
{
    qDebug() << row << column;
}


//--- LaserEnergySettingsWidget

LaserEnergySettingsWidget::LaserEnergySettingsWidget(QWidget *parent) : QWidget(parent)
{
    this->setLayout(new QVBoxLayout);

    groupboxLayout = new QGridLayout;

    comboboxLUT = new QComboBox(this);
    comboboxInput = new QComboBox(this);
    comboboxOutput = new QComboBox(this);

    basePlot = new BasePlotWidgetQwt(this);
    basePlot->setPlotAxisTitles("Laser Energy (mJ/mm²)", "Position Voltage (V)");

    QLabel *labelLUT = new QLabel("Laser Energy LUT", this);
    QLabel *labelSource = new QLabel("Position Input", this);
    QLabel *labelPosition = new QLabel("Position Output", this);

    QGroupBox *groupbox = new QGroupBox("Laser Energy", this);
    //groupbox->setMaximumHeight(300);
    groupbox->setMinimumHeight(350);

    groupboxLayout->addWidget(labelLUT, 0, 0);
    groupboxLayout->addWidget(comboboxLUT, 0, 1);
    groupboxLayout->addWidget(labelSource, 1, 0);
    groupboxLayout->addWidget(comboboxInput, 1, 1);
    groupboxLayout->addWidget(labelPosition, 2, 0);
    groupboxLayout->addWidget(comboboxOutput, 2, 1);
    groupboxLayout->addWidget(basePlot, 3, 0, 2, 2);

    groupbox->setLayout(groupboxLayout);

    this->layout()->addWidget(groupbox);
    this->layout()->setMargin(0);

    comboboxInput->addItem("None");
    comboboxOutput->addItem("None");

    onLaserEnergyLUTUpdate();

    connect(comboboxLUT, SIGNAL(currentIndexChanged(int)), SLOT(onComboboxLUTCurrentIndexChanged()));
}


void LaserEnergySettingsWidget::onDeviceListUpdate()
{
    QList<DAQDevice> devList = Core::instance()->devManager->devices();

    comboboxInput->blockSignals(true);
    comboboxOutput->blockSignals(true);

    comboboxInput->clear();
    comboboxOutput->clear();

    comboboxInput->addItem("None");
    comboboxOutput->addItem("None");

    for(int i = 0; i < devList.size(); i++)
    {
        for(int j = 0; j < devList.at(i).analogIn.size(); j++)
        {
            comboboxInput->addItem(devList.at(i).analogIn.value(j));
        }

        for(int j = 0; j < devList.at(i).analogOut.size(); j++)
        {
            comboboxOutput->addItem(devList.at(i).analogOut.value(j));
        }
    }

    int indexInput = comboboxInput->findText(Core::instance()->laserEnergyPosition->getCurrentInputChannelIdentifier());
    int indexOutput = comboboxOutput->findText(Core::instance()->laserEnergyPosition->getCurrentOutputChannelIdentifier());

    if(indexInput != -1)
        comboboxInput->setCurrentIndex(indexInput);
    if(indexOutput != -1)
        comboboxOutput->setCurrentIndex(indexOutput);

    comboboxInput->blockSignals(false);
    comboboxOutput->blockSignals(false);
}


void LaserEnergySettingsWidget::onLaserEnergyLUTUpdate()
{
    QList<DatabaseContent*> *lel = Core::instance()->getDatabaseManager()->getLaserEnergy();

    comboboxLUT->blockSignals(true);
    comboboxLUT->clear();

    int index = -1;

    for(int i = 0; i < lel->size(); i++)
    {
        comboboxLUT->addItem(lel->at(i)->name, lel->at(i)->filename);
        if(Core::instance()->laserEnergyPosition->getCurrentLUTName() == lel->at(i)->name)
            index = i;
    }

    if(index != -1)
        comboboxLUT->setCurrentIndex(index);

    comboboxLUT->blockSignals(false);

    onComboboxLUTCurrentIndexChanged();
}


void LaserEnergySettingsWidget::onOK()
{
    Core::instance()->laserEnergyPosition->setLaserEnergyLUT(comboboxLUT->currentData().toString());

    if(comboboxInput->currentText() != "None")
        Core::instance()->laserEnergyPosition->setAnalogInputChannel(comboboxInput->currentText());
    else
        Core::instance()->laserEnergyPosition->setAnalogInputChannel("");

    if(comboboxOutput->currentText() != "None")
        Core::instance()->laserEnergyPosition->setAnalogOutputChannel(comboboxOutput->currentText());
    else
        Core::instance()->laserEnergyPosition->setAnalogOutputChannel("");
}


void LaserEnergySettingsWidget::onComboboxLUTCurrentIndexChanged()
{
    int lutIndex = Core::instance()->getDatabaseManager()->indexOfLaserEnergy(comboboxLUT->currentData().toString());
    if(lutIndex != -1)
    {
        LaserEnergy *lut = Core::instance()->getDatabaseManager()->getLaserEnergy(lutIndex);

        QVector<double> setData, posData, energyData;

        /*for(int i = 0; i < lut->lookupTable.size(); i++)
        {
            setData.push_back(lut->lookupTable.at(i).set);
            posData.push_back(lut->lookupTable.at(i).pos);
            energyData.push_back(lut->lookupTable.at(i).energy);
        }*/

        for(QMap<double, QPair<double, double> >::iterator it = lut->table.begin(); it != lut->table.end(); ++it)
        {
            setData.push_back(it.value().first);
            posData.push_back(it.value().second);
            energyData.push_back(it.key());
        }

        basePlot->detachAllCurves();

        QColor curveColor(Qt::red);

        setCurve = new BasePlotCurve(QString("SET"));

        QwtSymbol* s1 = new QwtSymbol(QwtSymbol::Cross);
        s1->setColor(curveColor);
        s1->setSize(12);
        setCurve->setSymbol(s1);
        setCurve->setPen(curveColor);
        setCurve->setSamples(energyData, setData);
        setCurve->setRenderHint(QwtPlotItem::RenderAntialiased);

        //setCurve->attach(basePlot->qwtPlot);
        basePlot->registerCurve(setCurve);

        posCurve = new BasePlotCurve(QString("POS"));

        curveColor = QColor(Qt::blue);

        QwtSymbol* s2 = new QwtSymbol(QwtSymbol::Cross);
        s2->setColor(curveColor);
        s2->setSize(12);
        posCurve->setSymbol(s2);
        posCurve->setPen(curveColor);
        posCurve->setSamples(energyData, posData);
        posCurve->setRenderHint(QwtPlotItem::RenderAntialiased);

        //posCurve->attach(basePlot->qwtPlot);
        basePlot->registerCurve(posCurve);

        basePlot->setMaxLegendColumns(1);
    }
}


//--- LaserEnergyInfoWidget

LaserEnergyInfoWidget::LaserEnergyInfoWidget(QWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout;

    labelFluence = new QLabel("5.3", this);
    QLabel *labelPositionHeader = new QLabel("Position Value:", this);
    labelPosition = new QLabel("1.2", this);
    comboboxSetpoint = new QComboBox(this);

    layout->addWidget(labelFluence);
    layout->addWidget(labelPositionHeader);
    layout->addWidget(labelPosition);
    layout->addWidget(comboboxSetpoint);

    this->setLayout(layout);
}


// --- LaserEnergyControllerWidget ---

LaserEnergyControllerWidget::LaserEnergyControllerWidget(QWidget *parent) : QWidget(parent)
{
    setLayout(new QHBoxLayout(this));

    spinboxLaserFluence = new QDoubleSpinBox(this);
    spinboxLaserFluence->setDecimals(3);
    QLabel *labelEnergyUnit = new QLabel("mJ/mm²");
    labelPositionVoltage = new QLabel("(0 V)");
    buttonSet = new QToolButton();
    buttonSet->setText("Set");
    buttonEnableDisableIO = new QToolButton();
    buttonEnableDisableIO->setText("Enable IO");

    layout()->addWidget(spinboxLaserFluence);
    layout()->addWidget(labelEnergyUnit);
    layout()->addWidget(labelPositionVoltage);
    layout()->addWidget(buttonSet);
    layout()->addWidget(buttonEnableDisableIO);
    layout()->setMargin(0);

    ioEnabled = false;
    setpointValid = false;
    lastSetpoint = 0.0f;

    connect(buttonSet, SIGNAL(released()), SLOT(onButtonSetReleased()));
    connect(spinboxLaserFluence, SIGNAL(editingFinished()), SLOT(onButtonSetReleased()));
    connect(spinboxLaserFluence, SIGNAL(valueChanged(double)), SLOT(onSpinBoxLaserFluenceValueChanged()));
    connect(buttonEnableDisableIO, SIGNAL(released()), SLOT(onButtonEnableDisableIOReleased()));

    connect(Core::instance()->laserEnergyPosition, SIGNAL(calculatedPositionValueChanged(double, bool)), SLOT(onCalculatedPositionValueChanged(double, bool)));
    connect(Core::instance()->laserEnergyPosition, SIGNAL(ioEnabled(bool)), SLOT(onIOEnabled(bool)));
    connect(Core::instance()->guiSettings, SIGNAL(settingsChanged()), SLOT(onGUISettingsChanged()));
}


double LaserEnergyControllerWidget::getLaserFluence()
{
    return spinboxLaserFluence->value();
}

bool LaserEnergyControllerWidget::isIOEnabled()
{
    return ioEnabled;
}


void LaserEnergyControllerWidget::onButtonSetReleased()
{
    spinboxLaserFluence->setStyleSheet("QDoubleSpinBox { background : white; } ");
    Core::instance()->guiSettings->setValue("laserenergycontroller", "lastLaserenergy", spinboxLaserFluence->value());
    Core::instance()->laserEnergyPosition->setLaserFluence(spinboxLaserFluence->value());
}


void LaserEnergyControllerWidget::onButtonEnableDisableIOReleased()
{
    Core::instance()->laserEnergyPosition->enableIO(!ioEnabled);
}


void LaserEnergyControllerWidget::onCalculatedPositionValueChanged(double voltage, bool valid)
{
    if(valid)
        labelPositionVoltage->setText(QString("(%0 V)").arg(this->locale().toString(voltage, 'f', 3)));
    else
    {
        spinboxLaserFluence->setStyleSheet("QDoubleSpinBox { background : tomato; } ");
        labelPositionVoltage->setText(QString("(not valid)"));
    }

    setpointValid = valid;
    lastSetpoint = voltage;
}


void LaserEnergyControllerWidget::onIOEnabled(bool enabled)
{
    ioEnabled = enabled;
    if(enabled)
    {
        buttonEnableDisableIO->setText("Disable IO");
        buttonEnableDisableIO->setStyleSheet("QToolButton { color : forestgreen; }");
    }
    else
    {
        buttonEnableDisableIO->setText("Enable IO");
        buttonEnableDisableIO->setStyleSheet("QToolButton { color : black; }");
    }
}


void LaserEnergyControllerWidget::onSpinBoxLaserFluenceValueChanged()
{
    spinboxLaserFluence->setStyleSheet("QDoubleSpinBox { background : yellow; } ");
}


void LaserEnergyControllerWidget::onGUISettingsChanged()
{
    spinboxLaserFluence->setValue(Core::instance()->guiSettings->value("laserenergycontroller", "lastLaserenergy", 0).toDouble());
}


void LaserEnergyControllerWidget::setSpinBoxLaserFluenceSingleStep(double value)
{
    spinboxLaserFluence->setSingleStep(value);
}


// --- LaserEnergyMeasurementWidget ---

LaserEnergyMeasurementWidget::LaserEnergyMeasurementWidget(QWidget *parent) : QWidget(parent)
{
    setLayout(new QHBoxLayout(this));

    positionValid = false;
    lastPosition = 0.0f;

    labelMeasuredLaserFluence = new QLabel("-", this);
    //QLabel *labelEnergyUnit = new QLabel("mJ/mm²", this);

    layout()->addWidget(labelMeasuredLaserFluence);
    //layout()->addWidget(labelEnergyUnit);
    layout()->setMargin(0);

    connect(Core::instance()->laserEnergyPosition, SIGNAL(readedPositionVoltage(double,bool)), SLOT(onReadedPositionVoltage(double,bool)));
    connect(Core::instance()->laserEnergyPosition, SIGNAL(readedLaserFluence(double,bool)), SLOT(onReadedLaserFluence(double,bool)));
    connect(Core::instance()->laserEnergyPosition, SIGNAL(readedLaserFluenceAndPositionVoltage(double,double,bool)),
            SLOT(onReadedLaserFluenceAndPositionVoltage(double,double,bool)));
}


void LaserEnergyMeasurementWidget::onReadedPositionVoltage(double voltage, bool valid)
{
    if(valid)
        labelMeasuredLaserFluence->setToolTip(QString("Readed voltage: %0 V").arg(this->locale().toString(voltage, 'f', 3)));
    else
        labelMeasuredLaserFluence->setToolTip(QString("Readed voltage: %0 V, not in range").arg(this->locale().toString(voltage, 'f', 3)));

    positionValid = valid;
    lastPosition = voltage;
}


void LaserEnergyMeasurementWidget::onReadedLaserFluence(double fluence, bool valid)
{
    if(valid)
        labelMeasuredLaserFluence->setText(QString("%0 mJ/mm²").arg(fluence));
    else
        labelMeasuredLaserFluence->setText("Pos out of range, can't calculate fluence");
}


void LaserEnergyMeasurementWidget::onReadedLaserFluenceAndPositionVoltage(double fluence, double position, bool valid)
{
    if(valid)
    {
        labelMeasuredLaserFluence->setText(QString("%0 mJ/mm² (%1 V)").arg(this->locale().toString(fluence, 'f', 3)).arg(this->locale().toString(position, 'f', 3)));
        labelMeasuredLaserFluence->setToolTip(QString("Readed voltage: %0 V").arg(this->locale().toString(position, 'f', 3)));
    }
    else
    {
        labelMeasuredLaserFluence->setText("Pos out of range, can't calculate fluence");
        labelMeasuredLaserFluence->setToolTip(QString("Readed voltage: %0 V, not in range").arg(this->locale().toString(position, 'f', 3)));
    }

    positionValid = valid;
    lastPosition = position;
}
