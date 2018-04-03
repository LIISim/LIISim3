#include "da_blocksequence.h"

#include "dataacquisitionwindow.h"

DA_BlockSequenceWorker::DA_BlockSequenceWorker(QObject *parent) : QThread(parent), blockRunLock(1)
{
    connect(this, SIGNAL(finished()), SLOT(onFinished()));

    try
    {
        DataAcquisitionWindow *daw = dynamic_cast<DataAcquisitionWindow*>(parent);
        connect(daw, SIGNAL(blockAcquisitionFinished()), SLOT(blockCaptured()));
        connect(this, SIGNAL(startBlockRun()), daw, SLOT(runBlock()));
    }
    catch(std::exception& e)
    {
        qDebug() << "BlockSequenceWorker: exception occured casting parent to DataAcquisitionWindow - " << e.what();
    }
}

DA_BlockSequenceWorker::~DA_BlockSequenceWorker()
{
    stop();
}

void DA_BlockSequenceWorker::run()
{
    qDebug() << "BlockSequenceWorker::run() started";

    threadShouldStop = false;
    voltageBoundReached = false;
    if(blockRunLock.available() == 0)
        blockRunLock.release();

    DataAcquisitionWindow *daw;
    try
    {
        daw = dynamic_cast<DataAcquisitionWindow*>(parent());
    }
    catch(std::exception& e)
    {
        qDebug() << "BlockSequenceWorker: exception casting parent to DataAcquisitionWindow - " << e.what();
    }

    //---------------------------------------/
    //    create alternating voltage list
    //---------------------------------------/

    // TEST
//    QList<double> testGainList;
//    testGainList << 0.7 << 0.5 << 0.45 << 0.4;
//    daw->psSettingsWidget->updateGainList(testGainList);
//    threadShouldStop =  true;


    // start voltages (never go higher)
    QList<double> initGainList = daw->psSettingsWidget->getGainList();

    // number of alternating blocks
    int blockNumber     = daw->psSettingsWidget->getBlockSequenceBlockNumber();
    double decrement    = daw->psSettingsWidget->getBlockSequenceVoltageDecrement();
    double minVoltage   = Core::instance()->devManager->getAnalogOutLimitMin();
    double waitTime     = daw->psSettingsWidget->getBlockSequenceDelaySec();
    int noCaptures      = daw->psSettingsWidget->getNoCaptures();


    qDebug() << "BlockSequence: init gainlist " << initGainList.size();
    qDebug() << "BlockSequence: " << initGainList.at(0) << initGainList.at(1) << initGainList.at(2) << initGainList.at(3);
    qDebug() << "BlockSequence: decrement " << decrement;
    qDebug() << "BlockSequence: blocks " << blockNumber;

    // number of measurements
    int nsize = 1;
    int ch_nsize;

    // determine number of measurements
    for(int i = 0; i < initGainList.size(); i++)
    {
        double max = initGainList.at(i);

        ch_nsize = ceil(( max - minVoltage) / decrement);

        if(ch_nsize > nsize)
            nsize = ch_nsize;
    }

    // update GUI
    updateTimeLeft(daw, nsize, waitTime, noCaptures);

    // list of gain values
    QList<QList<double>> voltSequences;

    QList<double> gainValues, gainList;

    // create list of sequences
    for(int j = 0; j < nsize; j++)
    {
        if(j == 0)
        {
            voltSequences.append(initGainList);
            continue;
        }

        gainValues.clear();

        // for all channels
        for(int i = 0; i < initGainList.size(); i++)
        {
            double old_voltage = voltSequences.at(j-1).at(i);
            double new_voltage = old_voltage - decrement;

            if(new_voltage < minVoltage)
            {
                // start with initial value for this channel again
                gainValues.append(initGainList.at(i));
            }
            else
                gainValues.append(new_voltage);
        }

        // add voltage list for all channels to sequence
        voltSequences.append(gainValues);
        qDebug() << "BlockSequence: gain SET: " << gainValues.at(0)
                 << gainValues.at(1) << gainValues.at(2) << gainValues.at(3);

    }

    // create new list only if more than one block should be measured
    if(blockNumber > 1)
    {
        QList<QList<double>> voltSequences_temp;

        for(int b = blockNumber; b > 0; b--)
        {
            for(int k = voltSequences.size(); k > 0; k--)
            {
                if((k % b) == 0)
                {
                    gainValues = voltSequences.takeAt(k-1);
                    voltSequences_temp.append(gainValues);
                }
            }
        }
        std::reverse(voltSequences_temp.begin(), voltSequences_temp.end());
        voltSequences = voltSequences_temp;
    }

    daw->autoStartStreaming = false;

    while(!threadShouldStop && !voltageBoundReached)
    {
        QThread::msleep(10);

        qDebug() << "BlockSequenceWorker: acquire lock...";
        blockRunLock.acquire();

        QThread::msleep(10);

        // decrease not for first gain values
        if(!voltSequences.isEmpty())
            gainList = voltSequences.takeFirst();
        else
            voltageBoundReached = true;

        if(!voltageBoundReached)
        {            
            // set new gain voltages and wait
            daw->psSettingsWidget->updateGainList(gainList);

            long waittime_ms = (unsigned long)(daw->psSettingsWidget->getBlockSequenceDelaySec() * 1000);

            emit status("Waiting...");

            updateTimeLeft(daw, voltSequences.size(), waitTime, noCaptures);

            while(waittime_ms > 0 && !threadShouldStop)
            {
                waittime_ms -= 10;
                QThread::msleep(10);
            }

            // collect signals
            if(!threadShouldStop)
            {
                emit status("Capturing...");
                emit startBlockRun();
            }
        }
    }

    // reset GUI
    resetTimeLeft(daw);

    daw->autoStartStreaming = true;

    qDebug() << "BlockSequenceWorker::run() stopped";
}

void DA_BlockSequenceWorker::stop()
{
    threadShouldStop = true;

    int terminationCounter = 10;

    //wait until thread is stopped, or terminate it after 1 sec
    while(isRunning())
    {
        terminationCounter--;
        QThread::msleep(10);
        if(terminationCounter == 0)
            terminate();
    }
}

bool DA_BlockSequenceWorker::isStopping()
{
    return threadShouldStop;
}

void DA_BlockSequenceWorker::blockCaptured()
{
    if(blockRunLock.available() == 0)
        blockRunLock.release();
}

void DA_BlockSequenceWorker::onFinished()
{
    emit status("Block Sequence Finished");
    emit workerStopped();

    if(voltageBoundReached)
        emit voltageLimitReached();
}

void DA_BlockSequenceWorker::updateTimeLeft(DataAcquisitionWindow *daw, int nsize, double waitTime, int noCaptures)
{
    QString tunit;

    if(nsize == 0)
        resetTimeLeft(daw);

    // estimate time left (0.1 s per signal = 10 Hz)
    double timeLeft = nsize * (waitTime + 0.1 * double(noCaptures));

    if(timeLeft > 60.0)
    {
        tunit = "min";
        timeLeft = round(timeLeft / 6.0) / 10.0;
    }
    else
    {
        timeLeft = round(timeLeft);
        tunit = "s";
    }

    daw->psSettingsWidget->labelBlockSequenceTimeLeft->setText(QString("%0 %1")
                                                               .arg(timeLeft)
                                                               .arg(tunit));
    daw->psSettingsWidget->labelBlockSequenceSamplesLeft->setText(QString::number(nsize));
}


void DA_BlockSequenceWorker::resetTimeLeft(DataAcquisitionWindow *daw)
{
    daw->psSettingsWidget->labelBlockSequenceTimeLeft->setText("-");
    daw->psSettingsWidget->labelBlockSequenceSamplesLeft->setText("-");
}
