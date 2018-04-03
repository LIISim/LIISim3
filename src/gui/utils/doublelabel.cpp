#include "doublelabel.h"

DoubleLabel::DoubleLabel(QWidget *parent) : QWidget(parent)
{
    layoutMain = new QHBoxLayout(this);
    layoutMain->setMargin(0);
    setLayout(layoutMain);

    first = new QLabel(this);
    second = new QLabel(this);
}

DoubleLabel::DoubleLabel(QString firstText, QString secondText, QWidget *parent)
{
    layoutMain = new QHBoxLayout(this);
    layoutMain->setMargin(0);
    setLayout(layoutMain);

    first = new QLabel(firstText, this);
    second = new QLabel(secondText, this);

    layoutMain->addWidget(first);
    layoutMain->addWidget(second);
}

void DoubleLabel::setFirstText(QString text)
{
    first->setText(text);
}

void DoubleLabel::setSecondText(QString text)
{
    second->setText(text);
}
