#include "signalmanager.h"
#include "../general/LIISimException.h"

#include <QFile>
#include <QTime>
#include <QDir>
#include <QDirIterator>
#include <QStack>
#include <QMutexLocker>
#include <QTextStream>
#include <QThreadPool>
#include <QThread>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QList>
#include <QMap>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMessageBox>

#include "../core.h"

#include "../io/iocsv.h"
#include "../io/iomatlab.h"
#include "../io/iocustom.h"
#include "../io/ioxml.h"
#include "../../gui/signalEditor/importdialog.h"
#include "../../gui/signalEditor/exportdialog.h"

#include "mrungroup.h"
#include "processing/processingtask.h"
#include "processing/processingchain.h"
#include "processing/processingplugin.h"
#include "../../models/datamodel.h"
#include "../../calculations/fit/fitrun.h"


/**
 * @brief SignalManager::SignalManager Constructor
 * @param parent parent Object (default 0)
 */
SignalManager::SignalManager(DataModel *dataModel, QObject *parent) :  QObject(parent),
    m_dataModel(dataModel)
{
    MSG_DETAIL_1("init SignalManager");
    threadPool = QThreadPool::globalInstance();
    initActions();

    isLoading = false;
    m_isFitting = false;
    pendingProcessingTasks =0;

    connect(m_dataModel,SIGNAL(fitRunCountChanged()),SLOT(onFitrunCountChanged()));

    m_calcMode = 0;
}


void SignalManager::initActions()
{
    m_importAction = new QAction(QString("Load Signal Data"),this);
    m_importAction->setIcon(QIcon(Core::rootDir + "resources/icons/folder.png"));
    connect(m_importAction,SIGNAL(triggered()),SLOT(handleActionImport()));

    m_exportAction = new QAction(QString("Save Signal Data"),this);
    m_exportAction->setIcon(QIcon(Core::rootDir + "resources/icons/file_save_as.png"));
    connect(m_exportAction,SIGNAL(triggered()),SLOT(handleActionExport()));
}


/**
 * @brief SignalManager::processSignals
 * @param mrun current MRun (selected in GUI)
 * @param startStype signal type to start with (order raw, abs, tempr)
 */
// replaced by typeList
/*
void SignalManager::processSignals(MRun *mrun, Signal::SType startStype)
{
    QList<MRun*> runs;

    if(m_calcMode == 0) // calculate current run only
        runs << mrun;
    else if(m_calcMode == 1)
    {
        MRunGroup* g = mrun->group();
        if(!g)return;
        runs = g->mruns();
    }
    else if(m_calcMode == 2)
        runs = m_dataModel->mrunList();

    processRunList(runs,startStype);
}
*/

/**
 * @brief SignalManager::processSignals for processing of specific signal types (see checkboxes in SignalProcessingEditor:checkboxCalcRaw )
 * @param mrun current MRun (selected in GUI)
 * @param typelist signal type list to be processed (raw, abs, tempr)
 */
void SignalManager::processSignals(MRun *mrun, QList<Signal::SType> typeList)
{
    // Reset all messages related to database content
    MSG_ONCE_RESET_GROUP("SpectroscopicMaterial");

    QList<MRun*> runs;

    if(m_calcMode == 0) // calculate current run only
        runs << mrun;
    else if(m_calcMode == 1)
    {
        MRunGroup* g = mrun->group();
        if(!g)return;
        runs = g->mruns();
    }
    else if(m_calcMode == 2)
        runs = m_dataModel->mrunList();

    if(runs.size() == 0)
    {
        MESSAGE("nothing to process",LIISimMessageType::WARNING);
        return;
    }

    // do no signal processing if signal imports are pending!
    if(isImporting())
    {
        MSG_WARN("SignalManager: cannot process MRun (imports pending!)");
        return;
    }

    if(isExporting())
    {
        MSG_NORMAL("SignalManager: cannot process MRun (exports pending!)");
        return;
    }

    if(isBusy())
    {
        MSG_NORMAL("SignalManager: cannot process MRun (still busy!)");
        return;
    }

    if(m_dataModel->mrunCount() == 0)
    {
        MESSAGE("no signals for calculation available!",LIISimMessageType::WARNING);
        return;
    }

    // memory test
    if(!checkMemory())
        return;

    emit processingStateChanged(true);
    QString msg;
    msg = QString("Processing signal data of %0 run(s), please wait...").arg(runs.size());
    MSG_NORMAL(msg);
    MSG_STATUS_CONST(msg);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if(pendingProcessingTasks == 0)
        processingTimer.start();

    totalProcessingTasks = runs.size();

    for(int i = 0; i < runs.size(); i++)
    {
        if(!runs.at(i)->isBusy())
        {
            // check/update run's calculation status
            runs[i]->updateCalculationStatus();

            // do not process signals of run if the status check was not successful
            if(runs[i]->calculationStatus()->isError())
            {
                runs[i]->calculationStatus()->addError("Calculation skipped");
                totalProcessingTasks--;
                continue;
            }

            ProcessingTask* task = new ProcessingTask(runs.at(i), typeList);
            task->setAutoDelete(true);
            pendingProcessingTasks++;

            connect(task, SIGNAL(finished(MRun*)),
                    this, SLOT(onProcessingTaskFinished(MRun*)),
                    Qt::QueuedConnection);

            connect(task, SIGNAL(signal_msg(QString,LIISimMessageType)),
                    this, SLOT(onMessageReceive(QString,LIISimMessageType)),
                    Qt::QueuedConnection);

            connect(this, SIGNAL(processingTasksStop()),
                    task, SLOT(stopTask()),
                    Qt::QueuedConnection);

            threadPool->start(task);
        }
        else
        {
            QString msg = "SignalManager::processRunList: run " + runs.at(i)->getName() + " is still busy, no recalculation!";
            MESSAGE(msg,LIISimMessageType::WARNING);
        }
    }

    if(totalProcessingTasks == 0)
    {
        QApplication::restoreOverrideCursor();
        emit allProcessingTasksFinished();
        emit processingStateChanged(false);
        ProcessingTask::resetIdCounter();
        QString msg = "Status Check of all runs failed! Signal processing aborted.";
        MESSAGE(msg,INFO);
        MSG_STATUS(msg);
    }
}


/**
 * @brief SignalManager::checkMemory performs a check (if not bypassed by GUI)
 * if enough memory is available
 * @return returns true if enough memory is available or if the test has
 * been bypassed, returns false if the memory check failed
 */
bool SignalManager::checkMemory()
{
    // check if memory test is bypassed
    if(!core->guiSettings->value("memusage","bypass",true).toBool())
    {
        // etsimate memory cost of next processing task
        if(!mem_monitor.checkIfProcessingIsAllowed())
        {

            QString warnmsg = QString("LIISim cannot process signals due"
                                      " to memory limitations (max. "
                                      "allowed: %0 MB RAM). Please disable some "
                                      "step buffers or reduce the number of measurement "
                                      "runs/processing steps.")
                    .arg(MemUsageMonitor::toMB(mem_monitor.physicalMemoryAllowed()));

            if(mem_monitor.is32bit())
                warnmsg.append("\n\nNote: you are using a 32 bit version of LIISim. "
                               "This reduces the amount of memory available dramatically. "
                               "Please consider to use a 64 bit version of the program.");

            MESSAGE("NOT ENOUGH MEMORY AVAILABLE: " + warnmsg, LIISimMessageType::WARNING);

            // show scary warn message
            QMessageBox::warning(0,"Not enougth memory available!",warnmsg);
            return false;
        }
    }
    else
    {
        // if checkbox memory test is unchecked
        qDebug() << "Memory usage check DISABLED";
        MSG_DETAIL_1("!!! MEMORY TEST BYPASSSED (Program could crash if not enough memory is available!!!");
    }
    return true;
}


/**
 * @brief SignalManager::processAllMRuns process signals of all runs
 * beginning with the given signal type (default raw, execution order:
 * raw->absolute->temperature)
 * @param startStype start signal type
 */
void SignalManager::processAllMRuns(Signal::SType startStype)
{
    QList<MRun*> runList = m_dataModel->mrunList();
    processRunList(runList, startStype);
}


/**
 * @brief SignalManager::processRunList calculate processing chains for all runs in list,
 * starting with processing chains of given start signal type
 * @param runs list of measurement runs
 * @param startStype first signal type which should be processed (default: Raw)
 */
void SignalManager::processRunList(QList<MRun *> &runs, Signal::SType startStype)
{
    if(runs.size() == 0)
    {
        MESSAGE("nothing to process",LIISimMessageType::WARNING);
        return;
    }

    // do no signal processing if signal imports are pending!
    if(isImporting())
    {
        MSG_WARN("SignalManager: cannot process MRun (imports pending!)");
        return;
    }

    if(isExporting())
    {
        MSG_NORMAL("SignalManager: cannot process MRun (exports pending!)");
        return;
    }

    if(isBusy())
    {
        MSG_NORMAL("SignalManager: cannot process MRun (still busy!)");
        return;
    }

    if(m_dataModel->mrunCount() == 0)
    {
        MESSAGE("no signals for calculation available!",LIISimMessageType::WARNING);
        return;
    }

    // memory test
    if(!checkMemory())
        return;

    emit processingStateChanged(true);
    QString msg;
    msg = QString("Processing signal data of %0 run(s), please wait...").arg(runs.size());
    MSG_NORMAL(msg);
    MSG_STATUS_CONST(msg);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if(pendingProcessingTasks == 0)
        processingTimer.start();

    totalProcessingTasks = runs.size();

    for(int i = 0; i < runs.size(); i++)
    {
        if(!runs.at(i)->isBusy())
        {
            // check/update run's calculation status
            runs[i]->updateCalculationStatus();

            // do not process signals of run if the status check was not successful
            if(runs[i]->calculationStatus()->isError())
            {
                runs[i]->calculationStatus()->addError("Calculation skipped");
                totalProcessingTasks--;
                continue;
            }

            ProcessingTask* task = new ProcessingTask(runs.at(i),startStype);
            task->setAutoDelete(true);
            pendingProcessingTasks++;

            connect(task,SIGNAL(finished(MRun*)),
                    this, SLOT(onProcessingTaskFinished(MRun*)),
                    Qt::QueuedConnection);

            connect(task,SIGNAL(signal_msg(QString,LIISimMessageType)),
                    this, SLOT(onMessageReceive(QString,LIISimMessageType)),
                    Qt::QueuedConnection);

            threadPool->start(task);
        }
        else
        {
            QString msg = "SignalManager::processRunList: run " + runs.at(i)->getName() + " is still busy, no recalculation!";
            MESSAGE(msg,LIISimMessageType::WARNING);
        }
    }

    if(totalProcessingTasks == 0)
    {
        QApplication::restoreOverrideCursor();
        emit allProcessingTasksFinished();
        emit processingStateChanged(false);
        ProcessingTask::resetIdCounter();
        QString msg = "Status Check of all runs failed! Signal processing aborted.";
        MESSAGE(msg,INFO);
        MSG_STATUS(msg);
    }
}


void SignalManager::onProcessingTaskFinished(MRun *mrun)
{
    QMutexLocker lock(&procMutex);
    pendingProcessingTasks--;

    emit mrun->processingFinished();
    emit processingFinished(mrun->getName(),Signal::TEMPERATURE);    
    double pTime = processingTimer.elapsed() ;

    QString msg;

    if(pendingProcessingTasks % 2 != 0)
    {
        double processedTasks = totalProcessingTasks - pendingProcessingTasks;
        double timeLeft = round((pTime/ (processedTasks/totalProcessingTasks) - pTime) / 1000);

        msg = QString("Processing signal data of %0/%1 runs, please wait... (estimated time: %2 s left)")
                                .arg(processedTasks)
                                .arg(totalProcessingTasks)
                                .arg(timeLeft);
        MSG_STATUS_CONST(msg);
    }

    if(!mrun->calculationStatus()->isError() && !mrun->calculationStatus()->isCancelled())
    {
        //mrun->calculationStatus()->addSuccessMessage("Calculation successful");
        mrun->calculationStatus()->setProcessingFinishedSuccessful();
    }


    if(pendingProcessingTasks <= 0)
    {
        if(pTime > 1000)
        {
            msg.sprintf("Signal processing done (elapsed time %.1f s)",pTime/1000);
        }
        else
        {
            msg.sprintf("Signal processing done (elapsed time %.0f ms)",pTime);
        }
        MESSAGE(msg,NORMAL);
        MSG_STATUS_CONST(msg);

        QApplication::restoreOverrideCursor();
        emit allProcessingTasksFinished();        
        emit processingStateChanged(false);
        ProcessingTask::resetIdCounter();

        pendingProcessingTasks = 0;

        mem_monitor.updateMemInfo();
        mem_monitor.runDataEstimate();
    }
}


void SignalManager::onMessageReceive(const QString &msg, const LIISimMessageType &msg_type)
{
    MESSAGE(msg, msg_type);
}


/**
 * @brief SignalManager::setCalculationMode Sets the calculation
 * mode. This Option defines which runs should be considered during
 * the next signal processing event. Possible modes are:
 *  0: current (single) run only
 *  1: all runs in same group as current run
 *  2: all runs availabe
 *  (mode is selected by calc mode radio button in SignalEditor)
 * @param mode new mode
 */
void SignalManager::setCalculationMode(int mode)
{
    // check bounds
    if(mode < 0 || mode > 2)
        return;
    m_calcMode = mode;
}



/************************************
 ***            IMPORT            ***
 ***
 ***  - SignalManager Connects GUI (importdialog.cpp) with IOBase class
 ***  - SignalManager loads the right import class (e.g. IOcsv, IOcustom,...)
 ***    dependend on IORequest
 ***  - IRQ (Import request) contains all information necessary for import
 ***
 ************************************/

/**
 * @brief SignalManager::handleActionImport opens import window and
 * connects importdialog.cpp with signalmanager
 */
void SignalManager::handleActionImport()
{
    ImportDialog diag(core);
    connect(&diag,SIGNAL(signalGuiImportRequest(SignalIORequest)),this,SLOT(importSignalsManager(SignalIORequest)));
    connect(&diag,SIGNAL(checkImportRequest(SignalIORequest)), SLOT(checkImportRequest(SignalIORequest)));
    connect(this, SIGNAL(checkImportRequestFinished(QList<SignalIORequest>)), &diag, SLOT(onCheckImportRequestResults(QList<SignalIORequest>)));
    connect(&diag, SIGNAL(loadImportRequests(QList<SignalIORequest>)), SLOT(loadImportRequests(QList<SignalIORequest>)));
    connect(this, SIGNAL(ioImportSuccess(SignalIORequest,SignalFileInfoList)), &diag, SLOT(onIOImportSuccess(SignalIORequest,SignalFileInfoList)));
    connect(this, SIGNAL(ioImportError(SignalIORequest,SignalFileInfo,QString)), &diag, SLOT(onIOImportError(SignalIORequest,SignalFileInfo,QString)));
    diag.exec();
}


void SignalManager::checkImportRequest(SignalIORequest irq)
{
    IOBase* io = nullptr;

    //create io-objects based on io type
    if(irq.itype == CSV_SCAN || irq.itype == CSV)
    {
        io = new IOcsv;
    }
    else if(irq.itype == CUSTOM)
    {
        io = new IOcustom;
    }
    else if(irq.itype == XML)
    {
        io = new IOxml;
    }

    connect(io, SIGNAL(checkFilesResult(QList<SignalIORequest>)), SLOT(onCheckFilesResult(QList<SignalIORequest>)));
    io->setAutoDelete(true);

    QtConcurrent::run(io, &IOBase::checkImportRequest, irq);

    //emit checkImportRequestFinished(io->checkImportRequest(irq));
}


void SignalManager::loadImportRequests(QList<SignalIORequest> irq)
{
    if(!irq.isEmpty())
    {
        MSG_STATUS_CONST("Loading signal data, please wait...");

        emit importStateChanged(true);
        // start time measurement for loading process
        isLoading = true;

        // adjust number of cores for import
        QThreadPool::globalInstance()->setMaxThreadCount( core->generalSettings->coreCountImport() );

        IOBase* io = 0;

        //create io-objects based on io type
        if(irq.first().itype == CSV_SCAN || irq.first().itype == CSV)
        {
            io = new IOcsv;
        }
        else if(irq.first().itype == CUSTOM)
        {
            io = new IOcustom;
        }
        else if(irq.first().itype == XML)
        {
            io = new IOxml;
        }
        else
        {
            // unknown import types
            onImportFinished(0,0.0);
            return;
        }

        io->setAutoDelete(true);

        connect(io,SIGNAL(importFinished(int,double)),
                SLOT(onImportFinished(int,double)),
                Qt::QueuedConnection);

        connect(io,SIGNAL(progressUpdate(float)),
                this,SLOT(onProgressUpdate(float)),
                Qt::QueuedConnection);

        connect(io, SIGNAL(importSuccess(SignalIORequest,SignalFileInfoList)), SLOT(onIOImportSuccess(SignalIORequest,SignalFileInfoList)));

        connect(io, SIGNAL(importError(SignalIORequest,SignalFileInfo,QString)), SLOT(onIOImportError(SignalIORequest,SignalFileInfo,QString)));

        io->importSignals(irq);
    }
}


/**
 * @brief SignalManager::importSignals handles import requests from sent by
 * ImportDialog::onOk()
 * @param irq SignalIORequest
 * @details creates io object, which will do the import work.
 * connects to the "importFinished()" signal of io-object
 */
void SignalManager::importSignalsManager(SignalIORequest irq)
{
    MSG_STATUS_CONST("Loading signal data, please wait...");

    emit importStateChanged(true);
    // start time measurement for loading process
    isLoading = true;

    // adjust number of cores for import
    QThreadPool::globalInstance()->setMaxThreadCount( core->generalSettings->coreCountImport() );

    IOBase* io = 0;

    //create io-objects based on io type
    if(irq.itype == CSV_SCAN || irq.itype == CSV)
    {
        io = new IOcsv;
    }
    else if(irq.itype == CUSTOM)
    {
        io = new IOcustom;
    }
    else if(irq.itype == XML)
    {
        io = new IOxml;
    }
    else
    {
        // unknown import types
        onImportFinished(0,0.0);
        return;
    }

    io->setAutoDelete(true);

    connect(io,SIGNAL(importFinished(int,double)),
            SLOT(onImportFinished(int,double)),
            Qt::QueuedConnection);

    connect(io,SIGNAL(progressUpdate(float)),
            this,SLOT(onProgressUpdate(float)),
            Qt::QueuedConnection);

    io->importSignals(irq);
}


void SignalManager::onProgressUpdate(float value)
{
    emit progressUpdate((int)(value*100));
}


/**
 * @brief SignalManager::onImportFinished Handles the IOBase::importFinished() signal of io-objects
 * and notifies listerners about state changes.
 * @param noSignals number of signals read
 * @param time loading time in seconds
 */
void SignalManager::onImportFinished(int noSignals, double time)
{


    // reset number of cores
    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());


    if(IOBase::abort_flag)
    {
        MSG_STATUS("Loading aborted.");
        MSG_INFO("Import aborted!");
        IOBase::abort_flag = false;
    }
    else
    {
        MSG_STATUS("Loading done.");

        if(noSignals > 0)
        {
            QString msg;
            msg.sprintf("Read %d signals (time %2.2f s)", noSignals, time);
            MSG_NORMAL(msg);
        }
    }

    // MSG_DETAIL_1("SignalManager::onImportFinished(): DETAILED DATATREE AFTER IMPORT");
    // dataModel()->rootItem()->printDebugTree();
    // MSG_DETAIL_1("SignalManager::onImportFinished(): END OF TREE");

    mem_monitor.updateMemInfo();
    mem_monitor.runDataEstimate();

    // update calculation states of all runs.
    QList<MRun*> runs = Core::instance()->dataModel()->mrunList();
    for(int i = 0; i < runs.size(); i++)
        runs[i]->updateCalculationStatus();

    emit importFinished();
    emit importStateChanged(false);
    isLoading = false;
}



/************************************
 ***            EXPORT            ***
 ************************************/


void SignalManager::handleActionExport()
{
    ExportDialog diag;
    connect(&diag,SIGNAL(signalExportRequest(SignalIORequest)),this,SLOT(exportSignalsManager(SignalIORequest)));
     //connect(&diag,SIGNAL(signalCanceled()),this,SLOT(onExportCanceled()));
    diag.exec();
}


/**
 * @brief SignalManager::exportSignals
 *  gets export request from gui/signalEditor/ExportDialog::onOk().
 * Generates IOBase instances and calles IOBase::exportSignals().
 * @param rq
 */
void SignalManager::exportSignalsManager(SignalIORequest rq)
{
    qDebug() << "SignalManager: export signals (not parallelized!) " << rq.userData.value(0).toString();

    // check if export is allowed at this time
    if(isImporting())
    {
        MESSAGE("Cannot export signals during import!", WARNING);
        return;
    }

    if(pendingProcessingTasks >0 )
    {
        MESSAGE("Cannot export signals during calculation!", WARNING);
        return;
    }

    if(m_isExporting)
    {
        MESSAGE("Cannot export signals, pending exports!", WARNING);
        return;
    }

    // start export routine
    m_isExporting = true;
    emit exportStateChanged(true);
    MSG_STATUS("Exporting signal data, please wait...");

    // only CSV Export, other export types not implemented
    try
    {
        IOBase* io = 0;
        if(rq.itype == CSV)
        {
            io = new IOcsv();

        }
        else if(rq.itype == MAT)
        {
            io = new IOmatlab();
        }
        else if(rq.itype == XML)
        {
            io = new IOxml();
        }

        if(io == 0)
        {
            MESSAGE("Export type not supported! Export canceled!", WARNING);
            onExportFinished(0.0);
            return;
        }

        io->setAutoDelete(true);

        // connect the IOBase::exportFinished signal, which will
        //  call onExportFinished when the io-object has finished the export-job
        connect(io,SIGNAL(exportFinished(double)),SLOT(onExportFinished(double)));

        connect(io,SIGNAL(progressUpdate(float)),
                this,SLOT(onProgressUpdate(float)),
                Qt::QueuedConnection);

        io->exportSignals(rq);
    }
    catch(LIISimException e)
    {
        MESSAGE(e.what(),e.type());
    }

}


/**
 * @brief SignalManager::onExportFinished Handles the IOBase::exportFinished() signal
 * and notifies listerners about state changes
 * @param time export time in seconds
 */
void SignalManager::onExportFinished(double time)
{
    if(IOBase::abort_flag)
    {
        MSG_STATUS("Export aborted!");
        MSG_INFO("Export aborted!");
        IOBase::abort_flag = false;
    }
    else
    {
        QString msg = QString("Export: done (%0 s).").arg(time);
        MSG_NORMAL(msg);
        MSG_STATUS(msg);
    }

    m_isExporting = false;
    emit exportStateChanged(false);
    emit exportFinished();
}


/************************************
 ***            FitRun            ***
 ************************************/

/**
 * @brief SignalManager::onFitrunCountChanged this slot
 * is executed when the a Fitrun has been (un)registered
 * to the DataModel.
 *
 */
void SignalManager::onFitrunCountChanged()
{
    QList<FitRun*> fr = m_dataModel->fitRuns();
    for(int i = 0; i < fr.size(); i++)
    {
        connect(fr[i],SIGNAL(fitStarted()),this,SLOT(onFitStarted()),Qt::UniqueConnection);
        connect(fr[i],SIGNAL(fitFinished()),this,SLOT(onFitFinished()),Qt::UniqueConnection);
    }
}


/**
 * @brief SignalManager::onFitStarted This slot is executed when any observed
 * Fitrun has started a fit.
 */
void SignalManager::onFitStarted()
{    
    m_isFitting = true;
    emit fitStateChanged(true);
}


/**
 * @brief SignalManager::onFitFinished This slot is executed when any observed
 * Fitrun has finished a fit.
 */
void SignalManager::onFitFinished()
{
    m_isFitting = false;
    emit fitStateChanged(false);
}


void SignalManager::cancelProcessingTasks()
{
    emit processingTasksStop();
}


void SignalManager::onCheckFilesResult(QList<SignalIORequest> result)
{
    emit checkImportRequestFinished(result);
}


void SignalManager::onIOImportSuccess(SignalIORequest source, SignalFileInfoList fileList)
{
    emit ioImportSuccess(source, fileList);
}


void SignalManager::onIOImportError(SignalIORequest source, SignalFileInfo file, QString error)
{
    emit ioImportError(source, file, error);
}
