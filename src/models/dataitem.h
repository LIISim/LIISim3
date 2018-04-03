#ifndef DATAITEM_H
#define DATAITEM_H

#include <QObject>
#include <QDebug>
#include <QVariant>
#include <QVector>


/**
 * @brief The DataItem class represents a tree node of a DataItem tree.
 * @ingroup Hierachical-Data-Model
 * Each DataItem stores a vector of various types of data (QVariant).
 * If the data has been modified a dataChanged() signal is emitted.
 * A DataItem takes ownership of all children. This means that all children
 * will be destroyed if a DataItem is destroyed (via the QObject tree).
 * Further signals allow observers to handle certain events (child added/removed).
 * If a DataItem is destroyed QObject  emits a destroyed() signal automatically.
 */
class DataItem : public QObject
{
    Q_OBJECT

public:
    explicit DataItem(DataItem *m_parent = 0);

    /**
     * @brief id unique id of DataItem
     * @return id of this item
     */
    inline int id() const {return m_id;}

    ~DataItem();

    bool setData(int pos, QVariant value);
    QVariant data(int pos) const;
    int dataCount(){return m_data.size();}

    virtual bool insertChild(DataItem *child,int position = -1);
    virtual bool removeChild(DataItem* child);
    inline int childCount(){ return m_children.size();}
    DataItem* childAt(int index) const;
    DataItem* findChildById(int id, bool recursive = false);
    int position();

    DataItem* parentItem(){return m_parent;}

    virtual void printDebugTree(int pos = 0,int level = 0)const;

protected:

    /// @brief vector of data for item
    QVector<QVariant> m_data;

private:

    /// @brief list of child items
    QList<DataItem*> m_children;

    /// @brief pointer to parent item
    DataItem* m_parent;

    /// @brief global counter for id generation
    static int m_id_count;

    /// @brief unique item id
    int m_id;


signals:

    /**
     * @brief dataChanged This signal is emitted if content of the data vector has changed
     * @param pos position in data vector
     * @param value new value for data at position pos
     */
    void dataChanged(int pos = -1, QVariant value = 0);

    /**
     * @brief childInserted This signal is emitted if a DataItem has been inserted to the children list
     * @param child pointer to child DataItem
     */
    void childInserted( DataItem* child, int pos );


    /**
     * @brief childRemoved This signal is emitted if a DataItem has been removed from the children list
     * @param child pointer to child DataItem
     */
    void childRemoved( DataItem* child);


    /**
     * @brief destroyed This signal is emitted if a DataItem has been destroyed.
     * @param id id of data item
     */
    void destroyed(int id);

};

#endif // DATAITEM_H
