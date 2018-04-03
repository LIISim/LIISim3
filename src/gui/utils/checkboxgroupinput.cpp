#include "checkboxgroupinput.h"

#include <QDebug>

CheckboxGroupInput::CheckboxGroupInput(QWidget *parent) : PluginInputField(parent)
{
    layGrid = new QGridLayout;
    setLayout(layGrid);
    layGrid->setMargin(0);

    labelText = new QLabel;    
}


void CheckboxGroupInput::init()
{
    layGrid->addWidget(labelText, 0, 0, 1, 2);   // Pos (0,0) Colspan=2

    layGrid->setSpacing(0);

    checkBoxes.clear();

    for(int i = 0; i < labels.size(); i++)
    {
        QCheckBox *checkBox = new QCheckBox;

        checkBoxes.append(checkBox);

        checkBoxes.last()->setText(labels.at(i));
        checkBoxes.last()->setChecked(values.at(i));

        layGrid->addWidget(checkBoxes.last(), i, 1); // Pos(1,i)

        connect(checkBoxes.last(), SIGNAL(released()), this, SLOT(onValueChanged()));
    }
}


void CheckboxGroupInput::onValueChanged()
{
    //qDebug() << "CheckboxGroupInput: "<<  labels.at(0);
    emit parameterChanged();
}


// implementation of the PluginInputField interface

void CheckboxGroupInput::setPluginParamValue(QVariant value)
{
    QStringList list = value.toString().split(";");

    values.clear();

    foreach(QString s, list)
    {
        if(s != "")
        {
            // convert string to bool
            values.append((s.toInt() != 0));
        }
    }
}


void CheckboxGroupInput::setPluginParamMinValue(QVariant minValue)
{
    // not needed for checkbox
}

void CheckboxGroupInput::setPluginParamMaxValue(QVariant maxValue)
{
    // not needed for checkbox
}

void CheckboxGroupInput::setPluginLabelText(QString text)
{
    labelText->setText(text);
}


void CheckboxGroupInput::setPluginLabelList(QList<QString> labelList)
{
    labels = labelList;
}


void CheckboxGroupInput::setPluginValueList(QList<QVariant> valueList)
{
    QList<QVariant>::iterator i;

    for(i = valueList.begin(); i != valueList.end(); i++)
        values.append(i->toBool());
}


QVariant CheckboxGroupInput::getPluginParamValue()
{
    QString str;

    values.clear();

    for(int i=0; i < checkBoxes.size(); i++)
    {
        values.append(checkBoxes.at(i)->isChecked());

        str.append(QString("%0;").arg(checkBoxes.at(i)->isChecked()));
    }

    // returns string ith bool values for example: "0;1;1;"
    return str;
}
