#include "ft_parametertable.h"

#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include "core.h"

#include "../../calculations/fit/fitrun.h"
#include "../../calculations/fit/simrun.h"
#include "../../calculations/numeric.h"


/**
 * @brief FT_ParameterTable::FT_ParameterTable bottom left box
 * @param parent
 */
FT_ParameterTable::FT_ParameterTable(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    // unit conversion (internal state to GUI appearance)
    unitConversion_time     = 1E9; // s to ns
    unitConversion_pressure = 1E-2; // Pa to mbar

    //name

    QGridLayout *layoutName = new QGridLayout;
    layoutName->setMargin(0);
    QLabel *labelNameHeader = new QLabel("Fit Name:", this);
    labelNameHeader->setStyleSheet("font-weight: bold");

    labelName = new QLabel("-", this);

    layoutName->addWidget(labelNameHeader, 0, 0, 1, 1);
    layoutName->addWidget(labelName, 0, 1, 1, 1, Qt::AlignLeft);

    //modeling settings

    QGridLayout *layoutModelingSettings = new QGridLayout;
    layoutModelingSettings->setMargin(0);

    QLabel *labelModelingSettingsHeader = new QLabel("Modeling Settings", this);
    labelModelingSettingsHeader->setStyleSheet("font-weight: bold;");

    // left column
    layoutModelingSettings->addWidget(labelModelingSettingsHeader);
    layoutModelingSettings->addWidget(new QLabel("Heat Transfer Model:", this));
    layoutModelingSettings->addWidget(new QLabel("Gas Mixture:", this));
    layoutModelingSettings->addWidget(new QLabel("Material:", this));
    layoutModelingSettings->addWidget(new QLabel("Process pressure [mbar]:", this));


    labelHeatTransferModel = new QLabel("-", this);
    labelGasMixture = new QLabel("-", this);
    labelMaterial = new QLabel("-", this);
    labelProcessPressure = new QLabel("-", this);

    layoutModelingSettings->addWidget(labelHeatTransferModel, 1, 1, 1, 1);
    layoutModelingSettings->addWidget(labelGasMixture, 2, 1, 1, 1);
    layoutModelingSettings->addWidget(labelMaterial, 3, 1, 1, 1);
    layoutModelingSettings->addWidget(labelProcessPressure, 4, 1, 1, 1);

    checkboxUseEvaporation = new QCheckBox("Evaporation", this);
    checkboxUseEvaporation->setEnabled(false);
    checkboxUseConduction = new QCheckBox("Conduction", this);
    checkboxUseConduction->setEnabled(false);    
    checkboxUseRadiation = new QCheckBox("Radiation", this);
    checkboxUseRadiation->setEnabled(false);

    layoutModelingSettings->addWidget(checkboxUseEvaporation, 1, 2, 1, 1);
    layoutModelingSettings->addWidget(checkboxUseConduction, 2, 2, 1, 1);
    layoutModelingSettings->addWidget(checkboxUseRadiation, 3, 2, 1, 1);

    //fit parameter
    QGridLayout *layoutFitParameter = new QGridLayout;
    layoutFitParameter->setMargin(0);

    QLabel *labelFitParameterHeader = new QLabel("Initial Fit Parameter", this);
    labelFitParameterHeader->setStyleSheet("font-weight : bold;");

    layoutFitParameter->addWidget(labelFitParameterHeader, 0, 0, 1, 1);
    layoutFitParameter->addWidget(new QLabel("Particle Diameter [nm]:", this));
    layoutFitParameter->addWidget(new QLabel("Gas Temperature [K]:", this));
    layoutFitParameter->addWidget(new QLabel("Peak Temperature [K]:", this));

    QLabel *tmp = new QLabel("Fixed", this);
    layoutFitParameter->addWidget(tmp, 0, 1, 1, 1);
    layoutFitParameter->addWidget(new QLabel("Value", this), 0, 2, 1, 1);
    layoutFitParameter->addWidget(new QLabel("Lower Boundary", this), 0, 3, 1, 1);
    layoutFitParameter->addWidget(new QLabel("Upper Boundary", this), 0, 4, 1, 1);
    layoutFitParameter->addWidget(new QLabel("Max Delta", this), 0, 5, 1, 1);

    checkboxStartParticleDiameterFixed = new QCheckBox(this);
    checkboxStartParticleDiameterFixed->setDisabled(true);
    labelStartParticleDiameterValue = new QLabel("-", this);
    labelStartParticleDiameterLower = new QLabel("-", this);
    labelStartParticleDiameterUpper = new QLabel("-", this);
    labelStartParticleDiameterDelta = new QLabel("-", this);

    layoutFitParameter->addWidget(checkboxStartParticleDiameterFixed, 1, 1, 1, 1);
    layoutFitParameter->setAlignment(checkboxStartParticleDiameterFixed, Qt::AlignHCenter);
    layoutFitParameter->addWidget(labelStartParticleDiameterValue, 1, 2, 1, 1);
    layoutFitParameter->addWidget(labelStartParticleDiameterLower, 1, 3, 1, 1);
    layoutFitParameter->addWidget(labelStartParticleDiameterUpper, 1, 4, 1, 1);
    layoutFitParameter->addWidget(labelStartParticleDiameterDelta, 1, 5, 1, 1);

    checkboxGasTemperatureFixed = new QCheckBox(this);
    checkboxGasTemperatureFixed->setDisabled(true);
    labelGasTemperatureValue = new QLabel("-", this);
    labelGasTemperatureLower = new QLabel("-", this);
    labelGasTemperatureUpper = new QLabel("-", this);
    labelGasTemperatureDelta = new QLabel("-", this);

    layoutFitParameter->addWidget(checkboxGasTemperatureFixed, 2, 1, 1, 1, Qt::AlignHCenter);
    layoutFitParameter->addWidget(labelGasTemperatureValue, 2, 2, 1, 1);
    layoutFitParameter->addWidget(labelGasTemperatureLower, 2, 3, 1, 1);
    layoutFitParameter->addWidget(labelGasTemperatureUpper, 2, 4, 1, 1);
    layoutFitParameter->addWidget(labelGasTemperatureDelta, 2, 5, 1, 1);

    checkboxStartTemperatureFixed = new QCheckBox(this);
    checkboxStartTemperatureFixed->setDisabled(true);
    labelStartTemperatureValue = new QLabel("-", this);
    labelStartTemperatureLower = new QLabel("-", this);
    labelStartTemperatureUpper = new QLabel("-", this);
    labelStartTemperatureDelta = new QLabel("-", this);

    layoutFitParameter->addWidget(checkboxStartTemperatureFixed, 3, 1, 1, 1, Qt::AlignHCenter);
    layoutFitParameter->addWidget(labelStartTemperatureValue, 3, 2, 1, 1);
    layoutFitParameter->addWidget(labelStartTemperatureLower, 3, 3, 1, 1);
    layoutFitParameter->addWidget(labelStartTemperatureUpper, 3, 4, 1, 1);
    layoutFitParameter->addWidget(labelStartTemperatureDelta, 3, 5, 1, 1);


    //numeric parameter

    layoutNumericParameter = new QGridLayout;
    layoutNumericParameter->setMargin(0);

    QLabel *labelNumericParameter = new QLabel("Numeric Parameter", this);
    labelNumericParameter->setStyleSheet("font-weight : bold;");

    layoutNumericParameter->addWidget(labelNumericParameter, 0, 0, 1, 1);
    layoutNumericParameter->addWidget(new QLabel("Max Iterations:", this));
    layoutNumericParameter->addWidget(new QLabel("ODE Solver:", this));
    layoutNumericParameter->addWidget(new QLabel("ODE Step Size:", this));

    labelMaxIterations = new QLabel("-", this);
    labelODESolver = new QLabel("-", this);
    labelODEStepSize = new QLabel("-", this);

    layoutNumericParameter->addWidget(labelMaxIterations, 1, 1, 1, 1);
    layoutNumericParameter->addWidget(labelODESolver, 2, 1, 1, 1);
    layoutNumericParameter->addWidget(labelODEStepSize, 3, 1, 1, 1);


    // simulation only parameters

    layoutSimulationParameters = new QGridLayout;
    layoutSimulationParameters->setMargin(0);

    QLabel *labelSimulationParameters = new QLabel("Simulation Only Parameters", this);
    labelSimulationParameters->setStyleSheet("font-weight : bold;");

    layoutSimulationParameters->addWidget(labelSimulationParameters);
    layoutSimulationParameters->addWidget(new QLabel("Start Time [ns]:", this));
    layoutSimulationParameters->addWidget(new QLabel("Simulation length [ns]:", this));    
    layoutSimulationParameters->addWidget(new QLabel("Step Size [ns]:", this));

    labelStartTime = new QLabel("-", this);
    labelSimulationLength = new QLabel("-", this);    
    labelStepSize = new QLabel("-", this);

    layoutSimulationParameters->addWidget(labelStartTime, 1, 1, 1, 1);
    layoutSimulationParameters->addWidget(labelSimulationLength, 2, 1, 1, 1);    
    layoutSimulationParameters->addWidget(labelStepSize, 3, 1, 1, 1);

    // section/range

    layoutSection = new QGridLayout;
    layoutSection->setMargin(0);

    labelSection = new QLabel("Range", this);
    labelSection->setStyleSheet("font-weight : bold;");

    labelSectionBeginTitle = new QLabel("Begin [ns]:", this);
    labelSectionEndTitle = new QLabel("End [ns]:", this);

    layoutSection->addWidget(labelSection);
    layoutSection->addWidget(labelSectionBeginTitle);
    layoutSection->addWidget(labelSectionEndTitle);

    labelSectionStart = new QLabel("-", this);
    labelSectionEnd = new QLabel("-", this);

    layoutSection->addWidget(labelSectionStart, 1, 1, 1, 1);
    layoutSection->addWidget(labelSectionEnd, 2, 1, 1, 1);

    //build widget

    mainLayout->addLayout(layoutName);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(layoutModelingSettings);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(layoutFitParameter);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(layoutNumericParameter);
    mainLayout->addSpacing(10);    
    mainLayout->addLayout(layoutSection);
    //mainLayout->addSpacing(10);
    mainLayout->addLayout(layoutSimulationParameters);
    mainLayout->addStretch(-1);

    mainLayout->setAlignment(layoutName, Qt::AlignLeft | Qt::AlignTop);
    mainLayout->setAlignment(layoutModelingSettings, Qt::AlignLeft | Qt::AlignTop);
    mainLayout->setAlignment(layoutFitParameter, Qt::AlignLeft | Qt::AlignTop);
    mainLayout->setAlignment(layoutNumericParameter, Qt::AlignLeft | Qt::AlignTop);
    mainLayout->setAlignment(layoutSimulationParameters, Qt::AlignLeft | Qt::AlignTop);
    mainLayout->setAlignment(layoutSection, Qt::AlignLeft | Qt::AlignTop);
}


void FT_ParameterTable::clear()
{
    labelName->setText("-");

    labelHeatTransferModel->setText("-");
    labelGasMixture->setText("-");
    labelMaterial->setText("-");
    labelProcessPressure->setText("-");

    checkboxUseConduction->setChecked(false);
    checkboxUseEvaporation->setChecked(false);
    checkboxUseRadiation->setChecked(false);

    checkboxStartParticleDiameterFixed->setChecked(false);
    labelStartParticleDiameterValue->setText("-");
    labelStartParticleDiameterLower->setText("-");
    labelStartParticleDiameterUpper->setText("-");
    labelStartParticleDiameterDelta->setText("-");

    checkboxGasTemperatureFixed->setChecked(false);
    labelGasTemperatureValue->setText("-");
    labelGasTemperatureLower->setText("-");
    labelGasTemperatureUpper->setText("-");
    labelGasTemperatureDelta->setText("-");

    checkboxStartTemperatureFixed->setChecked(false);
    labelStartTemperatureValue->setText("-");
    labelStartTemperatureLower->setText("-");
    labelStartTemperatureUpper->setText("-");
    labelStartTemperatureDelta->setText("-");

    labelMaxIterations->setText("-");

    labelStartTime->setText("-");
    labelSimulationLength->setText("-");    
    labelStepSize->setText("-");

    labelSectionStart->setText("-");
    labelSectionEnd->setText("-");
}


void FT_ParameterTable::update(FitRun *fitRun)
{
    if(fitRun)
    {        
        // hide simulation only box
        for(int j = 0; j < layoutSimulationParameters->count(); j++)
            layoutSimulationParameters->itemAt(j)->widget()->setVisible(false);

        // hide numeric parameter and range
        for(int j = 0; j < layoutNumericParameter->count(); j++)
            layoutNumericParameter->itemAt(j)->widget()->setVisible(true);
        for(int j = 0; j < layoutSection->count(); j++)
            layoutSection->itemAt(j)->widget()->setVisible(true);

        labelName->setText(QString("%0 (%1, %2)").arg(fitRun->name())
                                     .arg(fitRun->creationDate().time().toString())
                                     .arg(fitRun->creationDate().date().toString("ddd MM-dd-yyyy")));

        ModelingSettings *mSettings = fitRun->modelingSettings(0);
        labelHeatTransferModel->setText(mSettings->heatTransferModel()->name);
        labelGasMixture->setText(mSettings->gasMixture().name);
        labelMaterial->setText(mSettings->material().name);
        labelProcessPressure->setText(QString::number(mSettings->processPressure() * unitConversion_pressure));

        checkboxUseEvaporation->setChecked(mSettings->heatTransferModel()->useEvaporation);
        checkboxUseConduction->setChecked(mSettings->heatTransferModel()->useConduction);        
        checkboxUseRadiation->setChecked(mSettings->heatTransferModel()->useRadiation);

        QList<FitParameter> fitParameter = fitRun->fitSettings()->fitParameters();

        checkboxStartParticleDiameterFixed->setChecked(!fitParameter.at(0).enabled());
        labelStartParticleDiameterValue->setText(QString::number(fitParameter.at(0).value()));
        labelStartParticleDiameterLower->setText(QString::number(fitParameter.at(0).lowerBound()));
        labelStartParticleDiameterUpper->setText(QString::number(fitParameter.at(0).upperBound()));
        labelStartParticleDiameterDelta->setText(QString::number(fitParameter.at(0).maxDelta()));

        checkboxGasTemperatureFixed->setChecked(!fitParameter.at(1).enabled());
        labelGasTemperatureValue->setText(QString::number(fitParameter.at(1).value()));
        labelGasTemperatureLower->setText(QString::number(fitParameter.at(1).lowerBound()));
        labelGasTemperatureUpper->setText(QString::number(fitParameter.at(1).upperBound()));
        labelGasTemperatureDelta->setText(QString::number(fitParameter.at(1).maxDelta()));

        checkboxStartTemperatureFixed->setChecked(!fitParameter.at(2).enabled());
        labelStartTemperatureValue->setText(QString::number(fitParameter.at(2).value()));
        labelStartTemperatureLower->setText(QString::number(fitParameter.at(2).lowerBound()));
        labelStartTemperatureUpper->setText(QString::number(fitParameter.at(2).upperBound()));
        labelStartTemperatureDelta->setText(QString::number(fitParameter.at(2).maxDelta()));

        NumericSettings *nSettings = fitRun->numericSettings();
        labelMaxIterations->setText(QString::number(nSettings->iterations()));
        labelODESolver->setText(Numeric::getAvailableODENameList().at(nSettings->odeSolverIdx()));
        labelODEStepSize->setText(QString::number(nSettings->odeSolverStepSizeFactor()));

        labelStepSize->setText("-");
        labelStartTime->setText("-");

        if(fitRun->sectionSet())
        {
            labelSectionStart->setText(QString::number(fitRun->sectionBegin() * unitConversion_time));
            labelSectionEnd->setText(QString::number(fitRun->sectionEnd() * unitConversion_time));
            labelSection->setVisible(true);
            labelSectionBeginTitle->setVisible(true);
            labelSectionEndTitle->setVisible(true);
            labelSectionStart->setVisible(true);
            labelSectionEnd->setVisible(true);
        }
        else
        {
            labelSectionStart->setText("-");
            labelSectionEnd->setText("-");
            labelSection->setVisible(false);
            labelSectionBeginTitle->setVisible(false);
            labelSectionEndTitle->setVisible(false);
            labelSectionStart->setVisible(false);
            labelSectionEnd->setVisible(false);
        }
    }
}


void FT_ParameterTable::update(SimRun *simRun)
{
    clear();

    if(simRun)
    {
        // show simulation only box
        for(int j = 0; j < layoutSimulationParameters->count(); j++)
            layoutSimulationParameters->itemAt(j)->widget()->setVisible(true);

        // hide numeric parameter and range
        for(int j = 0; j < layoutNumericParameter->count(); j++)
            layoutNumericParameter->itemAt(j)->widget()->setVisible(false);
        for(int j = 0; j < layoutSection->count(); j++)
            layoutSection->itemAt(j)->widget()->setVisible(false);

        labelName->setText(QString("Simulation %0").arg(simRun->mSimcounter));

        ModelingSettings *mSettings = simRun->modelingSettings();
        labelHeatTransferModel->setText(mSettings->heatTransferModel()->name);
        labelGasMixture->setText(mSettings->gasMixture().name);
        labelMaterial->setText(mSettings->material().name);
        labelProcessPressure->setText(QString::number(mSettings->processPressure() * unitConversion_pressure));

        checkboxUseEvaporation->setChecked(mSettings->heatTransferModel()->useEvaporation);
        checkboxUseConduction->setChecked(mSettings->heatTransferModel()->useConduction);
        checkboxUseRadiation->setChecked(mSettings->heatTransferModel()->useRadiation);

        QList<FitParameter> fitParameter = simRun->fitSettings()->fitParameters();

        checkboxStartParticleDiameterFixed->setChecked(!fitParameter.at(0).enabled());
        labelStartParticleDiameterValue->setText(QString::number(fitParameter.at(0).value()));
        labelStartParticleDiameterLower->setText(QString::number(fitParameter.at(0).lowerBound()));
        labelStartParticleDiameterUpper->setText(QString::number(fitParameter.at(0).upperBound()));
        labelStartParticleDiameterDelta->setText(QString::number(fitParameter.at(0).maxDelta()));

        checkboxGasTemperatureFixed->setChecked(!fitParameter.at(1).enabled());
        labelGasTemperatureValue->setText(QString::number(fitParameter.at(1).value()));
        labelGasTemperatureLower->setText(QString::number(fitParameter.at(1).lowerBound()));
        labelGasTemperatureUpper->setText(QString::number(fitParameter.at(1).upperBound()));
        labelGasTemperatureDelta->setText(QString::number(fitParameter.at(1).maxDelta()));

        checkboxStartTemperatureFixed->setChecked(!fitParameter.at(2).enabled());
        labelStartTemperatureValue->setText(QString::number(fitParameter.at(2).value()));
        labelStartTemperatureLower->setText(QString::number(fitParameter.at(2).lowerBound()));
        labelStartTemperatureUpper->setText(QString::number(fitParameter.at(2).upperBound()));
        labelStartTemperatureDelta->setText(QString::number(fitParameter.at(2).maxDelta()));

        NumericSettings *nSettings = simRun->numericSettings();

        labelMaxIterations->setText("-");
        labelODESolver->setText(Numeric::getAvailableODENameList().at(nSettings->odeSolverIdx()));
        labelODEStepSize->setText(QString::number(nSettings->odeSolverStepSizeFactor()));

        // Simulation only
        labelStartTime->setText(QString::number(nSettings->startTime() * unitConversion_time));
        labelSimulationLength->setText(QString::number(nSettings->simLength() * unitConversion_time));
        labelStepSize->setText(QString::number(nSettings->stepSize() * unitConversion_time));

        // Range
        labelSectionStart->setText("-");
        labelSectionEnd->setText("-");
    }
}

