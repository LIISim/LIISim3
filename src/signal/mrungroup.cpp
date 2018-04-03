#include "mrungroup.h"
#include "mrun.h"

#include "../general/LIISimException.h"
#include "../core.h"
#include "processing/processingchain.h"
#include "processing/processingpluginconnector.h"
/**
 * @brief MRunGroup::MRunGroup Constructor
 */
MRunGroup::MRunGroup(const QString& title) : DataItem(0)
{
    pos_title = 0;

    setData(pos_title,title);
    g_title = title;

    m_colorMap = new ColorMap(this);
    m_colorMap->setToJetStyle();

    connect(m_colorMap,SIGNAL(modified()),
            SLOT(assignMRunColors()));

    connect(this,SIGNAL(childInserted(DataItem*, int)),SLOT(assignMRunColors()));

    if(Core::instance())
        Core::instance()->dataModel()->registerGroup(this);
}


MRunGroup::~MRunGroup()
{
    QString msg = "Group closed: " + m_data[0].toString();
    MSG_STATUS(msg);
    MSG_NORMAL(msg);
}


/**
 * @brief MRunGroup::setTitle
 * @param title
 */
void MRunGroup::setTitle(const QString &title)
{
    this->g_title = title;
    setData(pos_title,title);
}


QString MRunGroup::title() const
{
    return g_title;
}


bool MRunGroup::insertChild(DataItem *child, int position, bool initGlobalProcessors)
{
    QList<ProcessingPluginConnector*> initppcs;

    if(initGlobalProcessors)
        initppcs = findInitialGroupProcessors();

    bool res = DataItem::insertChild(child,position);


    if(initGlobalProcessors && res)
    {
        MRun* mrun = dynamic_cast<MRun*>(child);

        QList<ProcessingPluginConnector*> oldppcs = mrun->ppcs();
        for(int i = 0; i < oldppcs.size(); i++)
        {
            oldppcs[i]->disconnectMRun(mrun);
        }

        for(int i = 0; i < initppcs.size(); i++)
        {
            initppcs[i]->connectMRun(mrun,i);
        }
    }
    return res;
}


/**
 * @brief MRunGroup::at returns MRun at given index
 * @param i index of requested MRun within the group
 * @return MRun
 */
MRun *MRunGroup::at(int i) const
{
    DataItem* child = childAt(i);
    if(child && child->inherits("MRun"))
        return dynamic_cast<MRun*>(child);
    return 0;
}


/**
 * @brief MRunGroup::findMRun find MRun within group by name
 * @param runname
 * @return
 */
MRun* MRunGroup::findMRun(QString runname)
{
    for(int i = 0; i < childCount(); i++)
        if(childAt(i)->data(0).toString()==runname)
            return Core::instance()->dataModel()->mrun(childAt(i)->id());
    return 0;
}


/**
 * @brief MRunGroup::assignMRunColors Handle changes of colormap member.
 * Assign new colors to mruns
 */
void MRunGroup::assignMRunColors()
{
    int n = this->childCount();
    for(int i = 0; i < n; i++)
    {
        childAt(i)->setData(1, m_colorMap->color(i,0,n) );
    }
}



QList<ProcessingPluginConnector*> MRunGroup::findInitialGroupProcessors()
{
    QList<ProcessingPluginConnector*> ppcs;

    if(this->childCount() > 0 )
    {
        MRun* first = at(0);

        return first->ppcs();
    }

    QList<MRunGroup*> groups = Core::instance()->dataModel()->groupList();
    groups.removeOne(this);

    for(int i = 0; i <groups.size(); i++)
    {
        if(groups[i]->childCount() > 0)
        {
            MRun* first = groups[i]->at(0);
            return first->ppcs(false);
        }
    }
    return ppcs;
}


QList<MRun*> MRunGroup::mruns()
{
    QList<MRun*> res;
    for(int i = 0; i < childCount(); i++)
        res << Core::instance()->dataModel()->mrun(childAt(i)->id());
    return res;
}

