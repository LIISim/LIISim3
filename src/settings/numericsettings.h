#ifndef NUMERICSETTINGS_H
#define NUMERICSETTINGS_H

#include "settingsbase.h"

/**  
 * @brief The NumericSettings class
 */
class NumericSettings : public SettingsBase
{
    Q_OBJECT
public:
    explicit NumericSettings(QObject* parent = 0);
    ~NumericSettings();

    inline int iterations() const {return m_iterations;}
    inline double stepSize() const {return m_stepSize;}
    inline int odeSolverStepSizeFactor() const {return m_odeSolver_stepSizeFactor;}
    inline double startTime() const { return m_startTime; }
    inline double simLength() const { return m_simLength; }

    inline int odeSolverIdx() { return m_odeSolver; }

    void setIterations(int iterations);
    void setOdeSolver(int idx);
    void setOdeSolverStepSizeFactor(int fac);

    void setStepSize(double stepSize);
    void setStartTime(double startTime);
    void setSimLength(double simLength);

    static int defaultFitMaxIterations();
    static double defaultStepSize();
    static int defaultODE();

    // default lambdas for levmar() can be overwritten
    double lambda_init = 0.1;       // 0.1
    double lambda_decrease = 0.5;   // 0.5
    double lambda_increase = 2.0;   // 2.0
    double lambda_scaling = 5.0;    // 5.0

private:
    void init();

    QString key_iterations;
    QString key_odeSolver;
    QString key_odeSolverSSF;
    QString key_stepSize;
    QString key_startTime;
    QString key_simLength;

    int m_iterations;
    int m_iterationsDefault;

    int m_odeSolver;
    int m_odeSolverDefault;

    int m_odeSolver_stepSizeFactor;

    double m_stepSize;
    double m_stepSizeDefault;
    double m_startTime;
    double m_simLength;
};

#endif // NUMERICSETTINGS_H
