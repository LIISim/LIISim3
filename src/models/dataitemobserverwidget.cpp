#include "dataitemobserverwidget.h"


// -----------------------
// TODO: DOCUMENTATION !!!
// -----------------------


DataItemObserverWidget::DataItemObserverWidget(DataItem *data, QWidget *parent) : QWidget(parent)
{
    this->data = data;


    // initialize connections to data
    if(data)
    {
        // connect oberserver slots
        connect(data,SIGNAL(destroyed()),SLOT(onDataDestroyed()));
        connect(data,SIGNAL(dataChanged(int,QVariant)),SLOT(onDataChanged(int,QVariant)));
        connect(data, SIGNAL(childInserted(DataItem*, int)), SLOT(onDataChildInserted(DataItem*, int)));
        connect(data, SIGNAL(childRemoved(DataItem*)),SLOT(onDataChildRemoved(DataItem*)));

        m_data_id = data->id();
    }
    else
    {
        m_data_id = -1;
    }
}


/**
 * @brief DataItemObserverWidget::~DataItemObserverWidget
 */
DataItemObserverWidget::~DataItemObserverWidget()
{
    //qDebug() << "~DataItemObserverWidget " << data_id();
}

void DataItemObserverWidget::setDataItem(DataItem *dataItem)
{
    if(data){
       data->disconnect( this );
    }
    // connect oberserver slots
    connect(dataItem,SIGNAL(destroyed()),SLOT(onDataDestroyed()));
    connect(dataItem,SIGNAL(dataChanged(int,QVariant)),SLOT(onDataChanged(int,QVariant)));
    connect(dataItem, SIGNAL(childInserted(DataItem*, int)), SLOT(onDataChildInserted(DataItem*, int)));
    connect(dataItem, SIGNAL(childRemoved(DataItem*)),SLOT(onDataChildRemoved(DataItem*)));

    m_data_id = dataItem->id();
    data = dataItem;
}


/**
 * @brief DataItemObserverWidget::onDataChanged
 * @param pos
 * @param value
 */
void DataItemObserverWidget::onDataChanged(int pos, QVariant value)
{

}


/**
 * @brief DataItemObserverWidget::onDataChildInserted
 * @param child_data
 */
void DataItemObserverWidget::onDataChildInserted(DataItem *child_data, int position)
{
    //qDebug() << "DataItemObserverWidget::onDataChildInserted";
}


/**
 * @brief DataItemObserverWidget::onDataChildRemoved
 * @param child_data
 */
void DataItemObserverWidget::onDataChildRemoved(DataItem *child_data)
{

}



/**
 * @brief DataItemObserverWidget::onDataDestroyed
 */
void DataItemObserverWidget::onDataDestroyed()
{
    //qDebug() << "DataItemObserverWidget::onDataDestroyed " << data_id();
    data = 0;
}
