#ifndef PROCESSINGPLUGINCONNECTOR_H
#define PROCESSINGPLUGINCONNECTOR_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QVariant>
#include "../signal.h"


class ProcessingChain;
class ProcessingPlugin;
class ProcessingPluginInputList;
class DataItem;
class MRun;

/**
 * @brief The ProcessingPluginConnector class observes a list of
 * ProcessingPlugins and keeps track of parameter and order changes.
 * This class is used to mirror the behavior of a ProcessingPlugin to other Plugin instance.
 * Therefore the observed Plugins must be of the same type.
 * @ingroup Signal-Processing
 */
class ProcessingPluginConnector : public QObject
{
    Q_OBJECT
public:
    explicit ProcessingPluginConnector(QString pluginName, int linkType, ProcessingChain* parentChain , bool doInit = true, QObject *parent = 0);
    explicit ProcessingPluginConnector(ProcessingPlugin* p);
    ~ProcessingPluginConnector();

    inline int pluginLinkType(){return m_linkState;}

    inline int id(){return m_linkID;}
    inline QString pluginName(){return m_pname;}

    Signal::SType m_stype;
    bool add(ProcessingPlugin* p);

    void disconnectMRun(MRun* mrun);
    void connectMRun(MRun* mrun, int groupChainIndex);
    void setActive(bool state);

private:

    QList<ProcessingPlugin*> m_plugins; ///< internal list of all observed plugin instances

    QString m_pname;    ///< name of managed ProcessingPluing
    int m_linkState;    ///< link state (A,G,S)

    int m_linkID;       ///< unique ProcessingPluginConnector ID
    static int m_linkID_count; ///< global ID counter

    bool m_dataMuteFlag;    ///< flag, used to block multiple slot executions
    bool m_pchainMuteFlag;  ///< flag, used to block multiple slot executions
    bool m_active;          ///< flag, used to enable/disable automatic instantiation of missing plugin instances

    ProcessingPlugin* findSenderPlugin(QObject* sender);
    QList<ProcessingPlugin*> clones(ProcessingPlugin* p);
    QList<ProcessingPlugin*> pluginsInSameGroup(ProcessingPlugin* p);
    void createMissingClones(ProcessingPlugin* p,bool copyParams = true, bool copyLinkState = true);
    void createClonesOnAllRuns(ProcessingPlugin* p,bool copyParams = true, bool copyLinkState = true);

    /**
     * @brief commonTCID If the connector object manages a temperature calcualtor,
     * this field holds the common temperature channel id of all managed temperature
     * calculators
     */
    int commonTCID;

public slots:

    // helpers, modify all plugins
    void setPluginsLinkState(int linkState);
    void setPluginsActive(bool active);
    void setPluginsVisible(bool visible);
    void setPluginsStepBufferFlag(bool sbFlag);
    void setPluginsParams(const ProcessingPluginInputList& params);

private slots:

    // handlers for ProcessingChain operations
    void onPlugInsertedToChain(DataItem* dataItem, int position);
    void onPlugRemovedFromChain(DataItem* dataItem);
    //handlers for ProcessingPlugins
    void onPlugDataChanged(int pos, QVariant data);
    void onPlugDestroyed(int id = -1);
};

#endif // PROCESSINGPLUGINCONNECTOR_H
