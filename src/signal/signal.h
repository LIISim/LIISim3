#ifndef SIGNAL_H
#define SIGNAL_H

#include "../general/channel.h"
#include <QVector>
#include "../../calculations/fit/fititerationresult.h"

/**
 * @brief The Signal class
 * @ingroup Hierachical-Data-Model
 */
class Signal
{
public:
    Signal();

    Signal(int size);
    Signal(int size, double dt);
    Signal(int size, double dt, double start_time);

    double time(int index);

    double getMaxValue();
    int    getMaxIndex() const;

    /**
     * @brief size signal size
     * @return number of datapoints
     */
    inline int size() const { return data.size();}
    Signal getSection(int start, int end=0);
    Signal getSection(double start, double end);
    double calcRangeAverage(double start, double end);
    double calcRangeMin(double start, double end);
    double calcRangeMax(double start, double end);
    double getTimeAtMaxSignalRange(double start, double end);
    double getTimeAtMinSignalRange(double start, double end);
    QPair<double, double> calcRangeAverageStdev(double start, double end);

    double at(double time);
    int indexAt(double time);
    bool hasDataAt(double time) const;
    double maxTime() const;
    void resample( double newStartTime, double newDt, int noSamples );

    enum SType { RAW, ABS, TEMPERATURE }; // signal type
    static QString stypeToString(Signal::SType stype);
    static SType stypeFromString(QString str);

    SType type;                  // signal type    
    int channelID;              // store channel id
    double dt;                  // time between datapoints [s]
    double start_time;          // [s]

    /** @brief vector of data points */
    QVector<double> data;        // datapoints
    QVector<double> stdev;       // standard deviation container for Plugin:MultiSignalAverage

    /** @brief vector for storing variation of particle diameter through signal duration
     *  due to evaporation the particle diameter varies, thus every temperature signal
     * should have a corresponding particle diameter trace */
    QVector<double> dataDiameter;

    /** @brief temperature planck fit: iteration list is stored for every time step */
    QList<QList<FitIterationResult>> fitData;   // fit results

    // TODO: put this meta fit information to MRun class
    QString fitMaterial;                        // save material used for temperature fitting
    QList<bool> fitActiveChannels;              // active channels used for temperature fit
};

#endif // SIGNAL_H
