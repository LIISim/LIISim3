#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QObject>
#include <QHash>
#include <QAction>

#include "dataitem.h"

class MRun;
class MRunGroup;
class FitRun;

/**
 * @brief The DataModel class holds a tree of DataItems for the
 * @ingroup Hierachical-Data-Model
 * programs main models (MRunGroup, MRun, ProcessingChain, ProcessingPlugin, etc.)
 * and provides functionality for data access.
 */
class DataModel : public QObject
{
    Q_OBJECT

    friend class DataItem;
public:
    explicit DataModel( QObject *parent = 0);
    ~DataModel();

    void registerGroup(MRunGroup* g);
    inline int groupCount(){return m_groups.size();}
    MRunGroup* group(int id);
    MRunGroup* defaultGroup();
    void setDefaultGroup(MRunGroup* g);

    inline DataItem* rootItem(){return m_dataRoot;}

    void registerMRun(MRun* m);
    inline int mrunCount(){return m_mruns.size();}
    MRun* mrun(int id);

    inline QList<MRun*> mrunList(){return m_mruns.values();}
    inline QList<MRunGroup*> groupList(){return m_groups.values();}
    QList<QPair<QString,int>> sortedGroupNameList();
    bool containsMRun(const QString & runname);

    QList<QAction*> moveToGroupActions();

    QList<QAction*> groupColorMapActions();

    inline QAction* actionClearAllRuns(){return m_actionClearAllRuns;}
    inline QAction* actionMoveToNewGroup(){return m_actionMoveToNewGroup;}

    MRunGroup* findGroup(const QString &gname);


    void registerFitRun(FitRun* fr);
    void unregisterFitrun(int id);

    inline QList<FitRun*> fitRuns(){return m_fitruns.values();}


private:

    /// @brief store hash map of mruns for faster data access
    QHash<int, MRun*>m_mruns;

    /// @brief hash map of mrun groups
    QHash<int, MRunGroup*> m_groups;

    /** @brief map of fitruns */
    QMap<int, FitRun*> m_fitruns;

    /// @brief map of 'move to group actions' (for context menus etc.)
    QMap<int, QAction*> m_moveToGroupActions;

    /// @brief list of colormap options (for context menus etc.)
    QList<QAction*> m_groupColorMapActions;

    /// @brief action for deleting all mruns
    QAction* m_actionClearAllRuns;

    /// @brief action for moving mruns to a new group
    QAction* m_actionMoveToNewGroup;

    /// @brief root of data tree (groups,mruns,processingchains etc...)
    DataItem* m_dataRoot;

    /// @brief id of a default group for MRuns
    int m_default_group_id;

signals:

    /**
     * @brief mrunAdded This signal is emitted if a mrun has been registered to the model
     * @param mrun
     */
    void mrunAdded(MRun* mrun);


    /**
     * @brief groupAdded This signal is emitted if a MRunGroup has been registered to the model
     * @param group
     */
    void groupAdded(MRunGroup* group);

    /**
     * @brief groupsChanged This signal is emitted when number of groups or group name is changed
     */
    void groupsChanged();

    void fitRunCountChanged();
    void newFitRunRegistered(FitRun *fitRun);

public slots:

    void moveToGroupActionTriggered(QAction* action);

    void groupColorMapActionTriggered(QAction* action);

private slots:

    void ungregisterDataItem(int id);

    void onActionCloseAllMRuns();
    void onActionMoveToNewGroup();

};

#endif // DATAMODEL_H
