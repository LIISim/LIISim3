#include "fitdata.h"

#include "../../core.h"
#include "../../signal/mrungroup.h"
#include "fitrun.h"
#include "../../calculations/numeric.h"


/**
 * @brief FitData::FitData default data fitting without calling initDataSignal();
 */
FitData::FitData()
{
    m_in_mrun_id = -1;
    m_in_mp_idx = 0;
    m_in_ch_id = 1;
    m_in_stype = Signal::TEMPERATURE;
    m_fitrun = 0;
}


/**
 * @brief FitData::FitData signal data fitting
 * @param mrun_id
 * @param mpoint_index
 * @param stype
 * @param channel_id
 */
FitData::FitData(int mrun_id,
                 int mpoint_index,
                 Signal::SType stype,
                 int channel_id,
                 bool signalSection,
                 double sectionStart,
                 double sectionEnd)
{
    m_in_mrun_id = mrun_id;
    m_in_mp_idx = mpoint_index;
    m_in_ch_id = channel_id;
    m_in_stype = stype;
    m_fitrun = 0;

    m_signalSection = signalSection;
    m_sectionStart = sectionStart;
    m_sectionEnd = sectionEnd;

    initDataSignal();
}


FitData::~FitData()
{
}


/**
 * @brief FitData::initData clears data and fills container with new values
 * @param xdata
 * @param ydata
 * @param stdev
 */
void FitData::initData(QVector<double> n_xdata, QVector<double> n_ydata, QVector<double> n_stdev)
{
    xdata.clear();
    ydata.clear();
    stdev.clear();

    xdata = n_xdata;
    ydata = n_ydata;
    stdev = n_stdev;
}


/**
 * @brief FitData::initBandwidth clears and fills container for bandpass integration fit (FitRun::TEMP)
 * @param bandwidths
 */
void FitData::initBandwidth(QVector<int> n_bandwidths)
{
    bandwidths.clear();

    bandwidths = n_bandwidths;
}


/**
 * @brief FitData::initDataSignal for signal data fitting
 */
void FitData::initDataSignal()
{
    try
    {
        MRun* r = Core::instance()->dataModel()->mrun(m_in_mrun_id);
        if(!r)
            throw LIISimException(QString("FitData: invalid MRun-ID (%0)").arg(m_in_mrun_id));

        MPoint* mp = r->getPost(m_in_mp_idx);
        if(!mp)
            throw LIISimException(QString("FitData: invalid MPoint-Index (%0)").arg(m_in_mp_idx));

        if(m_signalSection)
        {
            m_dataSignal = mp->getSignal(m_in_ch_id, m_in_stype).getSection(m_sectionStart, m_sectionEnd);
            if(mp->getSignal(m_in_ch_id, m_in_stype).hasDataAt(m_sectionStart))
                m_dataSignal.start_time = m_sectionStart;
        }
        else
            m_dataSignal = mp->getSignal(m_in_ch_id, m_in_stype);
        m_dataSignal_tmax = m_dataSignal.getMaxValue();

        //qDebug() << "FitData::initData";

        // this is redundant, in future please use only xdata/ydata/stdev
        for(int i = 0; i < m_dataSignal.size(); i++)
        {
            xdata.append(m_dataSignal.time(i));
            ydata.append(m_dataSignal.data.at(i));
            //stdev
        }
    }
    catch(LIISimException e)
    {
        qDebug() << "FitData::initDataSignal() Error: " << e.what();
        m_dataSignal.channelID = m_in_ch_id;
        m_dataSignal.type = m_in_stype;
        m_dataSignal_tmax = 0.0;
    }
}


/**
 * @brief FitData::modeledSignal get modeled signal of fit iteration i (this function is only used for visualization (see FitResultWidget))
 * @param i index of fit iteration (default value = -1: returns fitted signal of last iteration
 * @return Signal
 */
Signal FitData::modeledSignal(int i)
{
    Signal out;

    if(i == -1)
        i = iterationResCount()-1;

    if(i < 0 || i > iterationResCount() - 1)
        return out;

    HeatTransferModel* htm = m_fitrun->modelingSettings(-1)->heatTransferModel();

    // res[2]: particle size
    // res[4]: gas temperature
    // res[6]: start temperature (peak)

    FitIterationResult res = iterationResult(i);

    if(res.size() < 2)
    {
        MSG_ERR("FitData::modelSignal: invalid iteration result");
        return out;
    }

    //qDebug() << "FitData::modeledSignal: " << res[2] << " - " << res[4] << " - " << res[6];

    htm->setProcessConditions(m_fitrun->modelingSettings(-1)->processPressure(), res[4]);

    Signal model = Numeric::solveODE(res[6],
                                        res[2],
                                        *htm,                                        
                                        m_dataSignal.size(),
                                        m_dataSignal.dt,
                                        m_fitrun->numericSettings());

    model.start_time = m_dataSignal.start_time;

    return model;
}


Signal FitData::getEvaporationCurve(int iteration)
{    
    Signal source = modeledSignal(iteration);

    HeatTransferModel *htm = m_fitrun->modelingSettings(-1)->heatTransferModel();

    Signal evaporation;
    evaporation.start_time  = source.start_time;
    evaporation.dt          = source.dt;

    for(int i = 0; i < source.data.size(); i++)
    {
        evaporation.data.append(htm->calculateEvaporation(source.data.at(i),
                                                          source.dataDiameter.at(i)));
    }

    return evaporation;
}


Signal FitData::getConductionCurve(int iteration)
{
    Signal source = modeledSignal(iteration);
    HeatTransferModel *htm = m_fitrun->modelingSettings(-1)->heatTransferModel();

    Signal conduction;
    conduction.start_time = source.start_time;
    conduction.dt         = source.dt;

    for(int i = 0; i < source.data.size(); i++)
    {
        conduction.data.append(htm->calculateConduction(source.data.at(i),
                                                        source.dataDiameter.at(i)));
    }

    return conduction;
}


Signal FitData::getRadiationCurve(int iteration)
{    
    Signal source = modeledSignal(iteration);

    HeatTransferModel *htm = m_fitrun->modelingSettings(-1)->heatTransferModel();

    Signal radiation;
    radiation.start_time = source.start_time;
    radiation.dt = source.dt;

    for(int i = 0; i < source.data.size(); i++)
    {
        radiation.data.append(htm->calculateRadiation(source.data.at(i),
                                                      source.dataDiameter.at(i)));
    }

    return radiation;
}


/**
 * @brief FitData::dataSignal this function is only used for visualization (see FitResultWidget)
 * @return
 */
Signal FitData::dataSignal()
{
    return m_dataSignal;
}


void FitData::clearResults()
{
    m_iterationResults.clear();
}


QString FitData::toString()
{
    QString runname ="NONEXISTENT-RUN!";
    MRun* r = Core::instance()->dataModel()->mrun(m_in_mrun_id);
    if(r) runname = r->getName();

    QString res = QString("FitData: %0 mp: %1 ch: %2 %3 num iteration results: %4\n")
            .arg(runname)
            .arg(m_in_mp_idx)
            .arg(m_in_ch_id)
            .arg(Signal::stypeToString(m_in_stype))
            .arg(m_iterationResults.size());
    for(int i = 0; i < m_iterationResults.size(); i++)
    {
        res.append(QString("\t%0\n").arg(m_iterationResults[i].toString()));
    }
    return res;
}





void FitData::addIterationResult(const FitIterationResult& res)
{
    m_iterationResults.append(res);
}


QList<FitIterationResult> FitData::iterationResultList()
{
    return m_iterationResults;
}


FitIterationResult FitData::iterationResult(int i)
{
    return m_iterationResults.value(i, FitIterationResult(0));
}


FitIterationResult FitData::iterationResultLast()
{
    return m_iterationResults.last();
}


void FitData::writeToXML(QXmlStreamWriter &w)
{
    MRun* mr = Core::instance()->dataModel()->mrun(m_in_mrun_id);
    if(!mr)
    {
        qDebug() << "FitData::writeToXML: invalid runid: "<<m_in_mrun_id;
        return;
    }
    MRunGroup* gr = Core::instance()->dataModel()->group( mr->parentItem()->id());

    w.writeStartElement("FitData");

    w.writeAttribute("gname",gr->title());
    w.writeAttribute("runname",mr->getName());
    w.writeAttribute("mp_idx",QString::number(m_in_mp_idx));
    w.writeAttribute("ch_id",QString::number(m_in_ch_id));
    w.writeAttribute("stype",Signal::stypeToString(m_in_stype));

    for(int i = 0; i < m_iterationResults.size();i++)
        m_iterationResults[i].writeToXML(w);

    w.writeEndElement(); // FitData
}

bool FitData::readFromXml(QXmlStreamReader &r)
{

    if(r.tokenType() != QXmlStreamReader::StartElement &&  r.name() == "FitData")
        return false;

    QXmlStreamAttributes a = r.attributes();
    QString gname = a.value("gname").toString();
    QString runname = a.value("runname").toString();

    MRunGroup* gr = Core::instance()->dataModel()->findGroup(gname);
    if(!gr)
    {
        qDebug() << "FitData::readFromXml: invalid groupname: " << gname;
        return false;
    }
    MRun* mr = gr->findMRun(runname);

    if(!mr)
    {
        qDebug() << "FitData::readFromXml: invalid runname: " << runname;
        return false;
    }

    m_in_mrun_id = mr->id();
    m_in_mp_idx = a.value("mp_idx").toInt();
    m_in_ch_id = a.value("ch_id").toInt();
    m_in_stype = Signal::stypeFromString(a.value("stype").toString());

    while( !(r.name() == "FitData" &&  r.tokenType() == QXmlStreamReader::EndElement))
    {
        if(r.tokenType() == QXmlStreamReader::StartElement)
        {
            if(r.name() == "FitIterationResult")
            {
                FitIterationResult res(0);
                res.readFromXml(r);
                m_iterationResults << res;
            }
        }
        r.readNext();
    }

    return true;
}

/**
 * @brief FitData::mrun returns the measurement run associated with
 * the data signal
 * @return MRun or if MRun does not exeist: NULL
 */
MRun* FitData::mrun()
{
    return Core::instance()->dataModel()->mrun(m_in_mrun_id);
}


/**
 * @brief FitData::getParameterCurve
 * @param param
 * @return
 */
QVector<double> FitData::getParameterCurve(int param)
{
    QVector<double> curve;
    if(m_iterationResults.isEmpty())
        return curve;

    if(param < 0 || param >= m_iterationResults[0].count())
        return curve;

    for(int i = 0; i < m_iterationResults.size(); i++)
        curve << m_iterationResults[i][param];

    return curve;
}
