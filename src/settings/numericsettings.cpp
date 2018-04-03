#include "numericsettings.h"
#include "../logging/msghandlerbase.h"

#include "../calculations/numeric.h"

NumericSettings::NumericSettings(QObject *parent) : SettingsBase(parent)
{
    MSG_DETAIL_1("init NumericSettings");
    groupName = "NumericSettings";

    key_iterations      = "iterations";
    key_odeSolver       = "odeSolver";
    key_odeSolverSSF    = "odeSolverStepSizeFactor";
    key_stepSize        = "stepSize";
    key_startTime       = "startTime";
    key_simLength       = "simLength";

    m_iterationsDefault     = defaultFitMaxIterations();
    m_iterations            = m_iterationsDefault;

    m_stepSizeDefault       = defaultStepSize();
    m_stepSize              = m_stepSizeDefault;

    m_odeSolverDefault      = (int)Numeric::defaultODESolver();
    m_odeSolver             = m_odeSolverDefault;

    m_odeSolver_stepSizeFactor  = 1;

    init();
}

NumericSettings::~NumericSettings() {}

/**
 * @brief NumericSettings::defaultFitMaxIterations
 * @return default value for number of fit iterations
 */
int NumericSettings::defaultFitMaxIterations()
{
    return Numeric::iterationsDefault;
}


/**
 * @brief NumericSettings::defaultStepSize
 * @return default value for fit stepsize [s]
 */
double NumericSettings::defaultStepSize()
{
    return Numeric::stepSizeDefault;
}


void NumericSettings::init()
{
    // check if keys exist, if not set default values
    if(!settings.contains(key_iterations))
        settings.insert(key_iterations, m_iterationsDefault);
    if(!settings.contains(key_odeSolver))
        settings.insert(key_odeSolver, m_odeSolverDefault);
    if(!settings.contains(key_odeSolver))
        settings.insert(key_odeSolverSSF, 1);
    if(!settings.contains(key_stepSize))
        settings.insert(key_stepSize, m_stepSizeDefault);
    if(!settings.contains(key_startTime))
        settings.insert(key_startTime, 0.0f);
    if(!settings.contains(key_simLength))
        settings.insert(key_simLength, 2000.0f);
}


void NumericSettings::setIterations(int iterations)
{
    m_iterations = iterations;
    settings.insert(key_iterations, m_iterations);
    emit settingsChanged();
}


void NumericSettings::setOdeSolver(int idx)
{
    m_odeSolver = idx;
    settings.insert(key_odeSolver, m_odeSolver);
    emit settingsChanged();
}


void NumericSettings::setOdeSolverStepSizeFactor(int fac)
{
    m_odeSolver_stepSizeFactor = fac;
    settings.insert(key_odeSolverSSF, m_odeSolver_stepSizeFactor);
    emit settingsChanged();
}


void NumericSettings::setStepSize(double stepSize)
{
    m_stepSize = stepSize;
    settings.insert(key_stepSize,m_stepSize);
    emit settingsChanged();
}


void NumericSettings::setStartTime(double startTime)
{
    m_startTime = startTime;
    settings.insert(key_startTime, m_startTime);
    emit settingsChanged();
}


void NumericSettings::setSimLength(double simLength)
{
    m_simLength = simLength;
    settings.insert(key_simLength, m_simLength);
    emit settingsChanged();
}
