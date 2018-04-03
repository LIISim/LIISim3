#include "datamodel.h"

#include <QDebug>
#include <QColorDialog>

#include "../signal/mrungroup.h"
#include "../signal/mrun.h"
#include "../calculations/fit/fitrun.h"

#include "../logging/msghandlerbase.h"

/**
 * @brief DataModel::DataModel Constructor
 * @param parent
 */
DataModel::DataModel(QObject *parent) :   QObject(parent)
{
    MSG_DETAIL_1("init DataModel");

    m_dataRoot = new DataItem();
    m_dataRoot->setData(0,"Root of All Evil");

    setDefaultGroup( new MRunGroup(QString("Ungrouped")));

    /* DataModel::groupColorMapActionTriggered() uses index to call colormap functions
     * 0 -> Single Color
     * 1 -> Jet Style
     * 2 -> Violet to Green
     * 3 -> Hot
     **/

    // init colormap actions
    m_groupColorMapActions << new QAction("Single Color",this);
    m_groupColorMapActions << new QAction("Jet Colormap", this);
    m_groupColorMapActions << new QAction("Violet to Green Colormap",this);
    m_groupColorMapActions << new QAction("Hot Colormap",this);
    m_groupColorMapActions << new QAction("Gray/Color Colormap", this);
    m_groupColorMapActions << new QAction("Cubehelix", this);

    m_actionClearAllRuns = new QAction("Close all MRuns",this);
    connect(m_actionClearAllRuns,SIGNAL(triggered()),
            SLOT(onActionCloseAllMRuns()));

    m_actionMoveToNewGroup = new QAction("Move selected MRuns to new group",this);
    connect(m_actionMoveToNewGroup,SIGNAL(triggered()),
            SLOT(onActionMoveToNewGroup()));
}


/**
 * @brief DataModel::~DataModel Destructor. Destroys DataItem hierachy.
 */
DataModel::~DataModel()
{
    m_groups.clear();
    m_mruns.clear();

    QList<FitRun*> fitruns = m_fitruns.values();
    int sz = fitruns.size();
    for(int i = 0; i < sz; i++)
    {
        unregisterFitrun(fitruns[i]->id());
        delete fitruns[i];
    }

    delete m_dataRoot;
}


/**
 * @brief DataModel::defaultGroup Returns a default MRunGroup
 * @return
 */
MRunGroup* DataModel::defaultGroup()
{
    if(m_groups.contains(m_default_group_id))
        return m_groups.value(m_default_group_id);
    return 0;
}

/**
 * @brief DataModel::setDefaultGroup
 * @param g
 */
void DataModel::setDefaultGroup(MRunGroup* g)
{
    registerGroup(g);
    m_default_group_id = g->id();
}



/**
 * @brief DataModel::group Get a MRunGroup by ID.
 * @param id
 * @return MRunGroup if ID is valid else 0
 */
MRunGroup* DataModel::group(int id)
{
    if(m_groups.contains(id))
        return m_groups.value(id);
    return 0;
}


/**
 * @brief DataModel::findGroup finds group by title.
 * Returns 0 if Group does not exist
 * @param gname
 * @return
 */
MRunGroup* DataModel::findGroup(const QString &gname)
{
    QList<MRunGroup*> gr = m_groups.values();
    for(int i=0; i < gr.size(); i++)
        if(gr[i]->title()==gname)
            return gr[i];
    return 0;
}


/**
 * @brief DataModel::groupListSorted returns sorted list of MRunGroups
 * @return
 */
QList<QPair<QString,int>> DataModel::sortedGroupNameList()
{

    QList<QPair<QString,int>> list;

    for(int i = 0; i < m_groups.values().size(); i++)
    {
        if(m_groups.values().at(i)->id() != m_default_group_id)
        {
            QPair<QString,int> pair;
            pair.first = m_groups.values().at(i)->title();
            pair.second = m_groups.values().at(i)->id();
            list.append(pair);
        }
    }

    std::sort(list.begin(),list.end());

    return list;
}


/**
 * @brief DataModel::mrun Get a MRun by ID.
 * @param id
 * @return MRun if ID is valid else 0
 */
MRun* DataModel::mrun(int id)
{
    if(m_mruns.contains(id))
        return m_mruns.value(id);
    return 0;
}


/**
 * @brief DataModel::containsMRun Checks if a MRun with given name has been registered to the model
 * @param runname name of MRun
 * @return true if MRun with runname exists else false
 */
bool DataModel::containsMRun(const QString &runname)
{
    QList<MRun*> runs = m_mruns.values();

    int sz = runs.size();
    for(int i=0; i< sz;i++)
        if(runs.at(i)->getName() == runname)
           return true;
    return false;
}


/**
 * @brief DataModel::registerMRun Register MRun to the models internal hashmap
 * @param m
 */
void DataModel::registerMRun(MRun *m)
{
    if(!m_mruns.contains(m->id()))
    {
        if(!m->parentItem())
            defaultGroup()->insertChild(m);
        m_mruns.insert(m->id(),m);
        // qDebug() << "DataModel group registered "<<m->id();
        connect(m,SIGNAL(destroyed(int)),SLOT(ungregisterDataItem(int)));
        emit mrunAdded(m);
    }
}


/**
 * @brief DataModel::registerGroup Register MRunGroup to the models internal hashmap
 * @param g
 */
void DataModel::registerGroup(MRunGroup *g)
{
    if(!m_groups.contains(g->id()))
    {
        m_groups.insert(g->id(),g);
        m_dataRoot->insertChild(g);

        QAction* groupAction = new QAction(g->title(),this);
        m_moveToGroupActions.insert(g->id(),groupAction);

        connect(g,SIGNAL(destroyed(int)),SLOT(ungregisterDataItem(int)));
        emit groupAdded(g);

        connect(g,SIGNAL(dataChanged(int,QVariant)),SIGNAL(groupsChanged()));
        emit groupsChanged();
    }
}


/**
 * @brief DataModel::registerFitRun registers FitRun to datamodel.
 * Emits FitRunCountChanged()
 * @param fr
 */
void DataModel::registerFitRun(FitRun *fr)
{
    m_fitruns.insert(fr->id(),fr);
    //qDebug() << "DataModel::registerFitRun: " << fr->id() << fr->name();
    emit newFitRunRegistered(fr);
    emit fitRunCountChanged();
}


/**
 * @brief DataModel::unregisterFitrun removes FitRun from datamodel
 * (no delete!). Emits FitRunCountChanged().
 * @param id FitRun id
 */
void DataModel::unregisterFitrun(int id)
{
    if(!m_fitruns.contains(id))
        return;
    //qDebug() << "DataModel::unregisterFitrun" << id;
    m_fitruns.remove(id);
    emit fitRunCountChanged();
}


/**
 * @brief DataModel::ungregisterDataItem Removes a registered MRun/MRunGroup from internal hashmaps
 * @param id
 */
void DataModel::ungregisterDataItem(int id)
{
    if(m_groups.contains(id))
    {
        m_groups.remove(id);
        m_moveToGroupActions.remove(id);
        // qDebug() << "DataModel: unregistered group" << id << groups.size() ;

        emit groupsChanged();
    }
    if(m_mruns.contains(id))
    {
        m_mruns.remove(id);
        // qDebug() << "DataModel: unregistered mrun" << id << mruns.size() ;
    }
}


/**
 * @brief DataModel::moveToGroupActions Returns a List of Actions. Each Action refers
 * to a certain group.
 * @return
 */
QList<QAction*> DataModel::moveToGroupActions()
{
    QList<int> keys = m_moveToGroupActions.keys();

    // update action texts
    for(int i = 0; i < keys.size(); i++)
    {
        int k = keys.at(i);
        MRunGroup* g = m_groups.value(k,0);
        m_moveToGroupActions.value(k)->setData(-1);
        if(g)
            m_moveToGroupActions.value(k)->setText(g->title());
    }
    return m_moveToGroupActions.values();
}


/**
 * @brief DataModel::groupColorMapActions Returns a list of actions with colormap options
 * for mrungroups
 * @return
 */
QList<QAction*> DataModel::groupColorMapActions()
{
    // update action texts
    for(int i = 0; i < m_groupColorMapActions.size(); i++)
    {
        m_groupColorMapActions[i]->setData(-1);
    }
    return m_groupColorMapActions;
}


/**
 * @brief DataModel::moveToGroupActionTriggered handle "move to group" action
 * trigger. the IDs of the mruns which shuld be moved must be
 * stored in a list in the actions data!
 * @param action QAction
 */
void DataModel::moveToGroupActionTriggered(QAction *action)
{
    int g_id = m_moveToGroupActions.key(action,-1);
    if(g_id < 0)
        return;

    QList<QVariant> m_ids = action->data().toList();

    for(int i = 0; i < m_ids.size(); i++)
    {
        int m_id = m_ids.at(i).toInt();

        MRun* mrun = m_mruns.value(m_id,0);

        if(!mrun)
            return;

        MRunGroup* group = m_groups.value(g_id,0);

        if(!group)
            return;

        group->insertChild(mrun);
    }
}


/**
 * @brief DataModel::onActionMoveToNewGroup This slot is executed if
 * the "move selected runs to new group" action is triggered.
 * The IDs of the mruns which shuld be moved must be
 * stored in a list in the actions data!
 */
void DataModel::onActionMoveToNewGroup()
{
    MRunGroup* g = new MRunGroup("New Group");
    this->registerGroup(g);

    QList<QVariant> m_ids = m_actionMoveToNewGroup->data().toList();

    for(int i = 0; i < m_ids.size(); i++)
    {
        int m_id = m_ids.at(i).toInt();

        MRun* mrun = m_mruns.value(m_id,0);

        if(!mrun)
            return;

        g->insertChild(mrun);
    }
}


/**
 * @brief DataModel::groupColorMapActionTriggered handle group color action trigger.
 * The ID of the group which should be modified must be stored in the actions data!
 * @param action
 */
void DataModel::groupColorMapActionTriggered(QAction *action)
{
    // get group id from action data
    int g_id = action->data().toInt();
    if(g_id < 0)
        return;

    MRunGroup* group = m_groups.value(g_id,0);

    if(!group)
        return;

    int actionIndex = m_groupColorMapActions.indexOf(action);
    QColor col;

    // ColorMapAction list is initialized in Constructor DataModel::DataModel()
    switch(actionIndex)
    {
    case 0:
        col = QColorDialog::getColor(Qt::red,0,"Select Group Color");
        if(col.isValid())
            group->colorMap()->setToSingleColor(col);
        break;
    case 1:
        group->colorMap()->setToJetStyle();
        break;
    case 2:
        group->colorMap()->setToGradient_VioletGreen();
        break;
    case 3:
        group->colorMap()->setToHotStyle();
        break;
    case 4:
        group->colorMap()->setToGrayStyle();
        break;
    case 5:
        group->colorMap()->setToCubehelixColorMap();
        break;
    }
}


/**
 * @brief DataModel::onActionCloseAllMRuns This slot is executed if
 * the "clear all mruns" action is triggered (for example per context
 * menu in signalprocessngeditor).
 */
void DataModel::onActionCloseAllMRuns()
{
    QList<MRun*> runs = m_mruns.values();
    for(int i = 0; i < runs.size(); i++)
    {
        delete runs.at(i);
    }
}
