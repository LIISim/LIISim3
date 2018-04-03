#include "processingtask.h"
#include "processingchain.h"

#include <QtConcurrent/qtconcurrentrun.h>
#include <QMutexLocker>
#include <QThread>
#include <QtConcurrent/qtconcurrentrun.h>

#include "plugins/multisignalaverage.h"

// initialize static id counter
unsigned long ProcessingTask::id_count = 0;


/**
 * @brief ProcessingTask::ProcessingTask Constructor
 * @param mrun  Pointer to MRun Object
 * @param start_signal_type first signal type which should be processed
 * @param parent Parent
 * @detail All ProcessingChains of a MRun are executed in the following order:
 * 1) Raw signals, 2) absolute signals and 3) temperature signals. By setting
 * the start_signal_type to other values than Signal::RAW, all previous ProcessingChains
 * are skipped and not calculated!
 */
ProcessingTask::ProcessingTask(MRun *mrun, Signal::SType start_signal_type, QObject *parent) :  QObject(parent)
{

    // assign id, increment global counter
    this->id = id_count++;

    // assign members
    this->mrun = mrun;
    this->noMpoints = mrun->sizeAllMpoints();
    this->startStype = start_signal_type;
    threadShouldStop = false;

    // change the MRun's busy state (to avoid further mrun access during calculation!)
    mrun->setBusy(true);
}


ProcessingTask::ProcessingTask(MRun* mrun, QList<Signal::SType> typeList, QObject *parent) : QObject(parent)
{
    // assign id, increment global counter
    this->id = id_count++;

    // assign members
    this->mrun = mrun;
    this->noMpoints = mrun->sizeAllMpoints();
    this->startStype = Signal::RAW;
    this->processTypes = typeList;
    threadShouldStop = false;

    // change the MRun's busy state (to avoid further mrun access during calculation!)
    mrun->setBusy(true);
}


/**
 * @brief ProcessingTask::resetIdCounter resets the global id counter.
 */
void ProcessingTask::resetIdCounter()
{
    id_count = 0;
}


/**
 * @brief ProcessingTask::run is the ProcessingTasks main method!
 * @override overides QRunnable::Run
 */
void ProcessingTask::run()
{
    if(processTypes.isEmpty())
    {
        // process all chains in order, starting at start signal point
        switch(startStype)
        {
            case Signal::RAW:
                processChain(Signal::RAW);
                processChain(Signal::ABS);
                processChain(Signal::TEMPERATURE);
                break;

            case Signal::ABS:
                processChain(Signal::ABS);
                processChain(Signal::TEMPERATURE);
                break;

            case Signal::TEMPERATURE:
                processChain(Signal::TEMPERATURE);
        }
    }
    else
    {
        if(processTypes.contains(Signal::RAW))
            processChain(Signal::RAW);

        if(processTypes.contains(Signal::ABS))
            processChain(Signal::ABS);

        if(processTypes.contains(Signal::TEMPERATURE))
            processChain(Signal::TEMPERATURE);
    }

    // we're done here. Set the mrun free and notify the world, that we are finished.
    mrun->setBusy(false);
    emit finished(mrun);
}




/**
 * @brief ProcessingTask2::processChain executes processing chain in a single thread.
 * @param stype signal type of processing chain
 */
void ProcessingTask::processChain(Signal::SType stype)
{
    if(!mrun)
        return;

    ProcessingChain* pchain = mrun->getProcessingChain(stype);

    if(!pchain)
        return;

    QList<int> chIDs = mrun->channelIDs(pchain->stype);

    pchain->initializeCalculation();
    int noPlugs = pchain->plugs.size();

    try
    {
        // if processing chain is empty // copy mruns pre to post
        if(noPlugs == 0)
        {            
            for(int i  = 0; i < noMpoints; i++)
            {
                for(int j = 0; j < chIDs.size(); j++)
                {
                    MPoint * mpre = mrun->getPre(i);
                    MPoint * mpost = mrun->getPost(i);
                    mpost->setSignal(mpre->getSignal(chIDs[j], pchain->stype), chIDs[j], pchain->stype);                    
                }
            }
        }
        else
        {
            for(int p = 0; p < noPlugs; p++)
            {
                //FIXME: invalidate MRun?
                if(threadShouldStop)
                {
                    mrun->calculationStatus()->setCancelled();
                    return;
                }

                ProcessingPlugin* plugin = pchain->getPlug(p);
                plugin->processMPoints(0, noMpoints - 1);
              //  if(!plugin->stepBufferEnabled())
              //      plugin->cleanupStepBuffer();
            }

            // msa: write pchain output to all post signals
            if(pchain->msaPosition() > -1 && noMpoints > 0)
            {
                for(int j = 0; j < chIDs.size(); j++)
                {

                    MPoint * mpost = mrun->getPost(0);
                    Signal msa = mpost->getSignal(chIDs[j], pchain->stype);
                   // Signal msa = pchain->getPlug(pchain->noPlugs()-1)->processedSignal(0,chIDs[j]);

                    for(int i  = 1; i < noMpoints; i++)
                    {
                            mpost = mrun->getPost(i);
                            mpost->setSignal(msa, chIDs[j], pchain->stype);
                    }
                }
            }

            // update validation for this MRun
            mrun->updateValidList();

            for(int p = 0; p < noPlugs; p++)
                pchain->plugs[p]->reset();
        }
    }
    catch(LIISimException e)
    {
        mrun->calculationStatus()->addError(e.what());        
        signal_msg("Processing of " + mrun->getName() + ": " + e.what(), e.type());
    //    qDebug() << "ERROR: ProcessingTask: "<<e.what();
    }

    return;
}


void ProcessingTask::stopTask()
{
    threadShouldStop = true;
}

