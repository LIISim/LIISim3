#include "dataitem.h"
#include "../logging/msghandlerbase.h"

int DataItem::m_id_count = 0;


/**
 * @brief DataItem::DataItem Constructor
 * @param parent parent DataItem
 * @details If parent is a valid DataItem this item is inserted into the
 * parents children list
 */
DataItem::DataItem(DataItem *parent) :   QObject(parent)
{
    m_id = m_id_count++;

    m_parent = parent;


   // if(m_parent)
   //     m_parent->insertChild(this);
}


/**
 * @brief DataItem::~DataItem Destructor
 * @details Destroys this DataItem and all children recursivley.
 * If the DataItem has a valid parent item it will be removed from
 * the parents children list.
 */
DataItem::~DataItem(){
    //qDebug() << "~DataItem " << m_id << data(0).toString();

    for(int i = 0; i < m_children.size(); i++)
    {
        m_children[i]->m_parent = 0;
    }
    if(m_parent)
        m_parent->removeChild(this);

    emit destroyed(m_id);
}


/**
 * @brief DataItem::setData Assigns data value at given position.
 * @param pos position in data vector of item
 * @param value new value
 * @return true on success, false if position is invalid
 * @details This method sets the value at the given position in the
 * data vector. The data vector is appended if position equals the length of
 * the data vector. If position is negative or exceeds the data vector size
 * no changes to the data vector are made.
 */
bool DataItem::setData(int pos, QVariant value)
{
    if(pos < 0 || pos > m_data.size())
    {
        qDebug() << "DataItem::setData: invalid position "<<pos;
        return false;
    }
    if(pos == m_data.size())
    {
        m_data.append( value );
        // qDebug() << "DataItem::setData: appended data vector "<<pos;
        emit dataChanged(pos,value);
        return true;
    }
    m_data[pos] = value;
    emit dataChanged(pos,value);
    return true;
}


/**
 * @brief DataItem::data Get data from data vector
 * @param pos position in data vector
 * @return data value for valid positions or an empty QVariant for invalid positions
 */
QVariant DataItem::data(int pos) const
{
    if(pos < 0 || pos >= m_data.size())
        return QVariant();
    return m_data.at(pos);
}

bool DataItem::insertChild(DataItem *child, int position)
{
    if(!m_children.contains(child))
    {
        if(child->m_parent)
        {
            child->m_parent->removeChild(child);
        }
        if(position < 0)
        {

           // qDebug() << "DataItem.insertChild: invalid position " << position;
            position = m_children.size();

        }

        child->setParent(this);

        // assign child's parent to this item
        child->m_parent = this;

        m_children.insert(position,child);
        emit childInserted(child,position);
        return true;
    }
    return false;
}



int DataItem::position()
{
    if(m_parent)
        return m_parent->m_children.indexOf(this);
    return -1;
}

/**
 * @brief DataItem::removeChild Remove DataItem form children list
 * @param child
 * @return
 */
bool DataItem::removeChild(DataItem *child)
{

    bool res = m_children.removeOne(child);

    if(res)
    {
        child->m_parent = 0;
        // qDebug() << "   removedChild " << child->data(0).toString() << " from " << data(0).toString();
        emit childRemoved(child);
    }
    return res;
}



/**
 * @brief DataItem::childAt Get child DataItem
 * @param index index requested child
 * @return child item or 0 if index is invalid
 */
DataItem* DataItem::childAt(int index) const
{

    if(index >= 0 && index < m_children.size() )
        return m_children.at(index);
    else
        return 0;
}



/**
 * @brief DataItem::findChildById find child item in tree
 * @param id id of child item
 * @param recursive recursivley search all subtrees
 * @return child item or 0
 * @details If recursive is not set only first children of
 * this data item are tested.
 */
DataItem* DataItem::findChildById(int id, bool recursive)
{

    for(int i = 0; i < m_children.size();i++)
    {
        if(m_children[i]->id() == id)
        {
            return m_children[i];
        }
    }

    if(!recursive)
        return 0;


    DataItem* res = 0;

    for(int i = 0; i < m_children.size();i++)
    {

        res = m_children[i]->findChildById(id,true);
        if(res)
            return res;

    }
    return res;
}

/**
 * @brief DataItem::printDebugTree Iterates recursively through the data tree and
 * prints a string from the data vector to the debugging console
 * @param pos position of data in data vector which should be printed (default 0)
 * @param level recursion level (default 0, only importat for recursive calls)
 */
void DataItem::printDebugTree(int pos, int level) const
{
    QString offset = "";
    for(int i = 0; i < level; i++)
    {
        offset.append("   ");
    }

    QString str = QString(offset + data(pos).toString() +"(id %0)")
        .arg(m_id);



    MSG_DETAIL_1(str);
 //   qDebug() << level << offset + data(pos).toString() << "(id " << m_id << ")" <<data(0).toString();


    int nextlevel = level+1;

    for(int i = 0; i < m_children.size(); i++)
    {
        m_children.at(i)->printDebugTree(pos,nextlevel);
    }
}



