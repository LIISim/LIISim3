#include "multisignalaverage.h"

#include <QMutexLocker>
#include <limits>
#include <boost/multi_array.hpp>

#include "../../mrun.h"
#include "../../mpoint.h"
#include "../processingchain.h"

QList<Signal::SType> MultiSignalAverage::supportedSignalTypes = QList<Signal::SType>()
        << Signal::RAW << Signal::ABS << Signal::TEMPERATURE;
QString MultiSignalAverage::pluginName = "Multi-Signal Average";


MultiSignalAverage::MultiSignalAverage(ProcessingChain *parentChain) :   ProcessingPlugin(parentChain)
{
    shortDescription = "Calculate multi-signal average from valid signals ";

    executeSyncronized = true;
    preserveStdev = true;

    avgCalculated = false;
    avgSignalIdx = -1;

    startSignal = 1;
    endSignal = 1;
    if(mrun)
        endSignal = mrun->sizeAllMpoints();

    ProcessingPluginInput startSignalInput;
    startSignalInput.type = ProcessingPluginInput::INTEGER_FIELD;
    startSignalInput.value = startSignal;
    startSignalInput.minValue = startSignal;
    startSignalInput.maxValue = endSignal;
    startSignalInput.labelText = "Start signal: ";
    startSignalInput.identifier = "startsignal";
    startSignalInput.tooltip = "First signal number, which will be included in the average";

    inputs << startSignalInput;

    ProcessingPluginInput endSignalInput;
    endSignalInput.type = ProcessingPluginInput::INTEGER_FIELD;
    endSignalInput.value = endSignal;
    endSignalInput.minValue = startSignal;
    endSignalInput.maxValue = endSignal;
    endSignalInput.labelText = "End signal: ";
    endSignalInput.identifier = "endsignal";
    endSignalInput.tooltip = "Last signal number, which will be included in the average";

    inputs << endSignalInput;

    connect(mrun,
            SIGNAL(MRunDetailsChanged()),
            SLOT(onMRunChanged()));
}


/**
 * @brief MultiSignalAverage::onMRunChanged updates max values for end signal
 */
void MultiSignalAverage::onMRunChanged()
{
    inputs.getPluginInput("startsignal")->maxValue = mrun->sizeAllMpoints();
    inputs.getPluginInput("endsignal")->maxValue = mrun->sizeAllMpoints();
}


/**
 * @brief MultiSignalAverage::getName implementation of virtual function
 * @return name of plugin
 */
QString MultiSignalAverage::getName()
{
    return pluginName;
}


/**
 * @brief MultiSignalAverage::setInputs overrides virtual function
 * @param inputs list of Processing plugin inputs
 */
void MultiSignalAverage::setFromInputs()
{
    startSignal = inputs.getValue("startsignal").toInt();
    endSignal   = inputs.getValue("endsignal").toInt();
}


/**
 * @brief Initializes the average signals.
 * @details Determines for each channel of the processed signal type
 * a signal with
 */
void MultiSignalAverage::initializeAvgSignals()
{
    avgSignals.clear();
    counters.clear();

    if(mrun->sizeAllMpoints() >= endSignal)
        noMpoints = endSignal;
    else
        noMpoints = mrun->sizeAllMpoints();

    if(startSignal > endSignal)
        startSignal = endSignal;

    for(int p = 0; p < positionInChain; p++)
    {
        if(mrun->getProcessingChain(stype)->getPlug(p)->getName() == this->pluginName)
            noMpoints = 1;
    }

    QList<int> chids = mrun->channelIDs(stype);

    max_datasize = 0;

    // iterate through all channels
    for( int c = 0; c < chids.size(); c++)
    {
        double min_t  = std::numeric_limits<double>::max();
        double min_dt = std::numeric_limits<double>::max();
        double max_t  = std::numeric_limits<double>::min();

        for( int i = startSignal - 1; i < noMpoints; i++)
        {
            // include only signals which passed validation in previous step
            if(validAtPreviousStep(i))
            {
                // get signal of mpoint i with channelId c+1 at previous position in chain
                Signal s = processedSignalPreviousStep(i,chids[c]);

                if(s.start_time < min_t)
                    min_t = s.start_time;

                if(s.dt < min_dt)
                    min_dt = s.dt;

                if(s.maxTime() > max_t)
                    max_t = s.maxTime();
            }
        }

        int noDataPoints = int( (max_t - min_t) / min_dt + 1 );

        // determine maximum number of data for array scaling of "tempsignals"
        if(noDataPoints > max_datasize)
            max_datasize = noDataPoints;

        Signal s;
        avgSignals.push_back(s);
        avgSignals[c].start_time = min_t;
        avgSignals[c].dt = min_dt;
        avgSignals[c].data.fill(0.0, noDataPoints);
        avgSignals[c].stdev.fill(0.0, noDataPoints);
        avgSignals[c].type = stype;
        avgSignals[c].channelID = chids[c];

        QVector<int> countV;
        if(noDataPoints > 0)
            countV.fill(0, noDataPoints);
        counters.push_back(countV);
    }
}


/**
 * @brief MultiSignalAverage::processSignal implements virtual function
 * @param in  input signal
 * @param out output signal
 * @return true if the signal has passed validation
 */
bool MultiSignalAverage::processSignalImplementation(const Signal &in, Signal &out, int mpIdx)
{
    QMutexLocker lock(&variableMutex);

    bool valid = true;

    // calculate average signal (this is only done once and then avgCalculated is set to false)
    if(!avgCalculated)
    {
        avgCalculated = true;
        avgSignalIdx = mpIdx;

        initializeAvgSignals();

        QList<int> chids = mrun->channelIDs(stype);

        // array for temporarily saving data for covariance matrix calculation
        boost::multi_array<double,3> tempsignals(boost::extents[max_datasize][chids.size()][noMpoints]);

        // iterate through all channels
        for( int c = 0; c < chids.size(); c++)
        {
            double avgs_t_start = avgSignals[c].start_time;
            double avgs_dt      = avgSignals[c].dt;
            double avgs_data_sz = avgSignals[c].data.size();

            //process all measurement points  ...
            for( int i = startSignal - 1; i < noMpoints; i++)
            {
                // include only signals which passed validation in previous step
                if( validAtPreviousStep(i) )
                {
                    // get signal of mpoint i with channelId c+1 at previous position in chain
                    Signal s = processedSignalPreviousStep(i,chids[c]);

                    double cur_t;
                    for( int j = 0; j < avgs_data_sz; j++ )
                    {
                        cur_t = avgs_t_start + j * avgs_dt;

                        if(s.hasDataAt(cur_t))
                        {
                            avgSignals[c].data[j]  += s.at(cur_t);        // sum of data
                            avgSignals[c].stdev[j] += pow(s.at(cur_t),2); // sum of squares of data
                            counters[c][j]++;

                            // for each time step j,
                            // and every channel c,
                            //save the single shot values of mpoint i
                            if(stype == Signal::ABS || stype == Signal::RAW)
                                tempsignals[j][c][i] = s.at(cur_t);
                        }
                    }
                }
            }          
        }

        // calculate mean and standard deviation (iterate through all channels c)
        for( int c = 0; c < avgSignals.size(); c++ )
        {

            // iterate through data points
            for(int j = 0; j < avgSignals[c].data.size(); j++)
            {                
                double N = double(counters[c][j]);

                if( N > 0.0)
                {
                    double sum  = avgSignals[c].data[j];
                    double sum2 = avgSignals[c].stdev[j];

                    // calculate standard deviation
                    //  https://en.wikipedia.org/wiki/Algebraic_formula_for_the_variance
                    //  https://de.wikipedia.org/wiki/Verschiebungssatz_(Statistik)
                    // sigma^2 = 1/(N-1) * [sum of squares - sum^2/N]
                    avgSignals[c].stdev[j] = sqrt( 1/(N-1) * (sum2 - pow(sum,2)/N ));

                    // calculate mean
                    avgSignals[c].data[j] /= N;
                }
            }
        }

        // determine covariance matrix
        if(stype == Signal::ABS || stype == Signal::RAW)
        {
            if(stype == Signal::RAW)
                mrun->getPost(avgSignalIdx)->covar_list_raw.clear();
            else
                mrun->getPost(avgSignalIdx)->covar_list_abs.clear();

            int N;
            double sum, avgA, avgB, cov;

            // iterate through time steps
            for(int t = 0; t < max_datasize; t++)
            {
                // initialize covariance matrix with R=NxN (N = channel number)
                CovMatrix covar(chids.size());

                // iterate through channels ( calculate lower triangular matrix sigma(i,j))
                for(int i = 0; i < chids.size(); i++)
                    for(int j = 0; j <= i; j++)
                    {

                        // average value of channel A/B at time t
                        avgA = avgSignals[i].data[t];
                        avgB = avgSignals[j].data[t];

                        sum = 0.0;

                        // number of shots
                        N = tempsignals[t][i].size();

                        // iterate through single shots
                        for(int k = 0; k < N; k++)
                        {
                            sum += (tempsignals[t][i][k] - avgA)*(tempsignals[t][j][k] - avgB);
                        }

                        cov = (1.0 / N *  sum);

                        covar.set(i,j, cov);
                    }

                // store covariance matrix in mpoint with index avgSignalIdx
                if(stype == Signal::RAW)
                    mrun->getPost(avgSignalIdx)->covar_list_raw.append(covar);
                else
                    mrun->getPost(avgSignalIdx)->covar_list_abs.append(covar);
            }
        }

    }
    else
    {
        // comment the following if-clause out, to disable validation done by average calculation
        if( avgSignalIdx != mpIdx )
            valid = false;
    }

    // return average signal by channel ID
    int idx = in.channelID-1;

    if(avgSignals.size() > idx )
        out = avgSignals.at( in.channelID-1 );

    return valid;
}


QString MultiSignalAverage::getParameterPreview()
{
    return QString("(start=%0;end=%1)").arg(startSignal).arg(endSignal);
}


/**
 * @brief MultiSignalAverage::reset resets plugin state
 * @details clear results, forget that the average has been calculated
 */
void MultiSignalAverage::reset()
{
    QMutexLocker lock(&variableMutex);
    ProcessingPlugin::reset();
    avgSignals.clear();
    avgCalculated = false;
    avgSignalIdx = -1;
}
