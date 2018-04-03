#include "temperatureprocessingchain.h"

#include "processingplugin.h"
#include "plugins/temperaturecalculator.h"
#include "plugins/dummyplugin.h"


/**
 * @brief TemperatureProcessingChain::TemperatureProcessingChain constructor
 * @param m_mrun parent measurement run
 */
TemperatureProcessingChain::TemperatureProcessingChain(MRun *m_mrun)
    : ProcessingChain(m_mrun, Signal::TEMPERATURE)
    , tccount(0)
{
    // add a dummy plugin as separator of temperature calculators and
    // processing steps
    addPlug(new DummyPlugin(this,"Temperature Processing steps:"));
}


/**
 * @brief TemperatureProcessingChain::~TemperatureProcessingChain destructor
 */
TemperatureProcessingChain::~TemperatureProcessingChain()
{
}


/**
 * @brief TemperatureProcessingChain::addPlug overwrites ProcessingChain::addPlug(..),
 * ensures that TemperatureCalculators are added to the beginning of the ProcessingChain
 * @param p ProcessingPlugin
 */
void TemperatureProcessingChain::addPlug(ProcessingPlugin *p)
{
    if(p->getName() == TemperatureCalculator::pluginName)
    {
        insertPlug(p,tccount);
        return;
    }

    ProcessingChain::addPlug(p);
}


/**
 * @brief TemperatureProcessingChain::removePlug overwrites ProcessingChain::removePlug(..)
 * @param idx
 * @param deletePlugin
 */
void TemperatureProcessingChain::removePlug(int idx, bool deletePlugin)
{
    if(idx >= 0 &&
       idx < plugs.size() &&
       plugs[idx]->getName() == TemperatureCalculator::pluginName)
    {
        // decrease TemperatureCalculator counter
        tccount--;
    }
    ProcessingChain::removePlug(idx, deletePlugin);
}


/**
 * @brief TemperatureProcessingChain::insertPlug overwrites ProcessingChain::insertPlug(..)
 * ensures that TemperatureCalculators are added to the beginning of the ProcessingChain
 * @param p
 * @param position
 */
void TemperatureProcessingChain::insertPlug(ProcessingPlugin *p, int position)
{
    if(p->getName() == TemperatureCalculator::pluginName)
    {
        if(position > tccount)
            position = tccount;

        // increase TemperatureCalculator counter
        tccount++;
    }
    // if plugin is no TemperatureCalculator, correct index if necessary
    else if(position <= tccount)
    {
        position = tccount+1;
    }
    ProcessingChain::insertPlug(p,position);
}
