#ifndef DATAITEMINTERFACE_H
#define DATAITEMINTERFACE_H

#include <QObject>

#include "dataitem.h"

/**
 * @brief The DataItemObserverObject class
 * @ingroup Hierachical-Data-Model
 */
class DataItemObserverObject : public QObject
{
    Q_OBJECT
public:
    explicit DataItemObserverObject(DataItem* data = 0,QObject *parent = 0);

    virtual ~DataItemObserverObject();

    inline int data_id(){return m_data_id;}

    inline bool validData(){if(data)return true;return false;}
    inline bool invalidData(){if(data)return false;return true;}

protected:

    DataItem* data;

private:


    int m_data_id;

signals:

public slots:

    virtual void setDataItem(DataItem* dataItem);

protected slots:


    virtual void onDataChanged(int pos, QVariant value);
    virtual void onDataChildInserted( DataItem* child_data, int position);
    virtual void onDataChildRemoved( DataItem* child_data);
    virtual void onDataDestroyed();

private slots:


};

#endif // DATAITEMINTERFACE_H
