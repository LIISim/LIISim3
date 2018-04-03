#include "da_referencewidget.h"

#include "gui/utils/signalplotwidgetqwt.h"
#include <QDateTime>

DA_ReferenceElementWidget::DA_ReferenceElementWidget(QString text, QString tooltip, int channel, QColor color, QWidget *parent) : QWidget(parent)
{
    this->color = color;
    this->channel = channel;

    layout = new QHBoxLayout(this);
    checkboxReference = new QCheckBox(text, this);
    checkboxReference->setToolTip(tooltip);
    checkboxReference->setChecked(true);
    buttonRemove = new QToolButton(this);
    buttonRemove->setText("X");
    buttonRemove->setMinimumWidth(15);

    layout->addWidget(checkboxReference);
    layout->addWidget(buttonRemove);

    setLayout(layout);
    layout->setContentsMargins(5, 0, 5, 0);

    connect(checkboxReference, SIGNAL(stateChanged(int)), SLOT(onCheckboxReferenceStateChanged(int)));
    connect(buttonRemove, SIGNAL(released()), SLOT(onButtonRemoveReleased()));
}


DA_ReferenceElementWidget::~DA_ReferenceElementWidget()
{
    delete layout;
    delete checkboxReference;
    delete buttonRemove;
}


int DA_ReferenceElementWidget::getCheckedState()
{
    return checkboxReference->checkState();
}


QString DA_ReferenceElementWidget::getText()
{
    return checkboxReference->text();
}


QString DA_ReferenceElementWidget::getTooltip()
{
    return checkboxReference->toolTip();
}


void DA_ReferenceElementWidget::onCheckboxReferenceStateChanged(int state)
{
    emit stateChanged(state);
}


void DA_ReferenceElementWidget::onButtonRemoveReleased()
{
    emit removeRequest(this);
}


// ---


DA_ReferenceWidget::DA_ReferenceWidget(QWidget *parent) : RibbonToolBox("REFERENCE", parent)
{
    addCounter = 1;

    buttonFreeze = new QToolButton(this);
    buttonFreeze->setText("Freeze");
    buttonFreeze->setMinimumWidth(50);

    buttonClear = new QToolButton(this);
    buttonClear->setText("Clear");
    buttonClear->setMinimumWidth(50);

    addWidget(buttonFreeze, 0, 0);
    addWidget(buttonClear, 1, 0);

    connect(buttonFreeze, SIGNAL(released()), SLOT(freezeReference()));
    connect(buttonClear, SIGNAL(released()), SLOT(clearReferences()));
}


DA_ReferenceWidget::~DA_ReferenceWidget()
{

}


void DA_ReferenceWidget::draw(SignalPlotWidgetQwt *plot)
{
    int colorFactor = 110;
    int colorAdd = 0;
    if(references.size() > 0)
        colorAdd = (300 - colorFactor) / references.size();

    for(int i = 0; i < references.size(); i++)
    {
        if(references.at(i).first->getCheckedState() == Qt::Checked)
        {
            QColor color = references.at(i).first->color.darker(colorFactor + colorAdd * i);

            plot->setCurrentColor(color);
            plot->addSignal(references.at(i).second, references.at(i).first->getText(), false);
        }
    }
}


void DA_ReferenceWidget::addReference(StreamPoint point)
{
    for(int i = 1; i < 5; i++)
    {
        if(point.average.contains(i))
        {
            QString label;
            QColor color;
            switch(i)
            {
            case 1: label = QString("A(%0)").arg(addCounter); color = QColor("blue"); break;
            case 2: label = QString("B(%0)").arg(addCounter); color = QColor("red"); break;
            case 3: label = QString("C(%0)").arg(addCounter); color = QColor("green"); break;
            case 4: label = QString("D(%0)").arg(addCounter); color = QColor("yellow"); break;
            }

            DA_ReferenceElementWidget *widget = new DA_ReferenceElementWidget(label, QDateTime::currentDateTime().time().toString(), i, color);
            connect(widget, SIGNAL(removeRequest(QWidget*)), SLOT(onRemoveRequest(QWidget*)));
            connect(widget, SIGNAL(stateChanged(int)), SLOT(elementsChanged()));

            references.append(QPair<DA_ReferenceElementWidget*, Signal>(widget, point.average.value(i)));

            reorganise();
        }
    }
    addCounter++;

    emit referencesChanged(references);
}


void DA_ReferenceWidget::freezeReference()
{
    emit referenceRequest();
}


void DA_ReferenceWidget::clearReferences()
{
    while(references.size() > 0)
    {
        delete references.at(0).first;
        references.removeAt(0);
    }
    addCounter = 1;
    //reorganise();
    emit shouldRedraw();

    emit referencesChanged(references);
}


void DA_ReferenceWidget::elementsChanged()
{
    emit shouldRedraw();

    emit referencesChanged(references);
}


void DA_ReferenceWidget::onRemoveRequest(QWidget *widget)
{
    this->layout()->removeWidget(widget);

    for(int i = 0; i < references.size(); i++)
    {
        if(references.at(i).first == widget)
        {
            delete widget;
            references.removeAt(i);
        }
    }
    reorganise();
    emit shouldRedraw();

    emit referencesChanged(references);
}


void DA_ReferenceWidget::reorganise()
{
    int col = 1;
    int row = 0;

    for(int i = 0; i < references.size(); i++)
    {
        this->layout()->removeWidget(references.at(i).first);
    }

    for(int i = 0; i < references.size(); i++)
    {
        addWidget(references.at(i).first, row, col);

        row++;
        if(row >= 3)
        {
            col++;
            row = 0;
        }
    }
}

