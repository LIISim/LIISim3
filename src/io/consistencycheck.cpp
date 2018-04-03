#include "consistencycheck.h"

bool ConsistencyCheck::check(const SignalIORequest &rq)
{
    irregularityDetected = false;
    signalCountError = false;
    channelCountError = false;
    noChannels = 0;
    noSignals = 0;

    int channelMax = INT_MIN;
    int channelMin = INT_MAX;

    if(rq.itype == SignalIOType::CUSTOM)
    {
        QList<int> channelNumbers;

        QMap<int, QList<int>*> signalNumbers;

        for(SignalFileInfo file : rq.flist)
        {
            if(!channelNumbers.contains(file.channelId))
                channelNumbers.push_back(file.channelId);

            if(file.channelId < channelMin)
                channelMin = file.channelId;

            if(file.channelId > channelMax)
                channelMax = file.channelId;
        }

        std::sort(channelNumbers.begin(), channelNumbers.end());

        //channels ok
        if(channelMax == channelNumbers.last())
        {
            for(int channel : channelNumbers)
                signalNumbers.insert(channel, new QList<int>);

            int doubleSignalIDCount = 0;

            for(SignalFileInfo file : rq.flist)
            {
                if(!signalNumbers.value(file.channelId)->contains(file.signalId))
                    signalNumbers.value(file.channelId)->push_back(file.signalId);
                else
                    doubleSignalIDCount++;
            }

            if(doubleSignalIDCount != 0)
            {
                irregularityDetected = true;
                signalCountError = true;

                MSG_ASYNC(QString("Consistency Check - Irregularity detected in %0: Multiple occurence of the same signal count.").arg(rq.runname), LIISimMessageType::WARNING);
            }

            int signalCount = signalNumbers.value(channelNumbers.at(0))->size();
            int firstSignalCount = signalNumbers.value(channelNumbers.at(0))->size();

            int signalCountDiffers = 0;

            for(int i = 1; i < channelNumbers.size(); i++)
            {
                if(signalCount < signalNumbers.value(channelNumbers.at(i))->size())
                    signalCount = signalNumbers.value(channelNumbers.at(i))->size();

                if(firstSignalCount != signalNumbers.value(channelNumbers.at(i))->size())
                    signalCountDiffers++;
            }

            if(signalCountDiffers != 0)
            {
                irregularityDetected = true;
                signalCountError = true;

                MSG_ASYNC(QString("Consistency Check - Irregularity detected in %0: Signal count differs between channels.").arg(rq.runname), LIISimMessageType::WARNING);
            }

            for(int i = 1; i < channelNumbers.size(); i++)
            {
                QList<int> *temp = signalNumbers.take(channelNumbers.at(i));
                delete temp;
            }

            noChannels = channelMax;
            noSignals = signalCount;
        }
        //this should never happen
        else if(channelMax < channelNumbers.last())
        {

        }
        //seems like there are channels skipped, nothing we could solve
        else if(channelMax > channelNumbers.last())
        {
            irregularityDetected = true;

            channelCountError = true;

            MSG_ASYNC(QString("Consistency Check - Irregularity detected in %0: There are skipped channels.").arg(rq.runname), LIISimMessageType::WARNING);
        }

        if(channelMax > 4 || channelNumbers.size() > 4)
        {
            irregularityDetected = true;

            MSG_ASYNC(QString("Consistency Check - Irregularity detected in %0: Channel count exceeds processable number.").arg(rq.runname), LIISimMessageType::WARNING);
        }
    }

    return irregularityDetected;
}


void ConsistencyCheck::prepareRun(MRun *run)
{
    if(!run)
        return;

    if(signalCountError)
    {
        for(int i = 0; i < noChannels; i++)
            run->getCreatePre(i);
    }
}
