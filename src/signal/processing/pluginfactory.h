#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H


#include <QStringList>
#include <QObject>
#include <QAction>
#include <QList>
#include <QMap>
#include <QVector>
#include <QActionGroup>

#include "../signal.h"

class ProcessingPlugin;
class Core;
class MRun;
class MRunGroup;
class ProcessingChain;
class ProcessingPluginConnector;
class DataItem;

/**
 * @brief The PluginFactory class is responsible for creating instances of ProcessingPlugins.
 * @ingroup Signal-Processing
 * @details It provides informations about all available plugins for the GUI and is able to
 * create plugin instances by the plugin's name
 */
class PluginFactory : public QObject
{
    Q_OBJECT
public:

    static PluginFactory* instance()
    {
        if(!m_instance)
            m_instance = new PluginFactory;
        return m_instance;
    }


    static QStringList getAvailablePluginNames(Signal::SType stype);

    static ProcessingPlugin* createInstance(QString pluginName, ProcessingChain *parentChain);

    QList<QAction*> pluginCreationActions(ProcessingChain* pchain, int pluginLinkType);

    QList<QAction*> pluginStateChangeActions(ProcessingPlugin* currentContextPlugin);

    QAction* actionSavePchain(){return m_action_save_pchain;}
    QAction* actionLoadPchain(){return m_action_load_pchain;}
    QList<QAction*> recentChains();

    void handleApplicationStartup();

private:

    PluginFactory();

    static PluginFactory* m_instance;

    QList<QAction*> m_actions_addPlug_noLink;
    QList<QAction*> m_actions_addPlug_S;
    QList<QAction*> m_actions_addPlug_G;
    QList<QAction*> m_actions_addPlug_A;
    QList<QAction*> m_recentPchains;
    ProcessingChain* m_pchain;

    QStringList m_tempPlugs;
    QStringList m_absPlugs;
    QStringList m_rawPlugs;

    QAction* m_action_load_pchain;
    QAction* m_action_save_pchain;

    QActionGroup* m_stateChangeActions;
    QAction* m_stateAction_all;
    QAction* m_stateAction_group;
    QAction* m_stateAction_single;
    QAction* m_stateAction_noLink;
    ProcessingPlugin* m_currentContextPlugin;

    void addRecentChain(QString fname);

private slots:

    void addActionTriggered();
    void stateChangeActionTriggered(bool);

    void onPchainDestroyed();
    void onCurrentContextPluginDestroyed();

    void loadChain(QString fname = "");
    void saveChain();
    void onRecentChainTriggerd();

};

#endif // PLUGINMANAGER_H
