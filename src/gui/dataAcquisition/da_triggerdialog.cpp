#include "da_triggerdialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include "./core.h"
#include <QGroupBox>

/* Helper class TriggerWidget implementation */

TriggerWidget::TriggerWidget(QString description, enPS6000TriggerState state, enPS6000ThresholdMode mode,
                             enPS6000ThresholdDirection direction, double upperThreshold, double upperHysteresis,
                             double lowerThreshold, double lowerHysteresis, PSRange range, QWidget *parent) : QWidget(parent)
{
    QGroupBox *groupBox = new QGroupBox(description, this);

    QHBoxLayout *layout = new QHBoxLayout(this);

    comboxCondition = new QComboBox(this);
    comboxCondition->addItem("Don't Care", 0);
    comboxCondition->addItem("True", 1);
    comboxCondition->addItem("False", 2);

    comboxType = new QComboBox(this);
    comboxType->addItem("Level", 0);
    comboxType->addItem("Window", 1);

    comboxDirection = new QComboBox(this);

    layout->addWidget(comboxCondition);
    comboxCondition->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
    comboxCondition->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContentsOnFirstShow);
    layout->setAlignment(comboxCondition, Qt::AlignTop | Qt::AlignLeft);
    layout->addWidget(comboxType);
    comboxType->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContentsOnFirstShow);
    layout->setAlignment(comboxType, Qt::AlignTop);
    layout->addWidget(comboxDirection);
    comboxDirection->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
    layout->setAlignment(comboxDirection, Qt::AlignTop);

    QGridLayout *layoutGridProperties = new QGridLayout();

    labelUpperThreshold = new QLabel("Upper Threshold " + rangeToString(range), this);
    labelUpperThresholdHysteresis = new QLabel("Upper Hysteresis " + rangeToString(range), this);
    labelLowerThreshold = new QLabel("Lower Threshold " + rangeToString(range), this);
    labelLowerThresholdHysteresis = new QLabel("Lower Hysteresis " + rangeToString(range), this);

    numUpperThreshold = new NumberLineEdit(NumberLineEdit::NumberType::DOUBLE);
    numUpperHysteresis = new NumberLineEdit(NumberLineEdit::NumberType::DOUBLE);
    numLowerThreshold = new NumberLineEdit(NumberLineEdit::NumberType::DOUBLE);
    numLowerHysteresis = new NumberLineEdit(NumberLineEdit::NumberType::DOUBLE);

    layoutGridProperties->addWidget(labelUpperThreshold, 0, 0, 1, 1);
    layoutGridProperties->addWidget(numUpperThreshold, 0, 1, 1, 1);
    layoutGridProperties->addWidget(labelUpperThresholdHysteresis, 1, 0, 1, 1);
    layoutGridProperties->addWidget(numUpperHysteresis, 1, 1, 1, 1);
    layoutGridProperties->addWidget(labelLowerThreshold, 2, 0, 1, 1);
    layoutGridProperties->addWidget(numLowerThreshold, 2, 1, 1, 1);
    layoutGridProperties->addWidget(labelLowerThresholdHysteresis, 3, 0, 1, 1);
    layoutGridProperties->addWidget(numLowerHysteresis, 3, 1, 1, 1);

    layout->addLayout(layoutGridProperties);

    groupBox->setLayout(layout);

    QHBoxLayout *layoutMain = new QHBoxLayout(this);
    layoutMain->addWidget(groupBox);
    this->setLayout(layoutMain);

    connect(comboxCondition, SIGNAL(currentIndexChanged(int)), SLOT(onConditionChanged(int)));
    connect(comboxType, SIGNAL(currentIndexChanged(int)), SLOT(onTypeChanged(int)));
    connect(comboxDirection, SIGNAL(currentIndexChanged(int)), SLOT(onDirectionChanged(int)));
    onTypeChanged();
    onConditionChanged();

    comboxCondition->setCurrentIndex(stateToInt(state));
    if(stateToInt(state) != 0)
    {
        comboxType->setCurrentIndex(modeToInt(mode));
        comboxDirection->setCurrentIndex(directionToInt(mode, direction));
    }

    numUpperThreshold->setText(QString::number(upperThreshold));
    numUpperHysteresis->setText(QString::number(upperHysteresis));
    numLowerThreshold->setText(QString::number(lowerThreshold));
    numLowerHysteresis->setText(QString::number(lowerHysteresis));
}

void TriggerWidget::onConditionChanged(int index)
{
    switch(comboxCondition->currentData().toUInt())
    {
    case 0:
        comboxType->hide();
        comboxDirection->hide();
        numUpperThreshold->hide();
        numUpperHysteresis->hide();
        numLowerThreshold->hide();
        numLowerHysteresis->hide();
        labelUpperThreshold->hide();
        labelUpperThresholdHysteresis->hide();
        labelLowerThreshold->hide();
        labelLowerThresholdHysteresis->hide();
        break;
    default:
        comboxDirection->show();
        comboxType->show();
        comboxDirection->show();
        onDirectionChanged();
        break;
    }
}

void TriggerWidget::onTypeChanged(int index)
{
    switch(comboxType->currentData().toUInt())
    {
    case 0:
        comboxDirection->clear();
        comboxDirection->addItem("Above Upper Threshold", 0);
        comboxDirection->addItem("Above Lower Threshold", 1);
        comboxDirection->addItem("Below Upper Threshold", 2);
        comboxDirection->addItem("Below Lower Threshold", 3);
        comboxDirection->addItem("Rising Edge (Upper Threshold)", 4);
        comboxDirection->addItem("Rising Edge (Lower Threshold)", 5);
        comboxDirection->addItem("Falling Edge (Upper Threshold)", 6);
        comboxDirection->addItem("Falling Edge (Lower Threshold)", 7);
        comboxDirection->addItem("Rising or Falling Edge", 8);
        break;
    case 1:
        comboxDirection->clear();
        comboxDirection->addItem("Inside Window", 9);
        comboxDirection->addItem("Outside Window", 10);
        comboxDirection->addItem("Entering Window", 11);
        comboxDirection->addItem("Exiting Window", 12);
        comboxDirection->addItem("Entering or Exiting Window", 13);
        comboxDirection->addItem("Positive Runt", 14);
        comboxDirection->addItem("Negative Runt", 15);
        break;
    }
}

void TriggerWidget::onDirectionChanged(int index)
{
    switch(comboxDirection->currentData().toUInt())
    {
    case 0:
    case 2:
    case 4:
    case 6:
        //show upper threshold
        numUpperThreshold->show();
        numUpperHysteresis->show();
        labelUpperThreshold->show();
        labelUpperThresholdHysteresis->show();
        //hide lower threshold
        numLowerThreshold->hide();
        numLowerHysteresis->hide();
        labelLowerThreshold->hide();
        labelLowerThresholdHysteresis->hide();
        break;
    case 1:
    case 3:
    case 5:
    case 7:
        //show lower threshold
        numLowerThreshold->show();
        numLowerHysteresis->show();
        labelLowerThreshold->show();
        labelLowerThresholdHysteresis->show();
        //hide upper threshold
        numUpperThreshold->hide();
        numUpperHysteresis->hide();
        labelUpperThreshold->hide();
        labelUpperThresholdHysteresis->hide();
        break;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        //show both thresholds
        numUpperThreshold->show();
        numUpperHysteresis->show();
        numLowerThreshold->show();
        numLowerHysteresis->show();
        labelUpperThreshold->show();
        labelUpperThresholdHysteresis->show();
        labelLowerThreshold->show();
        labelLowerThresholdHysteresis->show();
        break;
    }
}


unsigned int TriggerWidget::stateToInt(enPS6000TriggerState state)
{
    switch(state)
    {
    case PS6000_CONDITION_DONT_CARE:    return 0;   break;
    case PS6000_CONDITION_TRUE:         return 1;   break;
    case PS6000_CONDITION_FALSE:        return 2;   break;
    }
}


unsigned int TriggerWidget::modeToInt(enPS6000ThresholdMode mode)
{
    switch(mode)
    {
    case PS6000_LEVEL:  return 0;   break;
    case PS6000_WINDOW: return 1;   break;
    }
}


unsigned int TriggerWidget::directionToInt(enPS6000ThresholdMode mode, enPS6000ThresholdDirection direction)
{
    switch(mode)
    {
    case PS6000_LEVEL:
        switch(direction)
        {
        case PS6000_ABOVE:              return 0;   break;
        case PS6000_BELOW:              return 2;   break;
        case PS6000_RISING:             return 4;   break;
        case PS6000_FALLING:            return 6;   break;
        case PS6000_RISING_OR_FALLING:  return 8;   break;
        case PS6000_ABOVE_LOWER:        return 1;   break;
        case PS6000_BELOW_LOWER:        return 3;   break;
        case PS6000_RISING_LOWER:       return 5;   break;
        case PS6000_FALLING_LOWER:      return 7;   break;
        }
        break;

    case PS6000_WINDOW:
        switch(direction)
        {
        case PS6000_INSIDE:         return 9;   break;
        case PS6000_OUTSIDE:        return 10;  break;
        case PS6000_ENTER:          return 11;  break;
        case PS6000_EXIT:           return 12;  break;
        case PS6000_ENTER_OR_EXIT:  return 13;  break;
        case PS6000_POSITIVE_RUNT:  return 14;  break;
        case PS6000_NEGATIVE_RUNT:  return 15;  break;
        }
        break;
    }
}


QString TriggerWidget::rangeToString(PSRange range)
{
    switch(range)
    {
    case PSRange::R50mV:    return QString("(± 50mV)");     break;
    case PSRange::R100mV:   return QString("(± 100mV)");    break;
    case PSRange::R200mV:   return QString("(± 200mV)");    break;
    case PSRange::R500mV:   return QString("(± 500mV)");    break;
    case PSRange::R1V:      return QString("(± 1V)");       break;
    case PSRange::R2V:      return QString("(± 2V)");       break;
    case PSRange::R5V:      return QString("(± 5V)");       break;
    case PSRange::R10V:     return QString("(± 10V)");      break;
    case PSRange::R20V:     return QString("(± 20V)");      break;
    }
}



/* ---- ---- */

DA_TriggerDialog::DA_TriggerDialog(PicoScopeSettings *settings, QWidget *parent) : QDialog(parent)
{
    this->settings = settings;

    setWindowTitle(tr("PicoScope Trigger"));
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout(this);

    widgetChannelA = new TriggerWidget("Channel A", settings->triggerState(PSChannel::A), settings->triggerMode(PSChannel::A),
                                                settings->triggerDirection(PSChannel::A), settings->upperThreshold(PSChannel::A),
                                                settings->upperHysteresis(PSChannel::A), settings->lowerThreshold(PSChannel::A),
                                                settings->lowerHysteresis(PSChannel::A), settings->range(PSChannel::A), this);
    widgetChannelB = new TriggerWidget("Channel B", settings->triggerState(PSChannel::B), settings->triggerMode(PSChannel::B),
                                                settings->triggerDirection(PSChannel::B), settings->upperThreshold(PSChannel::B),
                                                settings->upperHysteresis(PSChannel::B), settings->lowerThreshold(PSChannel::B),
                                                settings->lowerHysteresis(PSChannel::B), settings->range(PSChannel::B), this);
    widgetChannelC = new TriggerWidget("Channel C", settings->triggerState(PSChannel::C), settings->triggerMode(PSChannel::C),
                                                settings->triggerDirection(PSChannel::C), settings->upperThreshold(PSChannel::C),
                                                settings->upperHysteresis(PSChannel::C), settings->lowerThreshold(PSChannel::C),
                                                settings->lowerHysteresis(PSChannel::C), settings->range(PSChannel::C), this);
    widgetChannelD = new TriggerWidget("Channel D", settings->triggerState(PSChannel::D), settings->triggerMode(PSChannel::D),
                                                settings->triggerDirection(PSChannel::D), settings->upperThreshold(PSChannel::D),
                                                settings->upperHysteresis(PSChannel::D), settings->lowerThreshold(PSChannel::D),
                                                settings->lowerHysteresis(PSChannel::D), settings->range(PSChannel::D), this);
    widgetAUX = new TriggerWidget("AUX", settings->triggerState(PSChannel::AUX), settings->triggerMode(PSChannel::AUX),
                                           settings->triggerDirection(PSChannel::AUX), settings->upperThreshold(PSChannel::AUX),
                                           settings->upperHysteresis(PSChannel::AUX), settings->lowerThreshold(PSChannel::AUX),
                                           settings->lowerHysteresis(PSChannel::AUX), PSRange::R1V, this);

    layout->addWidget(widgetChannelA);
    layout->addWidget(widgetChannelB);
    layout->addWidget(widgetChannelC);
    layout->addWidget(widgetChannelD);
    layout->addWidget(widgetAUX);

    QWidget *buttonWidget = new QWidget(this);
    QHBoxLayout *buttonWidgetLayout = new QHBoxLayout(buttonWidget);
    buttonOK = new QPushButton("OK", this);
    buttonCancel = new QPushButton("Cancel", this);
    buttonWidgetLayout->addWidget(buttonCancel);
    buttonWidgetLayout->addWidget(buttonOK);
    buttonWidget->setLayout(buttonWidgetLayout);
    layout->addWidget(buttonWidget);
    layout->setAlignment(buttonWidget, Qt::AlignRight);

    this->setLayout(layout);

    connect(buttonOK, SIGNAL(clicked(bool)), SLOT(onOKClicked(bool)));
    connect(buttonCancel, SIGNAL(clicked(bool)), SLOT(onCancelClicked(bool)));
}


void DA_TriggerDialog::onOKClicked(bool checked)
{
    //Channel A
    settings->setTriggerState(PSChannel::A, intToState(widgetChannelA->comboxCondition->currentData().toUInt()));
    settings->setTriggerMode(PSChannel::A, intToMode(widgetChannelA->comboxType->currentData().toUInt()));
    settings->setTriggerDirection(PSChannel::A, intToDirection(widgetChannelA->comboxDirection->currentData().toUInt()));
    settings->setUpperThreshold(PSChannel::A, widgetChannelA->numUpperThreshold->getValue(), widgetChannelA->numUpperHysteresis->getValue());
    settings->setLowerThreshold(PSChannel::A, widgetChannelA->numLowerThreshold->getValue(), widgetChannelA->numLowerHysteresis->getValue());
    //Channel B
    settings->setTriggerState(PSChannel::B, intToState(widgetChannelB->comboxCondition->currentData().toUInt()));
    settings->setTriggerMode(PSChannel::B, intToMode(widgetChannelB->comboxType->currentData().toUInt()));
    settings->setTriggerDirection(PSChannel::B, intToDirection(widgetChannelB->comboxDirection->currentData().toUInt()));
    settings->setUpperThreshold(PSChannel::B, widgetChannelB->numUpperThreshold->getValue(), widgetChannelB->numUpperHysteresis->getValue());
    settings->setLowerThreshold(PSChannel::B, widgetChannelB->numLowerThreshold->getValue(), widgetChannelB->numLowerHysteresis->getValue());
    //Channel C
    settings->setTriggerState(PSChannel::C, intToState(widgetChannelC->comboxCondition->currentData().toUInt()));
    settings->setTriggerMode(PSChannel::C, intToMode(widgetChannelC->comboxType->currentData().toUInt()));
    settings->setTriggerDirection(PSChannel::C, intToDirection(widgetChannelC->comboxDirection->currentData().toUInt()));
    settings->setUpperThreshold(PSChannel::C, widgetChannelC->numUpperThreshold->getValue(), widgetChannelC->numUpperHysteresis->getValue());
    settings->setLowerThreshold(PSChannel::C, widgetChannelC->numLowerThreshold->getValue(), widgetChannelC->numLowerHysteresis->getValue());
    //Channel D
    settings->setTriggerState(PSChannel::D, intToState(widgetChannelD->comboxCondition->currentData().toUInt()));
    settings->setTriggerMode(PSChannel::D, intToMode(widgetChannelD->comboxType->currentData().toUInt()));
    settings->setTriggerDirection(PSChannel::D, intToDirection(widgetChannelD->comboxDirection->currentData().toUInt()));
    settings->setUpperThreshold(PSChannel::D, widgetChannelD->numUpperThreshold->getValue(), widgetChannelD->numUpperHysteresis->getValue());
    settings->setLowerThreshold(PSChannel::D, widgetChannelD->numLowerThreshold->getValue(), widgetChannelC->numLowerHysteresis->getValue());
    //Channel AUX
    settings->setTriggerState(PSChannel::AUX, intToState(widgetAUX->comboxCondition->currentData().toUInt()));
    settings->setTriggerMode(PSChannel::AUX, intToMode(widgetAUX->comboxType->currentData().toUInt()));
    settings->setTriggerDirection(PSChannel::AUX, intToDirection(widgetAUX->comboxDirection->currentData().toUInt()));
    settings->setUpperThreshold(PSChannel::AUX, widgetAUX->numUpperThreshold->getValue(), widgetAUX->numUpperHysteresis->getValue());
    settings->setLowerThreshold(PSChannel::AUX, widgetAUX->numLowerThreshold->getValue(), widgetAUX->numLowerHysteresis->getValue());

    this->close();
}

void DA_TriggerDialog::onCancelClicked(bool checked)
{
    this->close();
}



unsigned int DA_TriggerDialog::stateToInt(enPS6000TriggerState state)
{
    switch(state)
    {
    case PS6000_CONDITION_DONT_CARE:    return 0;   break;
    case PS6000_CONDITION_TRUE:         return 1;   break;
    case PS6000_CONDITION_FALSE:        return 2;   break;
    }
}


unsigned int DA_TriggerDialog::modeToInt(enPS6000ThresholdMode mode)
{
    switch(mode)
    {
    case PS6000_LEVEL:  return 0;   break;
    case PS6000_WINDOW: return 1;   break;
    }
}


unsigned int DA_TriggerDialog::directionToInt(enPS6000ThresholdMode mode, enPS6000ThresholdDirection direction)
{
    switch(mode)
    {
    case PS6000_LEVEL:
        switch(direction)
        {
        case PS6000_ABOVE:              return 0;   break;
        case PS6000_BELOW:              return 2;   break;
        case PS6000_RISING:             return 4;   break;
        case PS6000_FALLING:            return 6;   break;
        case PS6000_RISING_OR_FALLING:  return 8;   break;
        case PS6000_ABOVE_LOWER:        return 1;   break;
        case PS6000_BELOW_LOWER:        return 3;   break;
        case PS6000_RISING_LOWER:       return 5;   break;
        case PS6000_FALLING_LOWER:      return 7;   break;
        }
        break;

    case PS6000_WINDOW:
        switch(direction)
        {
        case PS6000_INSIDE:         return 9;   break;
        case PS6000_OUTSIDE:        return 10;  break;
        case PS6000_ENTER:          return 11;  break;
        case PS6000_EXIT:           return 12;  break;
        case PS6000_ENTER_OR_EXIT:  return 13;  break;
        case PS6000_POSITIVE_RUNT:  return 14;  break;
        case PS6000_NEGATIVE_RUNT:  return 15;  break;
        }
        break;
    }
}


enPS6000TriggerState DA_TriggerDialog::intToState(unsigned int state)
{
    switch(state)
    {
    case 0: return PS6000_CONDITION_DONT_CARE;  break;
    case 1: return PS6000_CONDITION_TRUE;       break;
    case 2: return PS6000_CONDITION_FALSE;      break;
    }
}


enPS6000ThresholdMode DA_TriggerDialog::intToMode(unsigned int mode)
{
    switch(mode)
    {
    case 0: return PS6000_LEVEL;    break;
    case 1: return PS6000_WINDOW;   break;
    }
}


enPS6000ThresholdDirection DA_TriggerDialog::intToDirection(unsigned int direction)
{
    switch(direction)
    {
    case 0: return PS6000_ABOVE;            break;
    case 2: return PS6000_BELOW;            break;
    case 4: return PS6000_RISING;           break;
    case 6: return PS6000_FALLING;          break;
    case 8: return PS6000_RISING_OR_FALLING;break;
    case 1: return PS6000_ABOVE_LOWER;      break;
    case 3: return PS6000_BELOW_LOWER;      break;
    case 5: return PS6000_RISING_LOWER;     break;
    case 7: return PS6000_FALLING_LOWER;    break;
    case 9: return PS6000_INSIDE;           break;
    case 10: return PS6000_OUTSIDE;         break;
    case 11: return PS6000_ENTER;           break;
    case 12: return PS6000_EXIT;            break;
    case 13: return PS6000_ENTER_OR_EXIT;   break;
    case 14: return PS6000_POSITIVE_RUNT;   break;
    case 15: return PS6000_NEGATIVE_RUNT;   break;
    }
}

