#ifndef FT_PARAMETERTABLE_H
#define FT_PARAMETERTABLE_H

#include <QWidget>
#include <QLayout>

class QLabel;
class QCheckBox;
class FitRun;
class SimRun;

class FT_ParameterTable : public QWidget
{
    Q_OBJECT

public:
    FT_ParameterTable(QWidget *parent = 0);

    void clear();
    void update(FitRun *fitRun);
    void update(SimRun *simRun);

private:

    double unitConversion_time;
    double unitConversion_pressure;

    QLabel *labelName;

    QLabel *labelHeatTransferModel;
    QLabel *labelGasMixture;
    QLabel *labelMaterial;
    QLabel *labelProcessPressure;
    QCheckBox *checkboxUseConduction;
    QCheckBox *checkboxUseEvaporation;
    QCheckBox *checkboxUseRadiation;

    QCheckBox *checkboxStartParticleDiameterFixed;
    QLabel *labelStartParticleDiameterValue;
    QLabel *labelStartParticleDiameterLower;
    QLabel *labelStartParticleDiameterUpper;
    QLabel *labelStartParticleDiameterDelta;

    QCheckBox *checkboxGasTemperatureFixed;
    QLabel *labelGasTemperatureValue;
    QLabel *labelGasTemperatureLower;
    QLabel *labelGasTemperatureUpper;
    QLabel *labelGasTemperatureDelta;

    QCheckBox *checkboxStartTemperatureFixed;
    QLabel *labelStartTemperatureValue;
    QLabel *labelStartTemperatureLower;
    QLabel *labelStartTemperatureUpper;
    QLabel *labelStartTemperatureDelta;

    QGridLayout *layoutNumericParameter;
    QLabel *labelMaxIterations;
    QLabel *labelODESolver;
    QLabel *labelODEStepSize;

    QGridLayout *layoutSimulationParameters;
    QLabel *labelStartTime;
    QLabel *labelSimulationLength;
    QLabel *labelInitialParticleTemp;
    QLabel *labelStepSize;

    QGridLayout *layoutSection;
    QLabel *labelSection;
    QLabel *labelSectionBeginTitle;
    QLabel *labelSectionEndTitle;
    QLabel *labelSectionStart;
    QLabel *labelSectionEnd;
};

#endif // FT_PARAMETERTABLE_H
