#ifndef SIMRUN_H
#define SIMRUN_H

#include <QObject>

#include "../../signal/signal.h"
#include "../../settings/fitsettings.h"
#include "../../settings/numericsettings.h"
#include "../../settings/modelingsettings.h"

class SimRun : public QObject
{
    Q_OBJECT
public:
    explicit SimRun(QObject *parent = 0);
    ~SimRun();

    inline ModelingSettings* modelingSettings(){return m_modelingSettings;}
    inline NumericSettings* numericSettings(){return m_numericSettings;}
    inline FitSettings* fitSettings(){return m_fitSettings;}

    void setFitSettings(FitSettings* fs);
    void setNumericSettings(NumericSettings* ns);
    void setModelingSettings(ModelingSettings* ms);

    void simulate();
    Signal getEvaporationCurve();
    Signal getConductionCurve();
    Signal getRadiationCurve();

    int mSimcounter;

    Signal sim_trace;

private:
    static int simcounter;

    ModelingSettings* m_modelingSettings;
    NumericSettings* m_numericSettings;
    FitSettings* m_fitSettings;

signals:

public slots:

};

#endif // SIMRUN_H
