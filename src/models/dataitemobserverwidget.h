#ifndef DATAITEMWIDGET_H
#define DATAITEMWIDGET_H

#include <QWidget>

#include "dataitem.h"


// -----------------------
// TODO: DOCUMENTATION !!!
// -----------------------


/**
 * @brief The DataItemObserverWidget class
 * @ingroup Hierachical-Data-Model
 * @ingroup GUI
 */
class DataItemObserverWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DataItemObserverWidget(DataItem* data =0,QWidget *parent = 0);

    virtual ~DataItemObserverWidget();

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

#endif // DATAITEMWIDGET_H
