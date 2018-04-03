#include "ft_datavisualization.h"

#include <QHBoxLayout>

#include "ft_datatable.h"
#include "ft_parametertable.h"
#include "../utils/minimizablewidget.h"

FT_DataVisualization::FT_DataVisualization(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    mDataTable = new FT_DataTable(this);
    mParameterTable = new FT_ParameterTable(this);

    MinimizableWidget *minWidget = new MinimizableWidget(this);
    minWidget->setTitle("Data Visualization");

    QWidget *tablesWidget = new QWidget(this);
    tablesWidget->setLayout(new QHBoxLayout);

    mPTScrollArea = new QScrollArea(this);
    mPTScrollArea->setWidgetResizable(true);
    mPTScrollArea->setWidget(mParameterTable);
    mPTScrollArea->setStyleSheet("QScrollArea { border : none; }");

    tablesWidget->layout()->setMargin(0);
    tablesWidget->layout()->addWidget(mDataTable);
    tablesWidget->layout()->addWidget(mPTScrollArea);

    minWidget->setWidget(tablesWidget);

    mainLayout->addWidget(minWidget);

    connect(mDataTable, SIGNAL(rowClicked(int)), SLOT(onDataTableRowClicked(int)));

    mDataTable->setVisible(false);
}


void FT_DataVisualization::update(FitData &fitData, int iteration)
{
    mPTScrollArea->setVisible(false);
    mDataTable->setVisible(true);

    mDataTable->update(fitData, iteration);
}


void FT_DataVisualization::update(FitRun *fitRun)
{
    mDataTable->setVisible(false);
    mPTScrollArea->setVisible(true);

    mParameterTable->update(fitRun);
}


void FT_DataVisualization::update(SimRun *simRun)
{
    mDataTable->setVisible(false);
    mPTScrollArea->setVisible(true);

    mParameterTable->update(simRun);
}


void FT_DataVisualization::clearDataTable()
{
    mDataTable->clear();
}


void FT_DataVisualization::clearParameterTable()
{
    mParameterTable->clear();
}


void FT_DataVisualization::onDataTableRowClicked(int row)
{
    emit dataTableRowClicked(row);
}
