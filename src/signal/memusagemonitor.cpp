#include "memusagemonitor.h"

#include "../core.h"



// include windows api
#ifdef Q_OS_WIN

#include "Windows.h"
#include "psapi.h"

#include "processing/processingchain.h"
#include "processing/processingplugin.h"
#include "processing/plugins/simpledatareducer.h"
#include "processing/plugins/multisignalaverage.h"

#endif


MemUsageMonitor::MemUsageMonitor()
{
    msg_prefix = "memory usage ";

    // update memory info every 5 seconds
    updateInterval = 5000;

    timer.setInterval(updateInterval);
    connect(&timer,SIGNAL(timeout()),
            SLOT(updateMemInfo()));
    timer.start();
}

MemUsageMonitor::~MemUsageMonitor()
{
}


/**
 * @brief MemUsageMonitor::checkIfProcessingIsAllowed
 * check if signal processing is possible based on
 * the current memory consumption and an estimate of
 * the memory usage after the next processing task.
 * @return
 */
bool MemUsageMonitor::checkIfProcessingIsAllowed()
{
    updateMemInfo();

    int64_t est = runDataEstimate();
    int64_t next_est = estimateMemUsageForNextProcessingTask();
    int64_t d_est = next_est - est;

    int64_t av = mem - mem_used;

    bool res = true;

    // check if enough mem available
    double mem_delta = double(mem - mem_used) - double(d_est);
    if(mem_delta < 0)
        res = false;

    // check if liisim exceeds process mem limit
    mem_delta = double(mem_allowed) - double(next_est);

    // TODO: multiply correction factor (eg. 1.1) for
    // processing overhead during calculation !!!

    if(mem_delta <= 0 )
        res = false;

    // qDebug() << "phys mem av: " << toMB(av) << "delta next " << toMB(d_est) << " res " << res;

    return res;
}


/**
 * @brief MemUsageMonitor::updateMemInfo
 * read information about memory usage from operation system
 * @return true if operation system is supported, false if not
 */
bool MemUsageMonitor::updateMemInfo()
{
    m_is32bit = false;

    // ------------------------------
    // operation system specific code
    // ------------------------------

    // ------------------------------
    // Windows
    // ------------------------------
#ifdef Q_OS_WIN


    // total virtual memory

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    v_mem = memInfo.ullTotalPageFile;
    v_mem_allowed = v_mem;

    // virtual memory used
    v_mem_used = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;

    // virtual memory used by liisim

    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(),(PROCESS_MEMORY_COUNTERS*) &pmc, sizeof(pmc));
    v_mem_liisim = pmc.PrivateUsage;

    // total physical memory
    mem = memInfo.ullTotalPhys;
    mem_allowed = mem;

    // total physical memory used
    mem_used = memInfo.ullTotalPhys - memInfo.ullAvailPhys;

    // total physical memory used by liisim
    mem_liisim = pmc.WorkingSetSize;

    // now... for 32-bit windows reduce the amout of memory available to 2GB !!!
    #ifndef _WIN64

        mem_allowed = 2000000000;
        v_mem_allowed = 2000000000;
        m_is32bit = true;

    #endif


    QString info = QString("%0(physical): "
                           "liisim %1 MB, "
                           "all processes %2 MB, "
                           "max allowed (liisim) %3 MB")
            .arg(msg_prefix)
            .arg(toMB(mem_liisim))
            .arg(toMB(mem_used))
            .arg(toMB(mem_allowed));

    MSG_DETAIL_1(info);
    // MSG_INFO(info);

#endif

    // -------------------------------
    // all other operating systems !!!
    // -------------------------------

#ifndef Q_OS_WIN

    QString warnmsg = msg_prefix + " memory usage statistics not available for operation system!";
    MSG_WARN(warnmsg);

    v_mem = 0;
    v_mem_used = 0;
    v_mem_liisim = 0;

    mem = 0;
    mem_used = 0;
    mem_liisim = 0;


    emit infosUpdated();
    return false;
#endif

    emit infosUpdated();
    return true;
}


/**
 * @brief MemUsageMonitor::runDataEstimate estimate of the data [B]
 * used by mruns to store signal and processing step buffer data.
 * The estimate does not consider shallow copies of QVector.
 * This means that the estimate might be significantly larger than
 * the actual RAM usage!
 * @return memory usage estimate [B]
 */
uint64_t MemUsageMonitor::runDataEstimate()
{
    QList<MRun*> mruns = Core::instance()->dataModel()->mrunList();

    uint64_t runcount = mruns.size();
    uint64_t mpointcount = 0;
    uint64_t sigcount = 0;
    uint64_t noDataPoints = 0;

    uint64_t sigcount_raw = 0;
    uint64_t sigcount_abs = 0;
    uint64_t sigcount_temp = 0;

    uint64_t sb_s_r = 0;
    uint64_t sb_s_a = 0;
    uint64_t sb_s_t = 0;

    uint64_t sb_d_r = 0;
    uint64_t sb_d_a = 0;
    uint64_t sb_d_t = 0;

    Signal::SType stype = Signal::RAW;

    for(int st = 0; st < 3; st++)
    {
        if(st == 1)
            stype = Signal::ABS;
        else if(st==2)
            stype = Signal::TEMPERATURE;

        uint64_t buf_sigcount = 0;
        uint64_t buf_datacount = 0;


        for(int i = 0; i < mruns.size();i++)
        {
            int cur_scount = 0;

            if(st==0)
                mpointcount += mruns[i]->sizeAllMpoints();

            for(int j = 0; j < mruns[i]->sizeAllMpoints(); j++)
            {

                MPoint* mp = mruns[i]->getPre(j);
                QList<int> cids = mp->channelIDs(stype);
                for(int k = 0; k < cids.size(); k++)
                {
                    sigcount++;
                    cur_scount++;
                    noDataPoints += mp->getSignal(cids[k],stype).data.size();
                }

                mp = mruns[i]->getPost(j);
                cids = mp->channelIDs(stype);
                for(int k = 0; k < cids.size(); k++)
                {
                    sigcount++;
                    cur_scount++;
                    noDataPoints += mp->getSignal(cids[k],stype).data.size();
                }
            }


            if(st == 0)
                sigcount_raw += cur_scount;
            else if(st == 1)
                sigcount_abs += cur_scount;
            else
                sigcount_temp += cur_scount;

            // collect data of processing chain
            ProcessingChain* pchain =mruns[i]->getProcessingChain(stype);



            for(int p = 0; p < pchain->noPlugs();p++)
            {
                ProcessingPlugin* pp = pchain->getPlug(p);

                buf_datacount += pp->stepBufferDataPoints();
                buf_sigcount += pp->stepBufferSignals();
            }
        }

        if(st == 0)
        {
            sb_d_r = buf_datacount;
            sb_s_r = buf_sigcount;
        }
        else if(st == 1)
        {
            sb_d_a = buf_datacount;
            sb_s_a = buf_sigcount;
        }
        else
        {
            sb_d_t = buf_datacount;
            sb_s_t = buf_sigcount;
        }
    }


    uint64_t sigdata = double(noDataPoints*8);
    uint64_t stepdata = double(( sb_d_r + sb_d_a + sb_d_t )*8);

    QString info = QString("%0(run estimate): "
                           "total %1 MB, "
                           "[signal data %2 MB, "
                           "step buffer data %3 MB] %4 %5 %6 (shallow vector copies are not considered!!!)")
            .arg(msg_prefix)
            .arg(toMB(sigdata + stepdata))
            .arg(toMB(sigdata))
            .arg(toMB(stepdata))
            .arg(toMB(sb_d_r*8))
            .arg(toMB(sb_d_a*8))
            .arg(toMB(sb_d_t*8));

    MSG_DETAIL_1(info);
    // MSG_INFO(info);

    return (sigdata + stepdata);
}


/**
 * @brief MemUsageMonitor::estimateMemUsageForNextProcessingTask
 * estimate the amount of memory which would be used to store the results
 * of the next signal processing task. The estimate does not consider
 * shallow copies of QVector. This means that the estimate might be significantly
 * larger than the actual RAM usage!
 * The idea is to estimate the number and size of future step buffer signals.
 * @return estimated memory usage in bytes
 */
uint64_t MemUsageMonitor::estimateMemUsageForNextProcessingTask()
{
    QList<MRun*> mruns = Core::instance()->dataModel()->mrunList();

    // total number of datapoints in stepbuffers for raw processing chains
    uint64_t sb_d_r = 0;

    // total number of datapoints in stepbuffers for absolute processing chains
    uint64_t sb_d_a = 0;

    // total number of datapoints in stepbuffers for temperature processing chains
    uint64_t sb_d_t = 0;

    Signal::SType stype = Signal::RAW;

    // positim of first MSA plugin in absolute singal processing chain
    int pchain_abs_msa_pos = -1;

    // accumulated dtFactor of all SimpleDataReducer plugins in absolute processing chain
    double pchain_abs_reduce_fac = 1.0;

    // total sum of datapoints of unprocessed signals
    uint64_t total_upr_data = 0;

     // total sum of datapoints of postprocessed signals
    uint64_t total_ppr_data = 0;

    for(int i = 0; i < mruns.size();i++)
    {
        // sum of datapoints of unprocessed signals per run
        uint64_t run_upr_data = 0;

        // sum of datapoints of postprocessed signals per run
        uint64_t run_ppr_data = 0;

        // average number of datapoints in unprocessed absolute signals
        uint64_t abs_upr_avg_sigsize =0;

        for(int st = 0; st < 3; st++)
        {
            if(st == 0)
                stype = Signal::RAW;
            else if(st==1)
                stype = Signal::ABS;
            else if(st==2)
                stype = Signal::TEMPERATURE;

            QList<int> cids = mruns[i]->channelIDs(stype);

            // number of channels
            int noChannels = cids.size();

            // sum of datapoints of unprocessed signals per run per signal type
            uint64_t run_stype_upr_data = 0;

            // sum of datapoints of postprocessed signals per run per signal type
            uint64_t run_stype_ppr_data = 0;

            // number of unprocessed signals of run
            uint64_t run_stype_upr_signals = 0;

            // iterate over all measurement points of run
            // accumulate signal/datapoint informations
            for(int j = 0; j < mruns[i]->sizeAllMpoints(); j++)
            {
                // unprocessed signals
                MPoint* mp = mruns[i]->getPre(j);
                for(int k = 0; k < noChannels; k++)
                {
                    run_stype_upr_data += mp->getSignal(cids[k],stype).data.size();
                    run_stype_upr_signals += 1;
                }

                // postprocessed signals
                mp = mruns[i]->getPost(j);
                for(int k = 0; k < noChannels; k++)
                    run_stype_ppr_data += mp->getSignal(cids[k],stype).data.size();

            }

            // average number of datapoints per signal
            double run_upr_avg_sigsize = double(run_stype_upr_data) / double(run_stype_upr_signals);

            // save average size of umprocessed absolute signals
            if(stype == Signal::ABS)
                abs_upr_avg_sigsize = run_upr_avg_sigsize;

            // update run statistics
            run_upr_data += run_stype_upr_data;
            run_ppr_data += run_stype_ppr_data;


            // collect data of processing chain
            ProcessingChain* pchain =mruns[i]->getProcessingChain(stype);

            int nomp = mruns[i]->sizeAllMpoints();

            // some plugins reduce the amount of signals (MSA)
            // or the amount of datapoints per signal (SimpleDataReducer)
            // significantly. This should be considered during the estimation
            // of possible step buffer sizes.
            // TODO: integrate estimate correction for GetSection plugins!

            int msapos = pchain->indexOfPlugin(MultiSignalAverage::pluginName);
            double dat_reduce_factor = 1.0;

            // for temperature signals
            if(stype == Signal::TEMPERATURE)
            {


                // for temperature signals no unprocessed datapoints exist.
                // assume same average signal size as for absolute signals
                run_upr_avg_sigsize = abs_upr_avg_sigsize;

                // for temperature signals: consider msa-position and dtFacotrs
                // of absolute processing chain

                dat_reduce_factor = pchain_abs_reduce_fac;
                if(pchain_abs_msa_pos > -1)
                    msapos = 0;
            }

            uint64_t buf_total_datacount = 0;
            uint64_t buf_step_datacount = 0;
            for(int p = 0; p < pchain->noPlugs();p++)
            {
                ProcessingPlugin* pp = pchain->getPlug(p);

                // estimate the number of datapoints in stepbuffer
                // after nect calculation dependent on activation- and
                // stepBufferEnables- Flagstates and MSA-Position
                buf_step_datacount = 0;
                if(pp->activated())
                {
                    // update the data reduce factor if plugin is a data reducer
                    if(pp->getName() == SimpleDataReducer::pluginName)
                    {
                        dat_reduce_factor = dat_reduce_factor * pp->getInputs().getValue("dtFactor").toDouble();
                    }

                    if(pp->stepBufferEnabled())
                    {
                        // note: additional pessimistic estimate assumtion: msa replication
                        // does not generate implicitly shared data for temperature calculators
                        if(msapos > -1 && pp->position() >= msapos && stype != Signal::TEMPERATURE)
                           buf_step_datacount = (noChannels * run_upr_avg_sigsize / dat_reduce_factor);
                        else
                           buf_step_datacount = (nomp * noChannels * run_upr_avg_sigsize / dat_reduce_factor);

                        buf_total_datacount += buf_step_datacount;
                    }

                }

            } // end processing chain loop

            // correction of post processed temperature signal estimate
            if(stype == Signal::TEMPERATURE)
            {
                // if the post processed temperature signals of the mrun
                // have not been initialized yet (->empty signals!),
                // we correct the estimate of the number of post processed
                // data points by the estimated step buffer size.
                if(run_stype_ppr_data == 0)
                   run_ppr_data += buf_step_datacount;

            }

            if(stype == Signal::ABS)
            {
                pchain_abs_msa_pos = msapos;
                pchain_abs_reduce_fac = dat_reduce_factor;
            }

            if(st == 0)
                sb_d_r += buf_total_datacount;
            else if(st == 1)
                sb_d_a += buf_total_datacount;
            else
                sb_d_t += buf_total_datacount;

        } // end stype loop

        total_upr_data += run_upr_data;
        total_ppr_data += run_ppr_data;

    } // end  mrun loop

    uint64_t sigdata = (total_ppr_data + total_upr_data ) * 8;
    uint64_t stepdata = (sb_d_r + sb_d_a + sb_d_t )*8;

    QString info = QString("%0(next processing task estimate): "
                           "total %1 MB, "
                           "[signal data %2 MB, "
                           "step buffer data %3 MB] (shallow vector copies are not considered!!!)")
            .arg(msg_prefix)
            .arg(toMB(sigdata + stepdata))
            .arg(toMB(sigdata))
            .arg(toMB(stepdata));

    MSG_DETAIL_1(info);
    // MSG_INFO(info);

    return (sigdata + stepdata);
}


/**
 * @brief MemUsageMonitor::toKB converts B to KB
 * @param bytes unsigned integer
 * @return floating point representation in KB
 */
double MemUsageMonitor::toKB(uint64_t bytes)
{
    return double(bytes)/(1024.0);
}


/**
 * @brief MemUsageMonitor::toMB converts B to MB
 * @param bytes unsigned integer
 * @return floating point representation in MB
 */
double MemUsageMonitor::toMB(uint64_t bytes)
{
    return double(bytes)/(1024.0*1024.0);
}


/**
 * @brief MemUsageMonitor::toGB converts B to GB
 * @param bytes unsigned integer
 * @return floating point representation in GB
 */
double MemUsageMonitor::toGB(uint64_t bytes)
{
    return double(bytes)/(1024.0*1024.0*1024.0);
}


