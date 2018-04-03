#include "mrungrouptreewidgetitem.h"

#include <QMessageBox>

#include "mruntreewidgetitem.h"

#include "../../../core.h"
#include "../../models/dataitem.h"
#include "../../models/datamodel.h"


// -----------------------
// TODO: DOCUMENTATION !!!
// -----------------------


MRunGroupTWI::MRunGroupTWI(DataItemTreeView::VisType vistype, MRunGroup* group, QTreeWidget *view)
    : DataItemObserverObject(group),
      QTreeWidgetItem(view)
{
    m_mute = true;
    this->vistype = vistype;
    setText(0,group->title());

    setToolTip(0,"measurement run group");
    setData(0,Qt::UserRole,1);
    setData(0,Qt::UserRole+1,group->id());

    setFlags(Qt::ItemIsEnabled |
             Qt::ItemIsEditable |
             Qt::ItemIsSelectable |
             Qt::ItemIsDragEnabled |
             Qt::ItemIsDropEnabled);

    m_group = group;
    m_mute = false;
    m_toggleShowAllRuns = false;
}


MRunGroupTWI::~MRunGroupTWI()
{
    m_checkedItems.clear();
    emit checkedItemsChanged();
}


/**
 * @brief MRunGroupTreeWidgetItem::onDataChanged
 * @param pos
 * @param value
 */
void MRunGroupTWI::onDataChanged(int pos, QVariant value)
{
    m_mute = true;

    if(pos == 0)
        setText(0,value.toString());

    m_mute = false;
}


/**
 * @brief MRunGroupTreeWidgetItem::onDataChildInserted
 * @param child_data
 */
void MRunGroupTWI::onDataChildInserted(DataItem *child_data, int position)
{
    if(child_data->inherits("MRun"))
    {
        MRun* mr = dynamic_cast<MRun*>(child_data);
        if(!mr)return;

        MRunTreeWidgetItem* item = new MRunTreeWidgetItem(vistype,mr);

        this->insertChild(position,item);
        item->setupItemWidgets();

        connect(item,SIGNAL(checkStateChanged(MRunTreeWidgetItem*,bool)),
                SLOT(onMrunTWICheckedStateChanged(MRunTreeWidgetItem*,bool)));
        connect(item, SIGNAL(visualStateChanged(MRunTreeWidgetItem*,bool)), SLOT(onVisualStateItemChanged(MRunTreeWidgetItem*,bool)));
        setExpanded(true);
    }
    //qDebug() << "DataItemObserverWidget::onDataChildInserted";
}


/**
 * @brief MRunGroupTreeWidgetItem::onDataChildRemoved
 * @param child_data
 */
void MRunGroupTWI::onDataChildRemoved(DataItem *child_data)
{
    if(child_data->inherits("MRun"))
    {
        for(int i = 0; i< this->childCount(); i++)
        {
            if(this->child(i)->data(0,Qt::UserRole+1) == child_data->id())
            {
                if(m_checkedItems.contains(this->child(i)))
                {
                    m_checkedItems.removeOne(this->child(i));
                    emit checkedItemsChanged();
                }
                delete this->takeChild(i);
                break;
            }
        }
    }
}


/**
 * @brief MRunGroupTWI::setCheckedItems
 * @param itemDIs
 */
void MRunGroupTWI::setCheckedItems(QList<int> &itemDIs)
{
    m_checkedItems.clear();
    for(int i = 0; i< this->childCount(); i++)
    {
        int itemID = this->child(i)->data(0,Qt::UserRole+1).toInt();
        if( itemDIs.contains(itemID) )
        {
            m_checkedItems.append(this->child(i));
            this->child(i)->setCheckState(0,Qt::Checked);
        }
        else
        {
            this->child(i)->setCheckState(0,Qt::Unchecked);
        }
    }
    emit checkedItemsChanged();
}


void MRunGroupTWI::deleteData()
{
    if(DataItemObserverObject::data->id() == Core::instance()->dataModel()->defaultGroup()->id() )
    {
        QString msg = "cannot delete default group!";
        MSG_NORMAL(msg);
        MSG_STATUS(msg);
        return;
    }

    if(Core::instance()->getSignalManager()->isBusy())
    {
        QString msg = "cannot delete group (active backgroud tasks!)";
        MSG_NORMAL(msg);
        MSG_STATUS(msg);
        return;
    }

    if(DataItemObserverObject::data->childCount() > 0)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Delete MRun-Group");
        msgBox.setText(
               QString("Group '%0' contains %1 Measurement runs.\nAre you sure that you want to delete this group?")
                    .arg(DataItemObserverObject::data->data(0).toString())
                    .arg(DataItemObserverObject::data->childCount()));

        msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if(msgBox.exec() == QMessageBox::Yes)
        {
            // do something
        }
        else
        {
            return;
        }
    }
    DataItemObserverObject::data->deleteLater();
}


/**
 * @brief MRunGroupTreeWidgetItem::onDataDestroyed
 */
void MRunGroupTWI::onDataDestroyed()
{
    m_group = 0;
    DataItemObserverObject::onDataDestroyed();
    this->deleteLater();
}


void MRunGroupTWI::handleTreeItemModified(int col)
{
    if(m_mute || !m_group)
        return;

    m_group->setTitle(this->text(0));
    //qDebug() << "MRunGroupTreeWidgetItem handleTreeItemChanged";
}


/**
 * @brief MRunGroupTWI::handleDoubleClick do what ever needs to be done when this item
 * is double cliced
 * @param col column of click
 */
void MRunGroupTWI::handleDoubleClick(int col)
{
    if(col > 0) // disable editing on other columns except the first!
        setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
    else
        setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
}


/**
 * @brief MRunGroupTWI::handleClick This slot is executed when
 * a single click has been performed on this item
 * (see DataItemTreeView::onTreeItemClicked)
 * @param col clicked column
 */
void MRunGroupTWI::handleClick(int col)
{
    m_toggleShowAllRuns = !m_toggleShowAllRuns;

    if(treeWidget() && (vistype == DataItemTreeView::ATOOL
                     || vistype == DataItemTreeView::EXP_DIAG))
        for(int i = 0; i < childCount();i++)
        {
            QTreeWidgetItem* item = child(i);
            if(item->data(0,Qt::UserRole).toInt() == 2)
            {
                MRunTreeWidgetItem* mi =  dynamic_cast<MRunTreeWidgetItem*>(item);
                mi->setShowMRun(m_toggleShowAllRuns);
            }
        }
}


/**
 * @brief MRunGroupTWI::onMrunTWICheckedStateChanged This slot is executed if the checked
 * state of a MRunTreeWidgetItem child has been changed
 * @param item modified item
 * @param state new checked state
 */
void MRunGroupTWI::onMrunTWICheckedStateChanged(MRunTreeWidgetItem *item, bool state)
{
    if(state)
    {
        if(!m_checkedItems.contains(item))
            m_checkedItems.append(item);
        emit checkedItemsChanged();
    }
    else
    {
        if(m_checkedItems.contains(item))
            m_checkedItems.removeOne(item);
        emit checkedItemsChanged();
    }
}


void MRunGroupTWI::onVisualStateItemChanged(MRunTreeWidgetItem* item, bool state)
{
    if(state)
    {
        if(!m_visualEnabledItems.contains(item))
            m_visualEnabledItems.append(item);
        emit visualEnabledItemsChanged();
    }
    else
    {
        if(m_visualEnabledItems.contains(item))
            m_visualEnabledItems.removeOne(item);
        emit visualEnabledItemsChanged();
    }
}


/**
 * @brief MRunGroupTWI::handleDrop
 * @param items
 * @param dropAbove drop items above this item (default false, does only affect group->group drops)
 */
void MRunGroupTWI::handleDrop(QList<QTreeWidgetItem *> items, bool dropAbove)
{
    for(int i = 0; i < items.size(); i++)
    {
        QTreeWidgetItem* item = items[i];
        int udat = item->data(0,Qt::UserRole).toInt();

        if(udat == 1) // another group has been dropped on this group
        {
            MRunGroupTWI* gi =  dynamic_cast<MRunGroupTWI*>(item);
            if(!gi)
                return;
            MRunGroup* g = gi->m_group;
            if(!g)
                return;

            DataItem* dataRoot = Core::instance()->dataModel()->rootItem();

            // todo copy checked mrun items of g (this information is lost!)
            dataRoot->removeChild(g);

            int p3 = m_group->position();

            if(dropAbove)
               dataRoot->insertChild(g,p3);
            else
               dataRoot->insertChild(g,p3+1);
        }
        else if(udat == 2) // a MRun has been dropped
        {
            MRunTreeWidgetItem* mi =  dynamic_cast<MRunTreeWidgetItem*>(item);
            if(!mi)
                return;
            MRun* mrun = Core::instance()->dataModel()->mrun(mi->data_id());
            if(!mrun)
                return;
            if(mrun->parentItem()->id() == m_group->id())
            {
                m_group->removeChild(mrun);
                m_group->insertChild(mrun,m_group->childCount());
            }
            else
                m_group->insertChild(mrun,m_group->childCount());
        }
    }
}
