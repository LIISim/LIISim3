#ifndef MULTISIGNALAVERAGE_H
#define MULTISIGNALAVERAGE_H

#include "../processingplugin.h"
#include <QList>
#include "../../signal.h"


/**
 * @brief The MultiSignalAverage class
 * @ingroup ProcessingPlugin-Implementations
 */
class MultiSignalAverage : public ProcessingPlugin
{
    Q_OBJECT

public:
    explicit MultiSignalAverage(ProcessingChain *parentChain);

    static  QString pluginName;

    static QList<Signal::SType> supportedSignalTypes;

    // implementations, overrides of virtual base class functions
    QString getName();
    bool processSignalImplementation(const Signal & in, Signal & out, int mpIdx);
    void setFromInputs();
    void reset();

    QString getParameterPreview();



private:

    unsigned int startSignal;
    unsigned int endSignal;

    int noMpoints;

    /// @brief flag indicating that the average signal has been calculated
    bool avgCalculated;

    /// @brief index of the measurement point where the average calculation
    int  avgSignalIdx;

    /// @brief avgSignals stores an average signal for each channel
    QList<Signal> avgSignals;

    void initializeAvgSignals();

    /** @brief max_datasize maximum number of datapoints (used for sizing tempsignals array) */
    int max_datasize;

    /**
     * @brief counters stores for each channel a vector containing the number
     * of signals which have been used for average calculation per
     * datapoint
     */
    QList<QVector<int>> counters;


signals:

public slots:

private slots:
    void onMRunChanged();

};

#endif // MULTISIGNALAVERAGE_H
