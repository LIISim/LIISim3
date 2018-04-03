#include "dataitemobserverobject.h"

// -----------------------
// TODO: DOCUMENTATION !!!
// -----------------------


DataItemObserverObject::DataItemObserverObject(DataItem *data, QObject *parent) :    QObject(parent)
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
 * @brief DataItemObserverObject::~DataItemObserverWidget
 */
DataItemObserverObject::~DataItemObserverObject()
{
    //qDebug() << "~DataItemObserverObject " << data_id();
}


void DataItemObserverObject::setDataItem(DataItem *dataItem)
{
    if(data){
       data->disconnect( this );
    }
    // connect oberserver slots
    connect(dataItem,SIGNAL(destroyed()),SLOT(onDataDestroyed()));
    connect(dataItem,SIGNAL(dataChanged(int,QVariant)),SLOT(onDataChanged(int,QVariant)));
    connect(dataItem, SIGNAL(childInserted(DataItem*, int)), SLOT(onDataChildInserted(DataItem*,int)));
    connect(dataItem, SIGNAL(childRemoved(DataItem*)),SLOT(onDataChildRemoved(DataItem*)));

    m_data_id = dataItem->id();
    data = dataItem;
}


/**
 * @brief DataItemObserverWidget::onDataChanged
 * @param pos
 * @param value
 */
void DataItemObserverObject::onDataChanged(int pos, QVariant value)
{

}


/**
 * @brief DataItemObserverObject::onDataChildInserted
 * @param child_data
 */
void DataItemObserverObject::onDataChildInserted(DataItem *child_data, int position)
{

}


/**
 * @brief DataItemObserverObject::onDataChildRemoved
 * @param child_data
 */
void DataItemObserverObject::onDataChildRemoved(DataItem *child_data)
{

}


/**
 * @brief DataItemObserverObject::onDataDestroyed
 */
void DataItemObserverObject::onDataDestroyed()
{
    //qDebug() << "DataItemObserverObject::onDataDestroyed " << data_id();
    data = 0;
}
