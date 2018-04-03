#ifndef TEMPERATUREPROCESSINGCHAIN_H
#define TEMPERATUREPROCESSINGCHAIN_H

#include "processingchain.h"

/**
 * @brief The TemperatureProcessingChain class is a processing chain
 * specialized for the processing of temperature signals. The chain
 * divides the contained plugins into two groups:
 * - TemperatureCalculators: used to create new temperature channels based on data input signals
 * - other processing plugins: normal ProcessingPlugins which are applied after temperature signal generation
 */
class TemperatureProcessingChain : public ProcessingChain
{
    Q_OBJECT

public:
    explicit TemperatureProcessingChain(MRun* m_mrun);
    ~TemperatureProcessingChain();

    virtual void addPlug(ProcessingPlugin * p);
    virtual void removePlug(int idx, bool deletePlugin = true);
    virtual void insertPlug(ProcessingPlugin *p, int position);

    inline int temperatureCalculatorCont(){return tccount;}
private:

    /** @brief number of temperature calculators in processing chain */
    int tccount;

signals:

public slots:
};

#endif // TEMPERATUREPROCESSINGCHAIN_H
