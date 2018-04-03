#include "labeledlineedit.h"

LabeledLineEdit::LabeledLineEdit(QWidget *parent) :  PluginInputField(parent)
{
    init(NumberLineEdit::DOUBLE);
}


/**
 * @brief LabeledLineEdit::LabeledLineEdit
 * @param descr text for description label
 * @param parent
 */
LabeledLineEdit::LabeledLineEdit(QString descr, QWidget *parent):  PluginInputField(parent)
{
    init(NumberLineEdit::DOUBLE);
    description->setText(descr);
}


/**
 * @brief LabeledLineEdit::LabeledLineEdit
 * @param descr text for description label
 * @param ntype validation type for lineedit content
 * @param parent
 */
LabeledLineEdit::LabeledLineEdit(QString descr, NumberLineEdit::NumberType ntype, QWidget *parent):  PluginInputField(parent)
{
    init(ntype);
    description->setText(descr);
}



void  LabeledLineEdit::init()
{
}


/**
 * @brief GUI inittialization
 * @param ntype validation type for lineedit
 */
void LabeledLineEdit::init(NumberLineEdit::NumberType ntype)
{
    layMainH = new QHBoxLayout;
    layMainH->setMargin(0);
    setLayout(layMainH);

    description = new QLabel;
    field =new NumberLineEdit(ntype);
    //field->setAlignment(Qt::AlignRight);
    field->setMaximumWidth(100);
    connect(field,SIGNAL(valueChanged()),this,SLOT(onValueChanged()));

    field->editingFinished();
    connect(field,SIGNAL(editingFinished()),this,SLOT(onEditingFinished()));

    layMainH->addWidget(description);
    layMainH->addWidget(field);
}


void LabeledLineEdit::setValue(double d)
{
    field->setValue(d);
}


/**
 * @brief LabeledLineEdit::setEmpty remove all content
 */
void LabeledLineEdit::setEmpty()
{
    field->setText(QString());
}


void LabeledLineEdit::setMaxFieldWidth(int width)
{
    field->setMaximumWidth(width);
}


double LabeledLineEdit::getValue()
{
    return field->getValue();
}


void LabeledLineEdit::onValueChanged()
{
   // emit valueChanged();

}

void LabeledLineEdit::onEditingFinished()
{
    emit valueChanged();
    emit parameterChanged();
}


// implementation of the PluginInputField interface

void LabeledLineEdit::setPluginParamValue(QVariant value)
{
    field->setValue(value.toDouble());
}


void LabeledLineEdit::setPluginParamMinValue(QVariant minValue)
{
    field->setMinValue(minValue.toDouble());
}


void LabeledLineEdit::setPluginParamMaxValue(QVariant maxValue)
{
     field->setMaxValue(maxValue.toDouble());
}


void LabeledLineEdit::setPluginLabelText(QString labelText)
{
    description->setText(labelText);
}


void LabeledLineEdit::setPluginLabelList(QList<QString> labels)
{
    // not needed in this class
}

void LabeledLineEdit::setPluginValueList(QList<QVariant> values)
{
    // not needed in this class
}

QVariant LabeledLineEdit::getPluginParamValue()
{
    return field->getValue();
}

