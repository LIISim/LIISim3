#ifndef PROCESSINGCHAIN_H
#define PROCESSINGCHAIN_H

#include "../../models/dataitem.h"
#include <QList>
#include <QMutex>
#include "../signal.h"
#include "processingtask.h"

class MRun;
class ProcessingTask;
class ProcessingPlugin;


/**
 * @brief The ProcessingChain class handels a list of Processing Plugins.
 * @ingroup Hierachical-Data-Model
 * @ingroup Signal-Processing
 * @details This class takes care of memory for intermediate step results and cotrols
 * the calculation flow of multiple ProcessingPlugins
 *
 * TODOs:
 * - enable inserting plugins at variable position
 */
class ProcessingChain : public DataItem
{
    Q_OBJECT

    friend class ProcessingTask; // allow Processing task access to private members!

    /** @brief  signal type the processing chain operates is applied to */
    Signal::SType stype;

    /** @brief measurement run the processing chain is applied to */
    MRun* m_mrun;

    int m_msaPosition;
    bool m_isMovinPlugin;

protected:

    /** @brief list of processing plugins */
    QList<ProcessingPlugin*> plugs;

public:

    /** @brief filename used to save filename if processing chain is loaded*/
    QString filename;

    explicit ProcessingChain(MRun * m_mrun, Signal::SType stype);
    virtual ~ProcessingChain();

    QString getMRunName();
    inline MRun* mrun(){return m_mrun;}

    Signal::SType getSignalType();
    int noPlugs();

    ProcessingPlugin* getPlug(int idx);
    ProcessingPlugin* getPluginByID(int id);

    // add/insert/remove/move/clear
    virtual void addPlug(ProcessingPlugin * p);
    virtual void insertPlug(ProcessingPlugin *p, int position);
    virtual void removePlug(int idx, bool deletePlugin = true);

    int indexOfPlugin(QString& pluginName, bool considerInactive = false);
    void movePlugin(int from, int to, bool insertBefore = false);
    void clearAll();

    // processing/calculation
    void initializeCalculation();

    Signal getStepSignalPre(int mpIdx, int chID, int stepIdx);

    bool isValid(int mpIdx);
    bool isValidAtStep(int mpIdx, int step);
    inline bool isMovingPlugin(){return m_isMovinPlugin;}

    bool contains(QString& pluginName);
    bool containsActivePlugin(QString& pluginName);

    bool containsActiveMSA();
    inline int msaPosition(){return m_msaPosition;}


    QList<ProcessingPluginConnector*> getPPCs(bool globalsOnly = false);

    // save/load functions for processing chain
    void save(QString fname);
    void load(QString fname);
    void copyFrom(ProcessingChain* pchain);

private slots:
    void onPluginDataChanged(int pos, QVariant value);

signals:
    void pluginGoneDirty();

};

#endif // PROCESSINGCHAIN_H
