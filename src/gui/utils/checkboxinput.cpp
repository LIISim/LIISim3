#include "checkboxinput.h"

CheckboxInput::CheckboxInput(QWidget *parent) : PluginInputField(parent)
{
    layBoxH = new QHBoxLayout;
    setLayout(layBoxH);
    layBoxH->setMargin(0);
    checkBox = new QCheckBox;
    layBoxH->addWidget(checkBox);

    connect(checkBox, SIGNAL(released()), this, SLOT(onValueChanged()));
}

void CheckboxInput::onValueChanged()
{
    emit parameterChanged();
}


void CheckboxInput::init()
{

}


// implementation of the PluginInputField interface

void CheckboxInput::setPluginParamValue(QVariant value)
{
    checkBox->setChecked(value.toBool());
}

void CheckboxInput::setPluginParamMinValue(QVariant minValue)
{
    // not needed for checkbox
}

void CheckboxInput::setPluginParamMaxValue(QVariant maxValue)
{
     // not needed for checkbox
}

void CheckboxInput::setPluginLabelText(QString labelText)
{
    checkBox->setText(labelText);
}

void CheckboxInput::setPluginLabelList(QList<QString> labels)
{
    // not needed in this class
}

void CheckboxInput::setPluginValueList(QList<QVariant> values)
{
    // not needed in this class
}

QVariant CheckboxInput::getPluginParamValue()
{
    return checkBox->isChecked();
}
