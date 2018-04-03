#include "ft_fitlist.h"

#include <QHBoxLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QMenu>
#include <QHeaderView>

#include "core.h"
#include "ft_resultvisualization.h"
#include "ft_datavisualization.h"
#include "ft_fitruntreeitem.h"
#include "ft_simruntreeitem.h"
#include "ft_runlistfitdataitem.h"

#include "../../calculations/fit/fitrun.h"
#include "../../calculations/fit/simrun.h"


FT_FitList::FT_FitList(FT_ResultVisualization *visualizationWidget, FT_DataVisualization *dataVisualization, QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    this->visualizationWidget = visualizationWidget;
    this->dataVisualization = dataVisualization;

    QHBoxLayout *layoutButton = new QHBoxLayout;
    layoutButton->setMargin(0);
    buttonClearList = new QPushButton("Clear FitList", this);
    buttonFit = new QPushButton("Start Fitting");
    buttonFit->setObjectName("FC_BTN_START_FIT");
    buttonSim = new QPushButton("Simulate");
    buttonSim->setObjectName("FC_BTN_SIMULATION");
    buttonCancel = new QPushButton("Cancel");
    buttonCancel->setVisible(false);

    layoutButton->addWidget(buttonFit, 0, Qt::AlignLeft);
    layoutButton->addWidget(buttonCancel, 0, Qt::AlignLeft);
    layoutButton->addWidget(buttonSim, 0, Qt::AlignLeft);
    layoutButton->addStretch(-1);
    layoutButton->addWidget(buttonClearList, 0, Qt::AlignRight);

    mainLayout->addLayout(layoutButton);

    treeWidget = new QTreeWidget(this);
    treeWidget->setColumnCount(7);

    QStringList header;
    header << "FitRun" << "Model" << "Data" << "Iteration" << "Dp" << "T Gas" << "T Peak";
    treeWidget->setHeaderLabels(header);

    // default alignment for all header items
    treeWidget->header()->setDefaultAlignment(Qt::AlignCenter);

    // first header item (0)  alignment
    treeWidget->header()->model()->setHeaderData(0,
                                                 Qt::Horizontal,
                                                 Qt::AlignLeft ,
                                                 Qt::TextAlignmentRole);

    mainLayout->addWidget(treeWidget);

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    actionDelete = new QAction("Remove", this);

    treeWidget->setColumnWidth(0, 260);
    treeWidget->setColumnWidth(1, 45);
    treeWidget->setColumnWidth(2, 35);
    treeWidget->setColumnWidth(3, 80);
    treeWidget->setColumnWidth(4, 60);
    treeWidget->setColumnWidth(5, 55);
    treeWidget->setColumnWidth(6, 55);

    connect(Core::instance()->dataModel(), SIGNAL(newFitRunRegistered(FitRun*)), SLOT(onNewFitRunRegistered(FitRun*)));
    connect(Core::instance()->getSignalManager(), SIGNAL(fitStateChanged(bool)), SLOT(onFitStateChanged(bool)));

    connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(onTreeWidgetItemClicked(QTreeWidgetItem*,int)));
    connect(dataVisualization, SIGNAL(dataTableRowClicked(int)), SLOT(onDataTableCellClicked(int)));

    connect(buttonClearList, SIGNAL(clicked(bool)), SLOT(onButtonClearListClicked()));
    connect(buttonFit, SIGNAL(clicked(bool)), SLOT(onButtonFitClicked()));
    connect(buttonSim, SIGNAL(clicked(bool)), SLOT(onButtonSimClicked()));
    connect(buttonCancel, SIGNAL(clicked(bool)), SLOT(onButtonCancelClicked()));

    connect(treeWidget, SIGNAL(customContextMenuRequested(QPoint)), SLOT(onContextMenuRequested(QPoint)));
    connect(actionDelete, SIGNAL(triggered(bool)), SLOT(onActionDelete()));
}


void FT_FitList::onNewFitRunRegistered(FitRun *fitRun)
{
    if(fitRun)
    {
        FT_FitRunTreeItem *item = new FT_FitRunTreeItem(visualizationWidget, fitRun, treeWidget->invisibleRootItem());
        item->setExpanded(true);
        //emit treeWidget->itemClicked(item, 0);
    }
}


void FT_FitList::onDataTableCellClicked(int row)
{
    for(int i = 0; i < treeWidget->invisibleRootItem()->childCount(); i++)
    {
        if(treeWidget->invisibleRootItem()->child(i)->isSelected())
            return;

        for(int j = 0; j < treeWidget->invisibleRootItem()->child(i)->childCount(); j++)
        {
            if(treeWidget->invisibleRootItem()->child(i)->child(j)->isSelected())
            {
                FT_RunListFitDataItem *item = dynamic_cast<FT_RunListFitDataItem*>(treeWidget->invisibleRootItem()->child(i)->child(j));
                item->setIteration(row);
                return;
            }
        }
    }
}


void FT_FitList::onButtonClearListClicked()
{
    while(treeWidget->invisibleRootItem()->childCount() > 0)
    {
        if(treeWidget->invisibleRootItem()->child(0)->type() == QTreeWidgetItem::UserType+10)
        {
            FT_FitRunTreeItem *item = dynamic_cast<FT_FitRunTreeItem*>(treeWidget->invisibleRootItem()->child(0));
            treeWidget->invisibleRootItem()->removeChild(item);
            item->cleanup();
            delete item;
        }
        else
        {
            FT_SimRunTreeItem *item = dynamic_cast<FT_SimRunTreeItem*>(treeWidget->invisibleRootItem()->child(0));
            treeWidget->invisibleRootItem()->removeChild(item);
            item->cleanup();
            delete item;
        }
    }

    dataVisualization->clearDataTable();
    dataVisualization->clearParameterTable();
}


void FT_FitList::onNewSimRun(SimRun *simRun)
{
    if(simRun)
    {
        FT_SimRunTreeItem *item = new FT_SimRunTreeItem(visualizationWidget, simRun, treeWidget->invisibleRootItem());
        emit treeWidget->itemClicked(item, 0);
    }
}


void FT_FitList::onButtonFitClicked()
{
    emit startFittingClicked();
}


void FT_FitList::onButtonSimClicked()
{
    emit startSimulationClicked();
}


void FT_FitList::onButtonCancelClicked()
{
    emit cancelClicked();
}


void FT_FitList::onFitStateChanged(bool state)
{
    if(state)
    {
        buttonFit->setVisible(false);
        buttonCancel->setVisible(true);
        buttonSim->setEnabled(false);
        buttonClearList->setEnabled(false);
    }
    else
    {
        buttonFit->setVisible(true);
        buttonCancel->setVisible(false);
        buttonSim->setEnabled(true);
        buttonClearList->setEnabled(true);
    }
}


//void FT_FitList::resizeEvent(QResizeEvent *event)
//{
//    treeWidget->setColumnWidth(0, treeWidget->width());

//    int c0 = treeWidget->width();

//    c0 -= treeWidget->columnWidth(1);
//    c0 -= treeWidget->columnWidth(2);
//    c0 -= treeWidget->columnWidth(3);
//    c0 -= treeWidget->columnWidth(4);
//    c0 -= treeWidget->columnWidth(5);
//    //c0 -= 30;

//    treeWidget->setColumnWidth(0, c0);
//}


void FT_FitList::onTreeWidgetItemClicked(QTreeWidgetItem *item, int column)
{
    treeWidget->blockSignals(true);
    for(int i = 0; i < treeWidget->invisibleRootItem()->childCount(); i++)
    {
        treeWidget->invisibleRootItem()->child(i)->setSelected(false);

        for(int j = 0; j < treeWidget->invisibleRootItem()->child(i)->childCount(); j++)
            treeWidget->invisibleRootItem()->child(i)->child(j)->setSelected(false);
    }
    treeWidget->blockSignals(false);

    item->setSelected(true);

    if(item->type() == QTreeWidgetItem::UserType+11)
    {
        FT_RunListFitDataItem *fitDataItem = dynamic_cast<FT_RunListFitDataItem*>(item);
        dataVisualization->update(fitDataItem->mFitData, fitDataItem->getIteration());
    }
    else if(item->type() == QTreeWidgetItem::UserType+10)
    {
        FT_FitRunTreeItem *fitRunItem = dynamic_cast<FT_FitRunTreeItem*>(item);
        dataVisualization->clearDataTable();
        dataVisualization->update(fitRunItem->mFitRun);
    }
    else if(item->type() == QTreeWidgetItem::UserType+12)
    {
        FT_SimRunTreeItem *simRunItem = dynamic_cast<FT_SimRunTreeItem*>(item);
        dataVisualization->clearDataTable();
        dataVisualization->update(simRunItem->mSimRun);
    }
}


void FT_FitList::onContextMenuRequested(const QPoint &point)
{
    QTreeWidgetItem *item = treeWidget->itemAt(point);
    if(!item)
        return;

    if(item->type() == QTreeWidgetItem::UserType+10 || item->type() == QTreeWidgetItem::UserType+12)
    {
        actionDelete->setData(point);

        QMenu menu(this);
        menu.addAction(actionDelete);
        menu.exec(treeWidget->viewport()->mapToGlobal(point));
    }
}


void FT_FitList::onActionDelete()
{
    QTreeWidgetItem *item = treeWidget->itemAt(actionDelete->data().toPoint());
    if(!item)
        return;

    if(item->type() == QTreeWidgetItem::UserType+10 || item->type() == QTreeWidgetItem::UserType+12)
    {
        if(item->type() == QTreeWidgetItem::UserType+10)
            dynamic_cast<FT_FitRunTreeItem*>(item)->cleanup();

        if(item->type() == QTreeWidgetItem::UserType+12)
            dynamic_cast<FT_SimRunTreeItem*>(item)->cleanup();

        treeWidget->invisibleRootItem()->removeChild(item);
        delete item;
    }
}

