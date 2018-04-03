#ifndef PROCESSINGPLUGIN_H
#define PROCESSINGPLUGIN_H

#include <QObject>

#include "../../models/dataitem.h"
#include "../signal.h"
#include "processingplugininputlist.h"
#include <QMutex>

class MRun;
class ProcessingChain;
class ProcessingPluginConnector;
class Core;
class PPStepBuffer;

/**
 * @brief Abstract Baseclass for ProcessingPlugins
 * @ingroup Hierachical-Data-Model
 * @ingroup Signal-Processing
 * @details This class is used to derive custom signal porcessing plugins.
 *
 * TODO: add callback slot for combobox signal (on index changed) .:. TODO .:.
 *
 * DataVector and dataChanged()-signal positions:
 * 0: name
 * 1: activation state
 * 2: dirty state
 * 3: parameters changed
 * 4: link state
 * 5: plot visibility
 * 6: step buffer flag
 * 7: input fields modified
 */
class ProcessingPlugin : public DataItem
{
    Q_OBJECT
friend class ProcessingPluginConnector;
public:

    explicit ProcessingPlugin(ProcessingChain* parentChain);
    virtual ~ProcessingPlugin();

    Signal::SType getStype();
    ProcessingPluginInputList getInputs();
    QString getShortDescription();

    void setParameters(const ProcessingPluginInputList& inputs);


    /** @brief flag indicating that this plugin should be excecuted
      * if all processing tasks have finished the calculation of the previous plugin */
    bool executeSyncronized;

    /** @brief position of plugin in processing chain */
    int positionInChain;


    /**
     * @brief getName abstract method which should return the plugin's name
     * @return ProcessingPlugin name
     * @details reimplement this method for all derived plugins!
     */
    virtual QString getName() = 0;

    /**
     * @brief ProcessingPlugin::processSignal The processing function, will call
     * 'processSignalImplementation' in the plugin.
     * @param in    input Signal
     * @param out   output Signal, with same properties as in signal except that the data vector is empty
     * @param mpIdx index of currently processed mpoint
     * @return true if the signal has passed validation
     */
    virtual bool processSignal(const Signal & in, Signal & out, int mpIdx);
    /**
     * @brief ProcessingPlugin::processSignalImplementation The actual processing function, needs to be
     * implemented by the plugin. Same properties as 'processSignal'.
     */
    virtual bool processSignalImplementation(const Signal & in, Signal & out, int mpIdx) = 0;
    virtual void reset();

    virtual void processMPoints(int mStart, int mEnd);

    /**
     * @brief getParameterPreview
     * @return a string containing information about a certain parameter(s)
     */
    virtual QString getParameterPreview() = 0;


    inline ProcessingChain* processingChain(){return m_pchain;}

    inline bool activated(){return m_activated;}
    void setActivated(bool activated);

    inline bool dirty(){return m_dirty;}
    void setDirty(bool dirty);

    void setStepBufferEnabled(bool state);
    bool stepBufferEnabled(){return stepBufferFlag;}

    bool busy();

    inline bool processingError() { return mProcessingError; }

    Signal processedSignal(int mPoint, int channel);
    Signal processedSignalPreviousStep(int mPoint, int channelID);

    bool validAt(int mPoint);
    bool validAtPreviousStep(int mPoint);

    void initializeCalculation();
    virtual void onAddedToPchain(){}


    inline int linkState(){return m_linkState;}
    void setLinkState(int t);
    int linkType();
    int linkID();

    inline ProcessingPluginConnector* ppc(){return m_ppc;}

    /// @brief returns true if this plugin should be visible in plot
    inline bool plotVisibility(){return m_plotVisibility;}

    void setPlotVisibility(bool value);

    int channelCount();

    void printDebugTree(int pos = 0,int level = 0)const;

    void cleanupStepBuffer();

    unsigned long stepBufferSignals();
    unsigned long stepBufferDataPoints();

protected:

    /**
     * @brief setFromInputs Derived classes should
     * reimplement this method to assign individual member parameters.
     */
    virtual void setFromInputs() = 0;

    /** @brief holds a short description about the plugins
     * (detailed plugindescription should be done in external file!)
     */
    QString shortDescription;
    QString pluginName;

    /** @brief list of inputs for plugin */
    ProcessingPluginInputList inputs;

    /** @brief mrun object */
    MRun* mrun;

    /** @brief signal type which has been applied to this plugin */
    Signal::SType stype;

    /** @brief this mutex should be locked for plugin state variables, which are modified during signal processing */
    QMutex variableMutex;

    /** @brief flag if output signal should contain standard deviation, should be set by plugin */
    bool preserveStdev;

    /** @brief true if error occurs during processing used in PPTreeWidgetItem for color*/
    bool mProcessingError;

private:

    ProcessingChain* m_pchain;

    /** @brief activated stores if the plugin is activated and should be processed */
    bool m_activated;

    /** @brief flag which is used by the processingchain/processing task to determine
        if the plugin needs to be recalculated (TODO: enable dirty-support in in chain/task)*/
    bool m_dirty;

    /** @brief use step buffer flag*/
    bool stepBufferFlag;

   // boost::multi_array<Signal,2> mPst;
    PPStepBuffer* stepBuffer;

    /** @brief p_validations signal is valid if at(x).value == number of channels    */
    QVector<int> p_validations;
    int p_calculatedMPts;


    /**
     * @brief m_linkState
     * 0: single
     * 1: group
     * 2: global
     */
    int m_linkState;
    int m_previousLinkState;


    /// @brief stores if this plugin's data shuld be plotted
    bool m_plotVisibility;

    ProcessingPluginConnector* m_ppc;

    /** @brief maps channel-id to step buffer index */
    QMap<int,int> m_chid_to_bufferidx;


signals:

    void procUpdateMessage(QList<QVariant> infos);

};

#endif // PROCESSINGPLUGIN_H
