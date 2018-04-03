#include "iobase.h"

#include <QtConcurrent/qtconcurrentrun.h>
#include <QApplication>
#include "../core.h"
#include "../signal/mrungroup.h"
#include "../settings/mrunsettings.h"
#include "consistencycheck.h"

bool IOBase::abort_flag = false;

IOBase::IOBase(QObject *parent)  :  QObject(parent)
{
    p_mode = PM_PERMRUN;
    m_finished_tasks = 0;
    m_task_count = 0;
    m_numberOfSignals = 0;
    m_autoDelete = false;
    m_concurrency = true;

    // connect the importSetupFinished signal
    connect(this,SIGNAL(importSetupFinished()),SLOT(onImportSetupFinished()));
    connect(this, SIGNAL(importStepFinished(int)),SLOT(onImportStepFinished(int)));

    // connect log
    connect(this, SIGNAL(sig_logMessage(QString,LIISimMessageType)),
            SLOT(handleQueuedMessage(QString,LIISimMessageType)),
            Qt::QueuedConnection);

    connect(this,
            SIGNAL(exportImplementationFinished(double)),
            SLOT(onExportImplementationFinished(double)),
            Qt::QueuedConnection);

    m_kill_ok1 = false;
    m_kill_ok2 = false;
    m_muteLog = false;
    m_mrun = 0;
}


/**
 * @brief IOBase::logMessage emit message signal if log-muting is disabled.
 * (used for async logging )
 * @param msg message
 * @param msg_type message type
 */
void IOBase::logMessage(const QString &msg, LIISimMessageType msg_type)
{
    if(!m_muteLog || (msg_type != NORMAL))
        emit sig_logMessage(msg,msg_type);
}


/**
 * @brief IOBase::handleQueuedMessage
 * @param msg
 * @param msg_type
 */
void IOBase::handleQueuedMessage(const QString &msg, LIISimMessageType msg_type)
{
    MSG_ASYNC(msg,msg_type);
}


/**
 * @brief IOBase::importSignals Starts import processes (usually
 * called by SignalManager::importSignals()). Initiates a concurrent
 * call to abstract method IOBase::setupImport()
 * @param irq
 * @details
 */
void IOBase::importSignals(const SignalIORequest & irq)
{
    m_initialRequest = irq;
    abort_flag = false;
    try
    {
        if(m_mrun != nullptr && irq.itype == CUSTOM)
        {
            ConsistencyCheck cc;

            if(cc.check(irq))
            {
                MSG_ASYNC(QString("Consistency Check returned errors for %0, import might lead to unexpected results.").arg(irq.runname), LIISimMessageType::WARNING);
                cc.prepareRun(m_mrun);
            }
        }

        // this call executes the setupImport method for the certain data format
        // (implementation can be found in subclasses)
        m_timer.start();
        if(m_concurrency)
            QtConcurrent::run(this,&IOBase::setupImport);
        else
            setupImport();
    }
    catch(LIISimException e)
    {
        logMessage(e.what(),e.type());

        // skip import-setup by emiting finished signal explicitly
        emit importSetupFinished();
    }
}


void IOBase::importSignals(const QList<SignalIORequest> &irq)
{
    if(!irq.isEmpty())
    {
        m_generatedRequests = irq;
        m_initialRequest = m_generatedRequests.takeLast();
        abort_flag = false;
        try
        {
            m_timer.start();
            if(m_concurrency)
                QtConcurrent::run(this, &IOBase::setupImport);
            else
                setupImport();
        }
        catch(LIISimException e)
        {
            logMessage(e.what(), e.type());

            emit importSetupFinished();
        }
    }
}


/**
 * @brief IOBase::exportSignals export signal data based
 * on given SignalIORequest
 * @param irq SignalIORequest
 */
void IOBase::exportSignals(const SignalIORequest &rq)
{
    abort_flag = false;
    // get export flags from request
    e_flag_raw = rq.userData.value(13, true).toBool();
    e_flag_abs = rq.userData.value(14, true).toBool();
    e_flag_tmp = rq.userData.value(15, true).toBool();
    e_flag_raw_postproc = rq.userData.value(16, true).toBool();
    e_flag_abs_postproc = rq.userData.value(17, true).toBool();
    e_flag_raw_stdev = rq.userData.value(28, false).toBool();
    e_flag_abs_stdev = rq.userData.value(29, false).toBool();

    e_flag_temp_postproc = rq.userData.value(25, true).toBool();
    e_flag_temp_stdev = rq.userData.value(26, false).toBool();

    e_flag_matlab_compression = rq.userData.value(35, false).toBool();

    QtConcurrent::run(this,&IOBase::exportImplementation,rq);
   // exportImplementation(rq);
}


/**
 * @brief IOBase::onExportImplementationFinished This
 * slot is executed when an exportImplementation (in subclass)
 * emitted the 'exportImplementationFinished' Signal.
 * The IOBase instance commits suicide if autodelete is enabled
 * and emits the final 'exportFinished' signal.
 * @param time
 */
void IOBase::onExportImplementationFinished(double time)
{
    if(m_autoDelete)
        this->deleteLater();
    emit exportFinished(time);
}


/**
 * @brief IOBase::setAutoDelete If auto-delete is set to true, this object will
 * destroy itself when import/export is done.
 * @param state
 */
void IOBase::setAutoDelete(bool state)
{
    m_autoDelete = true;
}


/**
 * @brief IOBase::setEnabledConcurrency enable/disable multithreaded import.
 * Multithreading is enabled per default!
 * @param state new value
 */
void IOBase::setEnabledConcurrency(bool state)
{
    m_concurrency = state;
}


void IOBase::checkImportRequest(const SignalIORequest &irq)
{
    m_initialRequest = irq;
    abort_flag = false;

    checkFiles();
}


/**
 * @brief IOBase::onImportSetupFinished This slot is executed when the file-format-specific
 * IOBase::setupImport() method has emitted the finished signal.
 * Based on the generated io-requests (should be created during setup process) new
 * MRun Objects are created and concurrent calles to IOBase::importSignals() are made.
 *
 */
void IOBase::onImportSetupFinished()
{
    try
    {
        // work estimate
        for(int i=0; i< m_generatedRequests.size(); i++)
        {
            SignalIORequest irq = m_generatedRequests.at(i);
            if(irq.flist.isEmpty())
                continue;

            // define how data are processed
            if(p_mode == PM_PERMRUN)    // make one call per mrun
                m_task_count += 1;
            else if(p_mode == PM_PERCHANNEL)    // make one call per mrun-channel
                m_task_count += irq.noChannels;
            else if(p_mode == PM_PERFILE)   // make one call per file
                m_task_count += irq.flist.size();
        }

        // iterate through io-requests 
        for(int i=0; i< m_generatedRequests.size(); i++)
        {

            SignalIORequest irq = m_generatedRequests.at(i);

            QString val;
            val.sprintf("%d",m_generatedRequests.at(i).flist.size());
            logMessage("Load MRun: "+irq.runname+" ("+irq.runsettings_filename+")"+": "+val+" files ...");

            // read MRun informations from request
            int noChannels = irq.noChannels;
            QString runname;

            if(irq.runname.isEmpty())
            {
               runname.sprintf("Run %d", Core::instance()->dataModel()->mrunCount());
            }
            else
               runname = irq.runname;

            // create no mrun if the import requests filelist is empty
            if(irq.flist.isEmpty())
            {
                logMessage("Skipping import of Run "+runname+" (no datafiles!)", WARNING);
                continue;
            }

            MRunGroup* group = Core::instance()->dataModel()->group(irq.group_id);

            // add new mrun object to list
            MRun* mrun;

            // use the specified mrun object if set
            if(m_mrun == 0)
            {
                // use existing run if id is valid
                if(irq.run_id == -1)
                {
                    if(irq.runsettings_filename.isEmpty())
                    {
                        mrun = new MRun( runname, noChannels, group);
                    }
                    else
                    {
                        mrun = new MRun(runname, irq.runsettings_filename, noChannels, group);
                    }
                    // register the mrun to the programs datamodel
                }
                else
                    mrun = Core::instance()->dataModel()->mrun(irq.run_id);
            }
            else
            {
                mrun = m_mrun;
            }

            if(!mrun)
                throw LIISimException(
                        QString("IOBase mrun with id %0 does not exist!").arg(irq.run_id));

            mrun->setImportRequest(irq);

            if(m_initialRequest.itype == SignalIOType::CUSTOM)
            {
                ConsistencyCheck cc;
                bool ccError = cc.check(m_initialRequest);
                if(ccError)
                {
                    MSG_ASYNC(QString("Consistency Check returned errors for %0, import might lead to unexpected results.").arg(irq.runname), LIISimMessageType::WARNING);
                    cc.prepareRun(mrun);
                }
            }

            if(m_initialRequest.itype != SignalIOType::XML)
            {
                mrun->setLiiSettings(getLIISettings( irq ));

                if(!irq.runsettings_dirpath.isEmpty())
                {
                    MRunSettings ms(mrun);
                    ms.load(irq.runsettings_dirpath);
                }
            }

            int nofiles = irq.flist.size();

            // make concurrent importSetp calls. Use the parallelization mode
            // control behavior of concurrent calls

            if(p_mode == PM_PERMRUN)    // make one call per mrun
            {              
                if(m_concurrency)
                    QtConcurrent::run(this,&IOBase::importStep,mrun,irq.flist);
                else
                    importStep(mrun,irq.flist);
            }
            else if(p_mode == PM_PERCHANNEL)    // make one call mrun-channel
            {               
                //create a list of signalfileinfos for each channel of current mrun
                QList<SignalFileInfoList> channelList;

                for(int c = 0; c < noChannels; c++)
                {
                    SignalFileInfoList clist;

                    for(int j=0; j < nofiles; j++)
                    {
                        SignalFileInfo fileInfo = irq.flist.at(j);
                        if(fileInfo.channelId == (c+1))
                        {
                            clist << fileInfo;
                        }
                    }
                    channelList.append(clist);
                }

                // make concurrent calls for each channel
                for(int c = 0; c < channelList.size(); c++)
                {
                    if(m_concurrency)
                        QtConcurrent::run(this,&IOBase::importStep,mrun,channelList.at(c));
                    else
                        importStep(mrun,channelList.at(c));
                }
            }
            else if(p_mode == PM_PERFILE)   // make one call per file
            {             
                for(int j = 0; j < nofiles; j++)
                {
                    SignalFileInfoList sList;
                    sList << irq.flist.at(j);

                    if(m_concurrency)
                        QtConcurrent::run(this,&IOBase::importStep,mrun,sList);
                    else
                        importStep(mrun,sList);
                }
            }

            // TODO CHECK EVENTLOOP FOR POSSIBLE CRASHES
            QApplication::processEvents();
        }
    }
    catch(LIISimException e)
    {
        qDebug() << e.what();
    }

    m_kill_ok1 = true;

    // if there is nothing to do, call the finished method directly
    if(m_task_count == 0)
    {
        m_finished_tasks--;
        onImportStepFinished(0);
    }

    if(m_kill_ok1 && m_kill_ok2 )
    {
        // emit the finished signal which will be caught by SignalManager
        emit importFinished(m_numberOfSignals,m_timer.elapsed()/1000.0);

        if(m_autoDelete)
            this->deleteLater();
    }
}


/**
 * @brief IOBase::onImportStepFinished This slot is executed when a importStep() has been finished.
 * If all importStep-calls are done the final IOBase::importFinished() signal is emitted (back to SignalManager)
 * @param noSignals number of read signals within importStep()
 */
void IOBase::onImportStepFinished(int noSignals)
{
    m_finished_tasks++;
    m_numberOfSignals += noSignals;

   // qDebug() << "finished tasks: " <<m_finished_tasks << " tc: "<<m_task_count;

    if(m_finished_tasks == m_task_count)
    {

        // set the 2nd kill-ok flag
        m_kill_ok2 = true;
    }

    emit progressUpdate( (float)m_finished_tasks/(float)m_task_count);

    if(m_kill_ok1 && m_kill_ok2 )
    {

        // delete empty groups which have been generated during import
        for(int i = 0; i < m_generatedRequests.size(); i++)
        {
            MRunGroup* group = Core::instance()->dataModel()->group(m_generatedRequests[i].group_id);
            if(group && group->childCount() == 0 )
                delete group;
        }
        // delete empty groups which have been generated during import
        MRunGroup* group = Core::instance()->dataModel()->group(m_initialRequest.group_id);
        if(group && group->childCount() == 0 )
            delete group;

        // emit the finished signal which will be caught by SignalManager
        emit importFinished(m_numberOfSignals,m_timer.elapsed()/1000.0);

        if(m_autoDelete)
            this->deleteLater();
    }
}


/**
 * @brief IOBase::getLIISettings helper method, gets LIISettings from SignalIORequest's user data.
 * If the requested database entry does not exist, default LIISettings will be returned.
 * @param irq SignalIORequest
 * @return LIISettings
 */
LIISettings IOBase::getLIISettings(SignalIORequest & irq)
{
    QString filename = irq.userData.value(3).toString();
    int index = Core::instance()->getDatabaseManager()->indexOfLIISettings(filename);
    LIISettings liisettings;
    if(index == -1)
    {
        liisettings = Core::instance()->modelingSettings->defaultLiiSettings();
        MSG_WARN("Import of MRun " + irq.runname
                 + ": liisettings not found: '" + filename
                 + "'. Using default LIISettings instead (" + liisettings.name+ ").");
    }
    else
        liisettings = *Core::instance()->getDatabaseManager()->getLIISetting(index);
    return liisettings;
}
