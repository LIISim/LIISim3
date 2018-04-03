
#include "dataitemtreeview.h"

#include <QDebug>
#include <QMenu>
#include <QAbstractItemModel>
#include <QMap>
#include <QMimeData>
#include <QPainter>
#include <QScrollBar>
#include <QMessageBox>

#include "../../../core.h"
#include "../../models/datamodel.h"
#include "../../models/dataitem.h"
#include "../../signal/mrungroup.h"
#include "../../signal/mrun.h"
#include "../../signal/processing/processingplugin.h"
#include "../../signal/processing/processingchain.h"
#include "../../signal/processing/pluginfactory.h"

#include "mrungrouptreewidgetitem.h"
#include "mruntreewidgetitem.h"

#include "processingplugintreewidgetitem.h"


// -----------------------
// TODO: DOCUMENTATION !!!
// -----------------------

/**
 * @brief DataItemTreeView::DataItemTreeView Constructor.
 * @param root DataItem which should be the root of the treeview
 * @param parent parent widget
 */
DataItemTreeView::DataItemTreeView(VisType visType, DataItem *root, QWidget *parent)
    : DataItemObserverWidget(root,parent)
{
    this->visType = visType;
    // inti gui
    layVbox = new QVBoxLayout;
    setLayout(layVbox);
    layVbox->setMargin(0);
    m_dummy = 0;
    m_treeWidget = new DataItemTreeWidget;
    m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    if(visType == SE_GROUPS)
    {
        m_treeWidget->setColumnCount(1);
        QStringList headerLabels;
        headerLabels << "Measurement Runs";
        setHeaderLabels(headerLabels);
    }

    if(visType == DA_VC_GROUPS)
    {
        m_treeWidget->setColumnCount(2);
        QStringList headerLabels;
        headerLabels << "Measurement Runs" << "";
        setHeaderLabels(headerLabels);
    }
    // treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    layVbox->addWidget(m_treeWidget);

    // init data
    if(root)
        setDataRoot(root,false);

    // init connections
    connect(m_treeWidget,SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this,SLOT(handleTreeItemChanged(QTreeWidgetItem*,int)));

    connect(m_treeWidget,SIGNAL(customContextMenuRequested(QPoint)),
            this,SLOT(onContextMenuRequested(QPoint)));

    connect(m_treeWidget,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(onTreeItemDoubleClicked(QTreeWidgetItem*,int)));

    connect(m_treeWidget,SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(onTreeItemClicked(QTreeWidgetItem*,int)));

    m_treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_treeWidget,SIGNAL(itemSelectionChanged()),this,SLOT(onSelectionChanged()));

    connect(m_treeWidget,
            SIGNAL(expanded(QModelIndex)),
            SLOT(onItemExpanded(QModelIndex)));

    // init actions
    acDelete = new QAction("Delete",this);
    acAdd = new QAction("Add",this);

    connect(acDelete,SIGNAL(triggered()),SLOT(onActionDelete()));
    connect(acAdd,SIGNAL(triggered()),SLOT(onAcAdd()));

    treeWidget()->setDragDropMode(QAbstractItemView::InternalMove);

    m_treeWidget->setDragEnabled(true);

    if(visType == SE_GROUPS || visType == EXP_DIAG)
        m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    resizeEvent(0);
    m_mute_checkedItemsChanged = false;

    //CAUTION!!!
    // if menu is teared off, from the teared off menu a new tear off menu causes program crashes

    menuDepSelection = new QMenu("Select MRuns with...", this);
    menuDepSelection->setTearOffEnabled(true);

    menuDepLIISettingsSelection = menuDepSelection->addMenu("LIISettings");
    menuDepLIISettingsSelection->setTearOffEnabled(true);

    menuDepFilterSelection = menuDepSelection->addMenu("Filter");
    menuDepFilterSelection->setTearOffEnabled(true);

    menuDepGainVoltageSelection = menuDepSelection->addMenu("Gain Voltage");
    menuDepGainVoltageSelection->setTearOffEnabled(true);

    menuDepGainChannel1Selection = menuDepGainVoltageSelection->addMenu("Channel 1");
    menuDepGainChannel2Selection = menuDepGainVoltageSelection->addMenu("Channel 2");
    menuDepGainChannel3Selection = menuDepGainVoltageSelection->addMenu("Channel 3");
    menuDepGainChannel4Selection = menuDepGainVoltageSelection->addMenu("Channel 4");

    menuDepGainChannel1Selection->setTearOffEnabled(true);
    menuDepGainChannel2Selection->setTearOffEnabled(true);
    menuDepGainChannel3Selection->setTearOffEnabled(true);
    menuDepGainChannel4Selection->setTearOffEnabled(true);

    multiSelectionMode = false;

    connect(menuDepLIISettingsSelection, SIGNAL(triggered(QAction*)), SLOT(onContextMenuDependantSelectorLIISettings(QAction*)));
    connect(menuDepFilterSelection, SIGNAL(triggered(QAction*)), SLOT(onContextMenuDependantSelectorFilter(QAction*)));
    connect(menuDepGainChannel1Selection, SIGNAL(triggered(QAction*)), SLOT(onContextMenuDependantSelectorGainVoltage(QAction*)));
    connect(menuDepGainChannel2Selection, SIGNAL(triggered(QAction*)), SLOT(onContextMenuDependantSelectorGainVoltage(QAction*)));
    connect(menuDepGainChannel3Selection, SIGNAL(triggered(QAction*)), SLOT(onContextMenuDependantSelectorGainVoltage(QAction*)));
    connect(menuDepGainChannel4Selection, SIGNAL(triggered(QAction*)), SLOT(onContextMenuDependantSelectorGainVoltage(QAction*)));
}


/**
 * @brief DataItemTreeView::setHeaderLabel set the header text for the first column
 * @param label
 */
void DataItemTreeView::setHeaderLabel(const QString &label)
{
    m_treeWidget->setHeaderLabel(label);
}


/**
 * @brief DataItemTreeView::setHeaderLabels set the header text for an
 * arbitrary number of columns
 * @param labels list of header labels
 */
void DataItemTreeView::setHeaderLabels(const QStringList& labels)
{
    m_treeWidget->setHeaderLabels(labels);
}


/**
 * @brief DataItemTreeView::setIndentation
 * @param i
 */
void DataItemTreeView::setIndentation(int i)
{
    m_treeWidget->setIndentation(i);
}


/**
 * @brief DataItemTreeView::setDataRoot set the root data of the tree.
 * @param root DataItem
 * @param cleanup clear all contents before setting the new root
 */
void DataItemTreeView::setDataRoot(DataItem *root,bool cleanup)
{
    if(cleanup)
        clearAll();

    if(!root)
        return;

    setDataItem(root);

    // insert child data
    for(int i = 0; i < root->childCount();i++)
    {
        DataItem* child = root->childAt(i);
        onDataChildInserted(child,i);

    }
}




/**
 * @brief DataItemTreeView::setColumnCount Sets the number of visible columns in the treeview
 * @param c number of columns
 */
void DataItemTreeView::setColumnCount(int c)
{
    m_treeWidget->setColumnCount(c);
}


/**
 * @brief DataItemTreeView::clearAll Deletes all content from treeview,
 * except of the permanent dummy item.
 */
void DataItemTreeView::clearAll()
{
    if(m_dummy)
    {
        bool state = m_dummy->isSelected();
        m_treeWidget->takeTopLevelItem(0);
        m_treeWidget->clear();
        m_treeWidget->insertTopLevelItem(0,m_dummy);
       // m_dummy->setSelected(state);
    }
    else
        m_treeWidget->clear();
}

void DataItemTreeView::clearDummy()
{
    if(m_dummy)
    {
        m_treeWidget->takeTopLevelItem(0);
        delete m_dummy;

    }
    m_dummy = 0;
}


/**
 * @brief DataItemTreeView::expandAll expandes all treeitems
 */
void DataItemTreeView::expandAll()
{
    m_treeWidget->expandAll();
}


/**
 * @brief DataItemTreeView::topLevelItem Returnts a Toplevel item of the tree.
 * A toplevel item shuld in most cases handle a 1st level-child of the dataroot.
 * @param index index of Toplevel item
 * @return QTreeWidgetItem
 */
QTreeWidgetItem* DataItemTreeView::topLevelItem(int index)
{
    if(index<0 || index >= m_treeWidget->topLevelItemCount())
        return 0;
    return m_treeWidget->topLevelItem(index);
}


/**
 * @brief DataItemTreeView::addTopLevelItem Adds an arbitrary toplevel item to this tree.
 * This does not affect the root DataItem data.
 * @param item any QTreeWidgetItem
 */
void DataItemTreeView::addTopLevelItem(QTreeWidgetItem *item)
{
    m_treeWidget->addTopLevelItem(item);
}


/**
 * @brief DataItemTreeView::indexOfTopLevelItem Returns the index of a toplevelitem
 * @param item
 * @return -1 if item is not found
 */
int DataItemTreeView::indexOfTopLevelItem(QTreeWidgetItem *item)
{
    return m_treeWidget->indexOfTopLevelItem(item);
}


/**
 * @brief DataItemTreeView::topLevelItemCount Returns number of 1st level items in treeview
 * @return
 */
int DataItemTreeView::topLevelItemCount()
{
    return m_treeWidget->topLevelItemCount();
}


/**
 * @brief DataItemTreeView::topLevelItemExpandedStates Returns a list of booleans storing
 * for each toplevel item if its expanded or not
 * @return
 */
QList<bool> DataItemTreeView::topLevelItemExpandedStates()
{
    QList<bool> expandedStates;
    for(int i = 0; i < m_treeWidget->topLevelItemCount();i++)
    {
        expandedStates << m_treeWidget->topLevelItem(i)->isExpanded();
    }
    return expandedStates;
}


/**
 * @brief DataItemTreeView::setTopLevelItemExpandedStates expand toplevelitems
 * from a list of boolean values.
 * @param states
 */
void DataItemTreeView::setTopLevelItemExpandedStates(QList<bool> states)
{
    for(int i = 0; i < m_treeWidget->topLevelItemCount();i++)
    {
        m_treeWidget->topLevelItem(i)->setExpanded(states.value(i,false));
    }
}


/**
 * @brief DataItemTreeView::setPermanentDummyItem Adds a permanent TreeWidgetItem
 * to the treewidget. It will not be deleted if the data root changes!
 * @param dummy
 */
void DataItemTreeView::setPermanentDummyItem(QTreeWidgetItem *dummy)
{
    m_dummy = dummy;
    m_treeWidget->insertTopLevelItem(0,dummy);
}


// ---------------------------------------
// IMPLEMENTATION THE DATAITEM WIDGET INTERFACE
// ---------------------------------------


/**
 * @brief DataItemTreeView::onDataChildInserted
 * @param child_data
 */
void DataItemTreeView::onDataChildInserted(DataItem *child_data, int position)
{
    // insert treewidgetitems based on the child_data's type
    if(child_data->inherits("MRunGroup"))
    {
        MRunGroup* g = dynamic_cast<MRunGroup*>(child_data);
        if(!g)return;
        MRunGroupTWI* gi = new MRunGroupTWI(this->visType , g);
        m_treeWidget->insertTopLevelItem(position,gi);
        // add groups
        for(int j = 0; j < child_data->childCount();j++)
            gi->onDataChildInserted(child_data->childAt(j),j);

        connect(gi,SIGNAL(checkedItemsChanged()),SLOT(onCheckedMRunsChanged()));
        connect(gi, SIGNAL(visualEnabledItemsChanged()), SLOT(onVisualStateChanged()));


    }else if(child_data->inherits("ProcessingPlugin"))
    {
        ProcessingPlugin* p = dynamic_cast<ProcessingPlugin*>(child_data);
        if(!p)return;
        PPTreeWidgetItem* pi = new PPTreeWidgetItem(p);

        if(m_dummy)
            position += 1;

        m_treeWidget->insertTopLevelItem(position,pi);
        pi->setupItemWidgets();




    }
}


/**
 * @brief DataItemTreeView::onDataChildRemoved
 * @param child_data
 */
void DataItemTreeView::onDataChildRemoved(DataItem *child_data)
{
    // do nothing here, the treewidget items shuld delete themselves
    if(child_data->inherits("MRunGroup")
       || child_data->inherits("ProcessingPlugin"))
    {
        for(int i = 0; i < m_treeWidget->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);

            if(item->data(0,Qt::UserRole+1).toInt()== child_data->id())
            {
                if(item->data(0,Qt::UserRole).toInt() == 1 )
                {
                    delete m_treeWidget->takeTopLevelItem(i);
                    i--;
                }
                else if(item->data(0,Qt::UserRole).toInt() == 4)
                {
                    delete m_treeWidget->takeTopLevelItem(i);
                    i--;
                }
            }
        }
    }
}









/**
 * @brief DataItemTreeView::onDataDestroyed
 */
void DataItemTreeView::onDataDestroyed()
{
    DataItemObserverWidget::onDataDestroyed();
}


// ---------------
// ACTION HANDLERS
// ---------------


/**
 * @brief DataItemTreeView::onAcAdd handles Context menu 'Add' actions.
 * The position of the clicked data item shuld be stored into the actions data.
 */
void DataItemTreeView::onAcAdd()
{
    // get position information and treeitem
    QPoint pos = acAdd->data().toPoint();
    QTreeWidgetItem* item = m_treeWidget->itemAt(pos);

    if(!item)
        return;

    // determine type of data item
    int userData = item->data(0,Qt::UserRole).toInt();

    // mrungroup
    if(userData == 1)
    {
        DataModel* dm = Core::instance()->dataModel();

        MRunGroup* g = new MRunGroup(QString("Group %0").arg(dm->groupCount()+1));
    }
}


/**
 * @brief DataItemTreeView::onActionDelete handles Context menu 'Delete/Remove' actions.
 * The position of the clicked data item shuld be stored into the actions data.
 */
void DataItemTreeView::onActionDelete()
{
    // get position information and treeitem
    QPoint pos = acDelete->data().toPoint();
    QTreeWidgetItem* item = m_treeWidget->itemAt(pos);
    if(!item)
        return;




    // determine type of data item
    int userData = item->data(0,Qt::UserRole).toInt();

    if(userData == 1)       // mrungroup
    {

        MRunGroupTWI* gi =  dynamic_cast<MRunGroupTWI*>(item);
        if(gi) gi->deleteData();

    }
    else if(userData == 2)  // mrun
    {
        QList<QTreeWidgetItem*> selection = m_treeWidget->selectedItems();
        if(!selection.contains(item))
            selection.append(item);

        for(int i = 0; i < selection.size(); i++)
        {
            QTreeWidgetItem* it = selection.at(i);
            MRunTreeWidgetItem* mi =  dynamic_cast<MRunTreeWidgetItem*>(it);
            if(mi)
            {
                mi->deleteData();
            }
        }


    }
    else if(userData == 4)  // processingplugin
    {
        PPTreeWidgetItem* pi = dynamic_cast<PPTreeWidgetItem*>(item);
        if(pi) pi->deleteData();
    }
}




// -----------------------
// HANDLE USER INTERACTION
// -----------------------


/**
 * @brief DataItemTreeView::onContextMenuRequested Create a custom context menu at the given point
 * @param point position of context menu
 */
void DataItemTreeView::onContextMenuRequested(const QPoint &point)
{
    this->grabKeyboard();
    if(invalidData())
    {
        // default case: 'load signal data'
        QMenu menu(this);
        menu.addAction(Core::instance()->getSignalManager()->actionDataImport());
        menu.exec( m_treeWidget->viewport()->mapToGlobal( point ));
        return;
    }

    // determine the clicked item
    QTreeWidgetItem* item = m_treeWidget->itemAt(point);


    // TREEROOT IS DATAROOT
    if(data == Core::instance()->dataModel()->rootItem())
    {
        QMenu menu(this);
        acAdd->setText("Add new group");
        menu.addAction(acAdd);
        if(item)
        {
            int userDat = item->data(0,Qt::UserRole).toInt();
            int id = item->data(0,Qt::UserRole+1).toInt();
            acDelete->setData(point);

            if(userDat == 1) // ITEM IS GROUP
            {
                selectionPoint = point;

                acDelete->setText("Remove group");
                menu.addAction(acDelete);

                // if more than 1 columns are visible -> add color options
                if(m_treeWidget->columnCount() > 1)
                {
                    QList<QAction*> actions = Core::instance()->dataModel()->groupColorMapActions();

                    // write clicked item id to action data
                    for(int i = 0; i< actions.size();i++)
                        actions.at(i)->setData(id);

                    menu.addSeparator();
                    QMenu* groupColorMenu = menu.addMenu("Set colormap");
                    groupColorMenu->addActions(actions);

                    connect(groupColorMenu,SIGNAL(triggered(QAction*)),
                            Core::instance()->dataModel(),SLOT(groupColorMapActionTriggered(QAction*)));
                }

                menu.addSeparator();

                //Add menu for changing LIISettings of group
                QMenu *menuSetGroupLIISettings = menu.addMenu("Set LIISettings of group");

                QList<DatabaseContent*> liiSettingsList =  *Core::instance()->getDatabaseManager()->getLIISettings();
                for(int j = 0; j < liiSettingsList.size(); j++)
                    menuSetGroupLIISettings->addAction(liiSettingsList[j]->name)->setData(liiSettingsList[j]->ident);

                connect(menuSetGroupLIISettings, SIGNAL(triggered(QAction*)), SLOT(onContextMenuSetGroupLIISettings(QAction*)));

                menu.addSeparator();

                if(multiSelectionMode)
                {
                    menu.addMenu(menuDepSelection);

                    menuDepLIISettingsSelection->clear();
                    menuDepFilterSelection->clear();
                    menuDepGainChannel1Selection->clear();
                    menuDepGainChannel2Selection->clear();
                    menuDepGainChannel3Selection->clear();
                    menuDepGainChannel4Selection->clear();

                    MRunGroup *mrgroup = Core::instance()->dataModel()->group(item->data(0, Qt::UserRole+1).toInt());
                    int groupid = item->data(0, Qt::UserRole+1).toInt();

                    if(!dependantSelectionMap.contains(groupid))
                        dependantSelectionMap.insert(groupid, SelectionContainer());

                    //add actions for liisettings selection
                    QList<int> liisettingsIDs;
                    for(int i = 0; i < mrgroup->mruns().size(); i++)
                    {
                        if(!liisettingsIDs.contains(mrgroup->mruns().at(i)->liiSettings().ident))
                            liisettingsIDs.append(mrgroup->mruns().at(i)->liiSettings().ident);
                    }

                    for(int i = 0; i < liisettingsIDs.size(); i++)
                    {
                        QAction *action = menuDepLIISettingsSelection->addAction(Core::instance()->getDatabaseManager()->liiSetting(liisettingsIDs.at(i))->name);
                        action->setData(liisettingsIDs.at(i));
                        action->setCheckable(true);
                        if(dependantSelectionMap[groupid].liisettings.contains(QVariant(liisettingsIDs.at(i))))
                            action->setChecked(true);
                    }
                    //add actions for filter selection
                    QList<QString> filterIDs;
                    for(int i = 0; i < mrgroup->mruns().size(); i++)
                    {
                        if(!filterIDs.contains(mrgroup->mruns().at(i)->filter().identifier))
                            filterIDs.append(mrgroup->mruns().at(i)->filter().identifier);
                    }

                    for(int i = 0; i < filterIDs.size(); i++)
                    {
                        QAction *action = menuDepFilterSelection->addAction(filterIDs.at(i));
                        action->setData(filterIDs.at(i));
                        action->setCheckable(true);
                        if(dependantSelectionMap[groupid].filter.contains(QVariant(filterIDs.at(i))))
                            action->setChecked(true);
                    }
                    //add actions for gain voltage selection
                    QList<double> channel1voltages;
                    QList<double> channel2voltages;
                    QList<double> channel3voltages;
                    QList<double> channel4voltages;
                    for(int i = 0; i < mrgroup->mruns().size(); i++)
                    {
                        if(mrgroup->mruns().at(i)->getNoChannels(Signal::ABS) > 0)
                            if(!channel1voltages.contains(mrgroup->mruns().at(i)->pmtGainVoltage(1)))
                                channel1voltages.append(mrgroup->mruns().at(i)->pmtGainVoltage(1));
                        if(mrgroup->mruns().at(i)->getNoChannels(Signal::ABS) > 1)
                            if(!channel2voltages.contains(mrgroup->mruns().at(i)->pmtGainVoltage(2)))
                                channel2voltages.append(mrgroup->mruns().at(i)->pmtGainVoltage(2));
                        if(mrgroup->mruns().at(i)->getNoChannels(Signal::ABS) > 2)
                            if(!channel3voltages.contains(mrgroup->mruns().at(i)->pmtGainVoltage(3)))
                                channel3voltages.append(mrgroup->mruns().at(i)->pmtGainVoltage(3));
                        if(mrgroup->mruns().at(i)->getNoChannels(Signal::ABS) > 3)
                            if(!channel4voltages.contains(mrgroup->mruns().at(i)->pmtGainVoltage(4)))
                                channel4voltages.append(mrgroup->mruns().at(i)->pmtGainVoltage(4));
                    }
                    if(channel1voltages.size() == 0)
                        menuDepGainChannel1Selection->setEnabled(false);
                    else
                    {
                        menuDepGainChannel1Selection->setEnabled(true);
                        for(int i = 0; i < channel1voltages.size(); i++)
                        {
                            QAction *action = menuDepGainChannel1Selection->addAction(QString::number(channel1voltages.at(i)));
                            action->setData(channel1voltages.at(i));
                            action->setCheckable(true);
                            if(dependantSelectionMap.value(groupid).gainChannel1.contains(channel1voltages.at(i)))
                                action->setChecked(true);
                        }
                    }
                    if(channel2voltages.size() == 0)
                        menuDepGainChannel2Selection->setEnabled(false);
                    else
                    {
                        menuDepGainChannel2Selection->setEnabled(true);
                        for(int i = 0; i < channel2voltages.size(); i++)
                        {
                            QAction *action = menuDepGainChannel2Selection->addAction(QString::number(channel2voltages.at(i)));
                            action->setData(channel2voltages.at(i));
                            action->setCheckable(true);
                            if(dependantSelectionMap.value(groupid).gainChannel2.contains(channel2voltages.at(i)))
                                action->setChecked(true);
                        }
                    }
                    if(channel3voltages.size() == 0)
                        menuDepGainChannel3Selection->setEnabled(false);
                    else
                    {
                        menuDepGainChannel3Selection->setEnabled(true);
                        for(int i = 0; i < channel3voltages.size(); i++)
                        {
                            QAction *action = menuDepGainChannel3Selection->addAction(QString::number(channel3voltages.at(i)));
                            action->setData(channel3voltages.at(i));
                            action->setCheckable(true);
                            if(dependantSelectionMap.value(groupid).gainChannel3.contains(channel3voltages.at(i)))
                                action->setChecked(true);
                        }
                    }
                    if(channel4voltages.size() == 0)
                        menuDepGainChannel4Selection->setEnabled(false);
                    else
                    {
                        menuDepGainChannel4Selection->setEnabled(true);
                        for(int i = 0; i < channel4voltages.size(); i++)
                        {
                            QAction *action = menuDepGainChannel4Selection->addAction(QString::number(channel4voltages.at(i)));
                            action->setData(channel4voltages.at(i));
                            action->setCheckable(true);
                            if(dependantSelectionMap.value(groupid).gainChannel4.contains(channel4voltages.at(i)))
                                action->setChecked(true);
                        }
                    }

                }

            }
            else if(userDat == 2) // ITEM IS MRUN
            {
                QList<QTreeWidgetItem*> selection = m_treeWidget->selectedItems();

                if(selection.size() > 1)
                    acDelete->setText("Remove selected MRuns");
                else
                    acDelete->setText("Remove MRun from list");

                // setup the "move to group" menu
                QMenu* moveToGroupMenu;

                if(selection.size() > 1)
                    moveToGroupMenu = menu.addMenu("Move selected MRuns to group");
                else
                    moveToGroupMenu = menu.addMenu("Move MRun to group");

                connect(moveToGroupMenu,SIGNAL(triggered(QAction*)),
                        Core::instance()->dataModel(),SLOT(moveToGroupActionTriggered(QAction*)));

                QList<QAction*> actions = Core::instance()->dataModel()->moveToGroupActions();

                // write clicked item id to action data
                QList<QVariant> selectedItemIds;



                if(selection.size() > 1)
                {
                    for(int i = 0; i < selection.size(); i++)
                        selectedItemIds << selection.at(i)->data(0,Qt::UserRole+1);
                }else
                    selectedItemIds << id;



                for(int i = 0; i< actions.size();i++)
                    actions.at(i)->setData(selectedItemIds);


                moveToGroupMenu->addActions(actions);

                // move to new group action
                QAction* moveToNewGroupAction = Core::instance()->dataModel()->actionMoveToNewGroup();
                moveToNewGroupAction->setData(selectedItemIds);
                menu.addAction(moveToNewGroupAction);

                menu.addSeparator();

                // LIISettings selection
                QMenu* liiSettingsMenu;
                if(selection.size() > 1)
                    liiSettingsMenu = menu.addMenu("Set LIISettings of selected MRuns");
                else
                    liiSettingsMenu = menu.addMenu("Set LIISettings of selected MRun");

                QList<DatabaseContent*> liiSettingsList =  *Core::instance()->getDatabaseManager()->getLIISettings();
                for(int j = 0; j < liiSettingsList.size(); j++)
                    liiSettingsMenu->addAction(liiSettingsList[j]->name);

                connect(liiSettingsMenu,SIGNAL(triggered(QAction*)), SLOT(onContextMenuSetLIISettings(QAction*)));

                menu.addSeparator();
                menu.addAction(acDelete);
            }
        }
        menu.addSeparator();
        //menu.addAction(Core::instance()->getSignalManager()->actionDataImport());
        menu.addAction(Core::instance()->dataModel()->actionClearAllRuns());

        menu.exec( m_treeWidget->viewport()->mapToGlobal( point ));

    }
    // TREEROOT IS PROCESSINGCHAIN
    else if( data->inherits("ProcessingChain"))
    {
        QMenu menu(this);

        QMenu addMenu("Add linked processing step (S) (individual parameters)",&menu);
        menu.addMenu(&addMenu);

        QMenu addMenuToGroup("Add linked processing step (G) (group parameters)",&menu);
        menu.addMenu(&addMenuToGroup);

        QMenu addMenuToAll("Add linked processing step (A) (global parameters)",&menu);
        menu.addMenu(&addMenuToAll);

        menu.addSeparator();

        QMenu addMenuNoLink("Add independent processing step",&menu);
        menu.addMenu(&addMenuNoLink);

        menu.addSeparator();

        ProcessingChain* pchain = qobject_cast<ProcessingChain*>(data);

        if(!pchain)
            return;

        QList<QAction*> addActionsNoLink = PluginFactory::instance()->pluginCreationActions(pchain,-1);
        addMenuNoLink.addActions(addActionsNoLink);

        QList<QAction*> addactions = PluginFactory::instance()->pluginCreationActions(pchain,0);
        addMenu.addActions(addactions);

        QList<QAction*> addactionsToGroup = PluginFactory::instance()->pluginCreationActions(pchain,1);
        addMenuToGroup.addActions(addactionsToGroup);

        QList<QAction*> addactionsToAll = PluginFactory::instance()->pluginCreationActions(pchain,2);
        addMenuToAll.addActions(addactionsToAll);

        acDelete->setData(point);
        acDelete->setText("Delete selected processing step");
        menu.addAction(acDelete);

        menu.addSeparator();

        if(item && pchain)
        {
            int id = item->data(0,Qt::UserRole+1).toInt();
            ProcessingPlugin* currentPlugin = pchain->getPluginByID(id);

            if(currentPlugin)
            {
              //  QMenu stateChangeMenu("Change processing state",&menu);

                QList<QAction*> mactions = PluginFactory::instance()->pluginStateChangeActions(currentPlugin);

              //  stateChangeMenu.addActions(PluginFactory::instance()->pluginStateChangeActions(currentPlugin));
              //  menu.addMenu(&stateChangeMenu);
                menu.addActions(mactions);
            }
        }

        menu.addSeparator();
        QAction* acSave = PluginFactory::instance()->actionSavePchain();
        QAction* acLoad = PluginFactory::instance()->actionLoadPchain();

        QList<QVariant> acInfo;
        acInfo << pchain->mrun()->id() << pchain->id();
        acSave->setData(acInfo);
        acLoad->setData(acInfo);

        menu.addAction(acSave);
        menu.addAction(acLoad);

        QMenu recentChains("Recent processing chains",&menu);
        recentChains.addActions(PluginFactory::instance()->recentChains());
        menu.addMenu(&recentChains);



        menu.exec( m_treeWidget->viewport()->mapToGlobal( point ));
    }

    this->releaseKeyboard();
}


/**
 * @brief DataItemTreeView::onSelectionChanged handles selection changes
 * of the treewidget
 */
void DataItemTreeView::onSelectionChanged()
{
    QList<QTreeWidgetItem*> selection = m_treeWidget->selectedItems();
    emit selectionChanged(selection);
}


/**
 * @brief DataItemTreeView::handleTreeItemChanged This slot is executed
 * if any Item in the tree has been modified by the user.
 * @param item modified item
 * @param col column which has been modified
 */
void DataItemTreeView::handleTreeItemChanged(QTreeWidgetItem *item, int col)
{
    if(mapCleanup)
    {
        dependantSelectionMap.clear();
    }

    // determine type of dataitem
    int userData = item->data(0,Qt::UserRole).toInt();

    // let the items decide themselves what to do....
    if(userData == 1) // mrungroup
    {
        MRunGroupTWI* gi =  dynamic_cast<MRunGroupTWI*>(item);
        if(gi) gi->handleTreeItemModified(col);
    }
    else if(userData == 2) // mrun
    {
        MRunTreeWidgetItem* mi =  dynamic_cast<MRunTreeWidgetItem*>(item);
        if(mi) mi->handleTreeItemModified(col);
    }
    else if(userData == 4)  // processingplugin
    {
        PPTreeWidgetItem* pi = dynamic_cast<PPTreeWidgetItem*>(item);
        if(pi) pi->handleTreeItemModified(col);
    }
    emit treeItemModified(item,col);
}


/**
 * @brief DataItemTreeView::keyPressEvent calls event handler and emits a signal (eg. for parent
 * widgets)
 * @param event
 */
void DataItemTreeView::keyPressEvent(QKeyEvent *event)
{
    handleKeyPress(event);
    emit keyPressed(event);

    DataItemObserverWidget::keyPressEvent(event);
}

void DataItemTreeView::keyReleaseEvent(QKeyEvent *event)
{
    emit keyReleased(event);
    DataItemObserverWidget::keyReleaseEvent(event);
}


/**
 * @brief DataItemTreeView::handleKeyPress handles any keypress for this widgets
 * @param event
 */
void DataItemTreeView::handleKeyPress(QKeyEvent *event)
{
    QList<QTreeWidgetItem*> selection = m_treeWidget->selectedItems();
    if(selection.isEmpty())
        return;

    if(event->key() == Qt::Key_Delete)
    {
        for(int i = 0; i < selection.size(); i++)
        {

            QTreeWidgetItem* item = selection[i];

            int userData = item->data(0,Qt::UserRole).toInt();

            if(userData == 1)
            {
                MRunGroupTWI* gi =  dynamic_cast<MRunGroupTWI*>(item);
                if(gi) gi->deleteData();
            }
            else if(userData == 2)
            {
                MRunTreeWidgetItem* mi =  dynamic_cast<MRunTreeWidgetItem*>(item);
                if(mi) mi->deleteData();
            }
            else if(userData == 4)
            {
                PPTreeWidgetItem* pi = dynamic_cast<PPTreeWidgetItem*>(item);
                if(pi) pi->deleteData();
            }
        }
    }
}


/**
 * @brief DataItemTreeView::onTreeItemDoubleClicked This slot is executed if a
 * double click has been performed on a treewidgetitem.
 * @param item clicked item
 * @param col clicked column
 */
void DataItemTreeView::onTreeItemDoubleClicked(QTreeWidgetItem *item, int col)
{
    // determine type of data
    int userData = item->data(0,Qt::UserRole).toInt();

    // let the treeitems decide what do with that doulbe click
    if(userData == 1) // MRUNGROUP
    {
        MRunGroupTWI* gi =  dynamic_cast<MRunGroupTWI*>(item);
        if(gi) gi->handleDoubleClick(col);
    }
    else if(userData == 2) // MRUN
    {
        MRunTreeWidgetItem* mi =  dynamic_cast<MRunTreeWidgetItem*>(item);
        if(mi) mi->handleDoubleClick(col);
    }
}


void DataItemTreeView::onTreeItemClicked(QTreeWidgetItem *item, int col)
{

    int userData = item->data(0,Qt::UserRole).toInt();

    if(userData == 2) // MRUN
    {
        MRunTreeWidgetItem* mi =  dynamic_cast<MRunTreeWidgetItem*>(item);
        if(mi) mi->handleClick(col);
    }
    else if(userData == 1) // MRUNGROUP
    {
        MRunGroupTWI* gi =  dynamic_cast<MRunGroupTWI*>(item);
        if(gi) gi->handleClick(col);
    }
    else if(userData == 4)
    {
        PPTreeWidgetItem* pi = dynamic_cast<PPTreeWidgetItem*>(item);
        if(pi) pi->handleClick(col);
    }
}


void DataItemTreeView::onCheckedMRunsChanged()
{
    QList<QTreeWidgetItem*> checkedItems;

    for(int i = 0; i < m_treeWidget->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
        int userData = item->data(0,Qt::UserRole).toInt();

        if(userData == 1)
        {
            MRunGroupTWI* gi =  dynamic_cast<MRunGroupTWI*>(item);
            checkedItems.append(gi->checkedItems());
        }
    }
    emit checkedItemsChanged(checkedItems);
}


/**
 * @brief DataItemTreeView::setCheckedRunIDs
 * @param itemIDs
 */
void DataItemTreeView::setCheckedRunIDs(QList<int> &itemIDs)
{
    if(visType != ATOOL &&  visType != EXP_DIAG)
        return;
    m_mute_checkedItemsChanged = true;
    for(int i = 0; i < m_treeWidget->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
        int userData = item->data(0,Qt::UserRole).toInt();

        if(userData == 1)
        {
            MRunGroupTWI* gi =  dynamic_cast<MRunGroupTWI*>(item);
            gi->setCheckedItems(itemIDs);
        }
    }

    m_mute_checkedItemsChanged = false;
    onCheckedMRunsChanged();
}


void DataItemTreeView::onVisualStateChanged()
{
    QList<QTreeWidgetItem*> visualEnabledItems;

    for(int i = 0; i < m_treeWidget->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
        int userData = item->data(0,Qt::UserRole).toInt();

        if(userData == 1)
        {
            MRunGroupTWI* gi =  dynamic_cast<MRunGroupTWI*>(item);
            visualEnabledItems.append(gi->visualEnabledItems());
        }
    }
    emit visualEnabledItemsChanged(visualEnabledItems);
}



/**
 * @brief DataItemTreeView::resizeEvent does some custom resizing
 * @param event resize event
 */
void DataItemTreeView::resizeEvent(QResizeEvent *event)
{

    // for the 3 column layout: adjust width of first (runname) column
    // (used in Analysistools + Processingstepviews)
    if(visType == ATOOL)
    {
        // resize first colum only, use fixed size for 2nd and 3rd columns
        int c1 = this->width();
        int c2 = m_treeWidget->columnWidth(1);
        int c3 = m_treeWidget->columnWidth(2);

        c1 -= c2;
        c1 -= 20; // fixed width for 3rd column

        // also consider the space of the vertical scrollbar (if visible)
        if(!this->isVisible() || m_treeWidget->verticalScrollBar()->isVisible())
            c1-=15;

        m_treeWidget->setColumnWidth(0,c1);
        m_treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    if(visType == SE_PSTEPS)
    {
        // resize first colum only, use fixed size for 2nd and 3rd columns
        int c1 = 40;
        int c3 = 40;
        int c4 = 20;
        int c2 = this->width()-c1-c3-c4;

        // also consider the space of the vertical scrollbar (if visible)
        if(m_treeWidget->verticalScrollBar()->isVisible())
            c2-=15;

        m_treeWidget->setColumnWidth(0,c1);
        m_treeWidget->setColumnWidth(1,c2);
        m_treeWidget->setColumnWidth(2,c3);
        m_treeWidget->setColumnWidth(3,c4);

        m_treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    if(visType == SE_GROUPS)
    {
        int c2 = 60;
        int c1 = this->width() - c2 -3;
        m_treeWidget->setColumnWidth(0,c1);
        m_treeWidget->setColumnWidth(1,c2);
    }
}


/**
 * @brief DataItemTreeView::onContextMenuSetLIISettings This slot is executed
 * when the user changed the LIISettings of one or multiple runs via the context menu
 * @param action
 */
void DataItemTreeView::onContextMenuSetLIISettings(QAction *action)
{
    // find LIISettings
    QList<DatabaseContent*> dbcs = *Core::instance()->getDatabaseManager()->getLIISettings();
    LIISettings* liiSettings = 0;
    for(int i = 0; i < dbcs.size(); i++)
        if(dbcs[i]->name == action->text())
        {
            liiSettings = dynamic_cast<LIISettings*>(dbcs[i]);
            break;
        }
    if(!liiSettings)
        return;

    // find runs, update LIISettings
    QList<QTreeWidgetItem*> selection = m_treeWidget->selectedItems();
    for(int i = 0; i < selection.size(); i++)
    {
        QTreeWidgetItem* item = selection[i];
        int runId = item->data(0,Qt::UserRole+1).toInt();

        MRun* mrun = Core::instance()->dataModel()->mrun(runId);
        if(!mrun)
            continue;
        mrun->setLiiSettings(*liiSettings);
    }
}


void DataItemTreeView::onContextMenuDependantSelectorLIISettings(QAction* action)
{
    QTreeWidgetItem* item = m_treeWidget->itemAt(selectionPoint);
    if(!item)
        return;

    if(dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].liisettings.contains(action->data()))
        dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].liisettings.removeAll(action->data());
    else
        dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].liisettings.append(action->data());

    mapToSelection(item);
}


void DataItemTreeView::onContextMenuDependantSelectorFilter(QAction *action)
{
    qDebug() << "DataItemTreeView::onContextMenuDependantSelectorFilter(QAction *action)";

    QTreeWidgetItem* item = m_treeWidget->itemAt(selectionPoint);
    if(!item)
        return;

    if(dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].filter.contains(action->data()))
        dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].filter.removeAll(action->data());
    else
        dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].filter.append(action->data());

    mapToSelection(item);
}


void DataItemTreeView::onContextMenuDependantSelectorGainVoltage(QAction *action)
{
    qDebug() << "DataItemTreeView::onContextMenuDependantSelectorGainVoltage";

    QTreeWidgetItem* item = m_treeWidget->itemAt(selectionPoint);
    if(!item)
        return;

    if(action->parent() == menuDepGainChannel1Selection)
    {
        if(dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel1.contains(action->data().toDouble()))
            dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel1.removeAll(action->data().toDouble());
        else
            dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel1.append(action->data().toDouble());
    }
    else if(action->parent() == menuDepGainChannel2Selection)
    {
        if(dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel2.contains(action->data().toDouble()))
            dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel2.removeAll(action->data().toDouble());
        else
            dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel2.append(action->data().toDouble());
    }
    else if(action->parent() == menuDepGainChannel3Selection)
    {
        if(dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel3.contains(action->data().toDouble()))
            dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel3.removeAll(action->data().toDouble());
        else
            dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel3.append(action->data().toDouble());
    }
    else if(action->parent() == menuDepGainChannel4Selection)
    {
        if(dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel4.contains(action->data().toDouble()))
            dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel4.removeAll(action->data().toDouble());
        else
            dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel4.append(action->data().toDouble());
    }

    mapToSelection(item);
}


void DataItemTreeView::mapToSelection(QTreeWidgetItem *item)
{
    if(!item)
        return;
    mapCleanup = false;

    for(int i = 0; i < item->childCount(); i++)
    {
        MRunTreeWidgetItem *twi = dynamic_cast<MRunTreeWidgetItem*>(item->child(i));
        MRun *run = Core::instance()->dataModel()->mrun(item->child(i)->data(0,Qt::UserRole+1).toInt());

        bool showItem = true;

        if(dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].liisettings.size() > 0)
            if(!dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].liisettings.contains(QVariant(run->liiSettings().ident)))
                showItem = false;

        if(dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].filter.size() > 0)
            if(!dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].filter.contains(QVariant(run->filter().identifier)))
                showItem = false;

        if(dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel1.size() > 0)
            if(!dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel1.contains(run->pmtGainVoltage(1)) || run->getNoChannels(Signal::RAW) < 1)
                showItem = false;

        if(dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel2.size() > 0)
            if(!dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel2.contains(run->pmtGainVoltage(1)) || run->getNoChannels(Signal::RAW) < 2)
                showItem = false;

        if(dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel3.size() > 0)
            if(!dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel3.contains(run->pmtGainVoltage(1)) || run->getNoChannels(Signal::RAW) < 3)
                showItem = false;

        if(dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel4.size() > 0)
            if(!dependantSelectionMap[item->data(0, Qt::UserRole+1).toInt()].gainChannel4.contains(run->pmtGainVoltage(1)) || run->getNoChannels(Signal::RAW) < 4)
                showItem = false;

        twi->setShowMRun(showItem);
    }

    mapCleanup = true;
}


void DataItemTreeView::onContextMenuSetGroupLIISettings(QAction *action)
{
    qDebug() << "DataItemTreeView::onContextMenuSetGroupLIISettings(QAction *action)";

    QTreeWidgetItem* item = m_treeWidget->itemAt(selectionPoint);
    if(!item)
        return;

    MRunGroup *group = Core::instance()->dataModel()->group(item->data(0,Qt::UserRole+1).toInt());

    LIISettings *liisettings = Core::instance()->getDatabaseManager()->liiSetting(action->data().toInt());
    if(!liisettings)
        return;

    QMessageBox msgBox;
    msgBox.setWindowTitle("Set LIISettings of Group");
    msgBox.setText(QString("Are you sure you want to set the LIISettings of all MRuns in the Group '%0' to '%1'?").arg(group->title()).arg(liisettings->name));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    if(msgBox.exec() == QMessageBox::Yes)
    {
        for(int i = 0; i < item->childCount(); i++)
        {
            //make sure it is an mrun
            if(item->child(i)->data(0,Qt::UserRole).toInt() == 2)
            {
                MRun *run = Core::instance()->dataModel()->mrun(item->child(i)->data(0,Qt::UserRole+1).toInt());
                if(run)
                    run->setLiiSettings(*liisettings);
            }
        }
    }
    else
        return;
}


void DataItemTreeView::setMultiSelectionMode(bool enabled)
{
    multiSelectionMode = enabled;
}
