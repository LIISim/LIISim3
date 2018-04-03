#ifndef FITDATA_H
#define FITDATA_H


#include <QList>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "fititerationresult.h"
#include "../../signal/signal.h"
#include "../../signal/mrun.h"

class FitRun;

/**
 * @brief The FitData class
 */
class FitData
{
public:

    // default data fit initialization
    explicit FitData();

    // signal fit initialization
    explicit FitData(int mrun_id,
                      int mpoint_index = 0,
                      Signal::SType stype = Signal::TEMPERATURE,
                      int channel_id = 1, bool signalSection = false,
                      double sectionStart = 0.0, double sectionEnd = 0.0);

    ~FitData();

    QVector<double> xdata;
    QVector<double> ydata;
    QVector<double> stdev;
    QVector<int> bandwidths; // for temperature bandwidth fit

    QList<QVector<double>> ydatalist; // each element should have the same size as xdata

    LIISettings mrun_LIISettings; // only used from spectrum test


    void initData(QVector<double> n_xdata, QVector<double> n_ydata, QVector<double> n_stdev);
    void initBandwidth(QVector<int> n_bandwidths);

    Signal dataSignal();
    Signal modeledSignal(int i = -1);

    Signal getConductionCurve(int iteration);
    Signal getEvaporationCurve(int iteration);
    Signal getRadiationCurve(int iteration);

    void clearResults();

    void addIterationResult(const FitIterationResult &res);
    inline int iterationResCount() const{ return m_iterationResults.size(); }

    QList<FitIterationResult> iterationResultList();
    FitIterationResult iterationResult(int i);
    FitIterationResult iterationResultLast();

    QString toString();

    inline int mrunID()const {                  return m_in_mrun_id;}
    inline int temperatureChannelID() const {   return m_in_ch_id;}
    inline int mpoint()const {                  return m_in_mp_idx;}
    inline FitRun* fitRun()const {              return m_fitrun;}

    void writeToXML(QXmlStreamWriter &w);
    bool readFromXml(QXmlStreamReader &r);

    MRun* mrun();

    void setFitRun(FitRun* fr){m_fitrun = fr;}

    QVector<double> getParameterCurve(int param);

private:

    QList<FitIterationResult> m_iterationResults;

    int m_in_mrun_id;
    int m_in_mp_idx;
    int m_in_ch_id;
    Signal::SType m_in_stype;

    Signal m_dataSignal;

    // peak value of data signal
    double m_dataSignal_tmax;

    FitRun* m_fitrun;

    void initDataSignal();

    bool m_signalSection;
    double m_sectionStart;
    double m_sectionEnd;

};

#endif // FITDATA_H
