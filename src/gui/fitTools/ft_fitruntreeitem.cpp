#include "ft_fitruntreeitem.h"

#include "ft_runlistfitdataitem.h"

FT_FitRunTreeItem::FT_FitRunTreeItem(FT_ResultVisualization *visualizationWidget,
                                     FitRun *fitRun,
                                     QTreeWidgetItem *parent) : QObject(),
                                                                QTreeWidgetItem(parent,
                                                                   QTreeWidgetItem::UserType+10)
{
    mFitRun = fitRun;
    mVisualizationWidget = visualizationWidget;

    QWidget *widgetBoxButton = new QWidget;
    QHBoxLayout *layoutBoxButton = new QHBoxLayout;
    layoutBoxButton->setMargin(0);
    widgetBoxButton->setLayout(layoutBoxButton);

    checkboxFitRun = new QCheckBox();
    checkboxFitRun->setTristate();
    checkboxFitRun->setChecked(true);

    buttonText = new QPushButton(QString("%0 (%1, %2)").arg(mFitRun->name())
                                 .arg(mFitRun->creationDate().time().toString())
                                 .arg(mFitRun->creationDate().date().toString("ddd MM-dd-yyyy")));
    buttonText->setFlat(true);

    layoutBoxButton->addWidget(checkboxFitRun);
    layoutBoxButton->addWidget(buttonText);
    layoutBoxButton->addStretch(-1);

    if(parent && treeWidget())
    {
        treeWidget()->setItemWidget(this, 0, widgetBoxButton);
    }

    connect(fitRun, SIGNAL(fitFinished()), SLOT(onFitFinished()));
    connect(checkboxFitRun, SIGNAL(stateChanged(int)), SLOT(onCheckboxStateChanged(int)));
    connect(buttonText, SIGNAL(clicked(bool)), SLOT(onButtonTextClicked()));
}


FT_FitRunTreeItem::~FT_FitRunTreeItem()
{
    while(childCount() > 0)
    {
        FT_RunListFitDataItem *item = dynamic_cast<FT_RunListFitDataItem*>(child(0));
        removeChild(item);
        delete item;
    }
}


void FT_FitRunTreeItem::cleanup()
{
    while(childCount() > 0)
    {
        FT_RunListFitDataItem *item = dynamic_cast<FT_RunListFitDataItem*>(child(0));
        removeChild(item);
        item->blockSignals(true);
        item->cleanup();
        item->blockSignals(true);
        delete item;
    }
}


void FT_FitRunTreeItem::onFitFinished()
{
    for(int i = 0; i < mFitRun->count(); i++)
    {
        FT_RunListFitDataItem *item = new FT_RunListFitDataItem(mVisualizationWidget, mFitRun->at(i), this);
        connect(item, SIGNAL(stateChanged()), SLOT(onChildStateChanged()));
        item->setChecked(true);
    }
    emit treeWidget()->itemClicked(this, 0);
}


void FT_FitRunTreeItem::onChildStateChanged()
{
    checkboxFitRun->blockSignals(true);

    bool allChecked = true;
    bool someChecked = false;

    for(int i = 0; i < childCount(); i++)
    {
        FT_RunListFitDataItem *item = dynamic_cast<FT_RunListFitDataItem*>(child(i));
        if(item->isChecked())
            someChecked = true;
        else
            allChecked = false;
    }

    if(allChecked)
        checkboxFitRun->setCheckState(Qt::Checked);
    else if(someChecked)
        checkboxFitRun->setCheckState(Qt::PartiallyChecked);
    else
        checkboxFitRun->setCheckState(Qt::Unchecked);

    checkboxFitRun->blockSignals(false);
}


void FT_FitRunTreeItem::onCheckboxStateChanged(int state)
{
    if(state == Qt::Unchecked)
    {
        for(int i = 0; i < childCount(); i++)
        {
            FT_RunListFitDataItem *item = dynamic_cast<FT_RunListFitDataItem*>(child(i));
            item->setChecked(false);
        }
    }
    else if(state == Qt::Checked || state == Qt::PartiallyChecked)
    {
        checkboxFitRun->blockSignals(true);
        checkboxFitRun->setChecked(true);
        checkboxFitRun->blockSignals(false);

        for(int i = 0; i < childCount(); i++)
        {
            FT_RunListFitDataItem *item = dynamic_cast<FT_RunListFitDataItem*>(child(i));
            item->blockSignals(true);
            item->setChecked(true);
            item->blockSignals(false);
        }
    }

    //emit treeWidget()->itemClicked(this, 0);
}


void FT_FitRunTreeItem::onButtonTextClicked()
{
    emit treeWidget()->itemClicked(this, 0);
}
