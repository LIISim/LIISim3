#include "numberlineedit.h"
#include <QDoubleValidator>
#include <QDebug>

NumberLineEdit::NumberLineEdit(QWidget *parent) : QLineEdit(parent)
{
    useValidation = false;
    validator = NULL;
    useMinMaxValidation = false;
}


NumberLineEdit::~NumberLineEdit()
{
  //  if( validator != NULL)
  //      delete validator;
}


NumberLineEdit::NumberLineEdit(NumberType ntype, QWidget *parent): QLineEdit(parent)
{
    init(ntype);
    useMinMaxValidation = false;
}


NumberLineEdit::NumberLineEdit(NumberType ntype, const QString &tooltip, QWidget *parent): QLineEdit(parent)
{
    init(ntype);
    setToolTip(tooltip);
    useMinMaxValidation = false;
}


void NumberLineEdit::setValue(double d)
{
    QString str;
    str.setNum(d);
    this->setText(str);
}


void NumberLineEdit::setMinValue(double minv)
{
    this->minValue = minv;
    useMinMaxValidation = true;
}


void NumberLineEdit::setMaxValue(double maxv)
{
    this->maxValue = maxv;
    useMinMaxValidation = true;
}


void NumberLineEdit::setText(const QString & str)
{
    QLineEdit::setText(str);
}


double NumberLineEdit::getValue()
{
    if (this->text().isEmpty()) return 0.0;
    return this->text().toDouble();
}


double NumberLineEdit::getValueWithinLimits()
{
    if (this->text().isEmpty()) return 0.0;

    double curValue = this->text().toDouble();

    if(curValue < minValue)
    {
        setValue(minValue);
        return minValue;
    }
    else if(curValue > maxValue)
    {
        setValue(maxValue);
        return maxValue;
    }
    else
        return curValue;
}


void NumberLineEdit::init(NumberType ntype)
{
    switch (ntype) {
    case NOVALIDATION:
        useValidation = false;
        validator = NULL;
        break;
    case INTEGER:
        useValidation = true;
        validator = new QIntValidator(this) ;
        setValidator(validator);
        break;
    case DOUBLE:
        useValidation = true;
        validator = new QDoubleValidator(this);
        setValidator(validator);
        break;
    default:
        break;
    }
    connect(this,SIGNAL(textChanged(QString)),this,SLOT(onTextChanged(QString)));
}


void NumberLineEdit::onTextChanged(const QString &str)
{
   // qDebug() << "NumberLineEdit.onTextChanged() " << str;
    int invdot = str.indexOf(',');

    QString s = str;
    if(invdot > -1)
    {
        s.replace(',',tr("."));
        setText(s);
    }

    if(useMinMaxValidation)
    {
        double curValue = s.toDouble();
        if(curValue < minValue || curValue > maxValue)
        {
            this->setStyleSheet("QLineEdit{background: red;}");
        }
        else
            this->setStyleSheet("QLineEdit{background: white;}");
    }
    emit valueChanged();
}
