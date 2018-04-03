#include "simrun.h"

#include "../../core.h"
#include "../../calculations/numeric.h"


int SimRun::simcounter = 1;

SimRun::SimRun(QObject *parent) : QObject(parent)
{

    m_modelingSettings = new ModelingSettings(this);
    m_numericSettings = new NumericSettings(this);
    m_fitSettings = new FitSettings(this);

    m_modelingSettings->copyFrom(Core::instance()->modelingSettings);

    mSimcounter = simcounter;
    simcounter++;
}


SimRun::~SimRun() {}


void SimRun::setFitSettings(FitSettings *fs)
{
    if(m_fitSettings)
        delete m_fitSettings;
    m_fitSettings = fs;
    m_fitSettings->setParent(this);
}


void SimRun::setNumericSettings(NumericSettings *ns)
{
    if(m_numericSettings)
        delete m_numericSettings;
    m_numericSettings = ns;
    m_numericSettings->setParent(this);
}


void SimRun::setModelingSettings(ModelingSettings *ms)
{
    if(m_modelingSettings)
        delete m_modelingSettings;
    m_modelingSettings = ms;
    m_modelingSettings->setParent(this);
}


/**
 * @brief SimRun::simulate simulate temperature trace with start conditions
 */
void SimRun::simulate()
{
    HeatTransferModel* htm = m_modelingSettings->heatTransferModel();

    // parameters
    double dp       = m_fitSettings->fitParameters().at(0).value();
    double gas_temp = m_fitSettings->fitParameters().at(1).value();
    double T_peak   = m_fitSettings->fitParameters().at(2).value();

    htm->setProcessConditions(m_modelingSettings->processPressure(), gas_temp);

    //calculate how many data points are needed with given 'dt' and simulation length
    int dataPoints = ceil(m_numericSettings->simLength()
                    / m_numericSettings->stepSize()) + 1;


    //qDebug() << "SiMRun: stepSizeFac: " << m_numericSettings->odeSolverStepSizeFactor();

    // not used anymore here: m_modelingSettings->initTemperature()

    sim_trace = Numeric::solveODE(T_peak,
                                    dp,
                                    *htm,
                                    dataPoints,                                  
                                    m_numericSettings->stepSize(),
                                    m_numericSettings);

    sim_trace.start_time = m_numericSettings->startTime();
}


Signal SimRun::getEvaporationCurve()
{
    HeatTransferModel* htm = m_modelingSettings->heatTransferModel();

    Signal htr;
    htr.start_time  = sim_trace.start_time;
    htr.dt          = sim_trace.dt;

    for(int i = 0; i < sim_trace.data.size(); i++)
    {
        htr.data.append(htm->calculateEvaporation(sim_trace.data.at(i),
                                                  sim_trace.dataDiameter.at(i)));
    }

    return htr;
}


Signal SimRun::getConductionCurve()
{
    HeatTransferModel* htm = m_modelingSettings->heatTransferModel();

    Signal htr;
    htr.start_time  = sim_trace.start_time;
    htr.dt          = sim_trace.dt;

    for(int i = 0; i < sim_trace.data.size(); i++)
    {
        htr.data.append(htm->calculateConduction(sim_trace.data.at(i),
                                                 sim_trace.dataDiameter.at(i)));
    }

    return htr;
}


Signal SimRun::getRadiationCurve()
{
    HeatTransferModel* htm = m_modelingSettings->heatTransferModel();

    Signal htr;
    htr.start_time  = sim_trace.start_time;
    htr.dt          = sim_trace.dt;

    for(int i = 0; i < sim_trace.data.size(); i++)
    {
        htr.data.append(htm->calculateRadiation(sim_trace.data.at(i),
                                                sim_trace.dataDiameter.at(i)));
    }

    return htr;
}
