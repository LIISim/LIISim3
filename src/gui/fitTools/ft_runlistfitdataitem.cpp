#include "ft_runlistfitdataitem.h"

#include "ft_resultvisualization.h"
#include "ft_plotcurve.h"
#include "../utils/colorgenerator.h"
#include "../../calculations/fit/fitrun.h"

FT_RunListFitDataItem::FT_RunListFitDataItem(FT_ResultVisualization *visualizationWidget, FitData fitData, QTreeWidgetItem *parent)
    : QObject(), QTreeWidgetItem(parent, QTreeWidgetItem::UserType+11)
{
    mFitData = fitData;
    mVisualizationWidget = visualizationWidget;

    curveData = nullptr;
    curveTemperature = nullptr;
    curveParticleDiameter = nullptr;
    curveParticleStartDiameter = nullptr;
    curveGasTemperature = nullptr;
    curveFitError = nullptr;

    // heat transfer rates
    curveEvaporation = nullptr;
    curveConduction = nullptr;
    curveRadiation = nullptr;

    if(parent && treeWidget())
    {
        checkboxFitRun = new QCheckBox();

        int tchid = mFitData.temperatureChannelID();

        buttonText = new QPushButton(QString("%0 (Signal: %1, T%2)")
                                     .arg(mFitData.mrun()->getName())
                                     .arg(mFitData.mpoint()+1)
                                     .arg(tchid));
        buttonText->setFlat(true);

        tChannelInfo = QString("Temperature-Channel  %0 \n"
                              "Method: %1 \n"
                              "Material (Spectroscopic): %2")
                              .arg(tchid)
                              .arg(mFitData.mrun()->tempMetadata.value(tchid).method)
                              .arg(mFitData.mrun()->tempMetadata.value(tchid).material);

        QWidget *widgetBoxButton = new QWidget;
        QHBoxLayout *layoutBoxButton = new QHBoxLayout;
        layoutBoxButton->setMargin(0);
        widgetBoxButton->setLayout(layoutBoxButton);
        widgetBoxButton->setToolTip(tChannelInfo);

        layoutBoxButton->addWidget(checkboxFitRun);
        layoutBoxButton->addWidget(buttonText, 0, Qt::AlignLeft);
        layoutBoxButton->addStretch(-1);

        treeWidget()->setItemWidget(this, 0, widgetBoxButton);

        visibilityModel = new VisibilityButton();
        visibilityModel->setToolTip("Show modeled signal from heat transfer model");
        treeWidget()->setItemWidget(this, 1, visibilityModel);

        visibilityData = new VisibilityButton();
        visibilityData->setToolTip("Show experimental data");
        treeWidget()->setItemWidget(this, 2, visibilityData);

        widgetIterations = new QWidget;
        widgetIterations->setLayout(new QHBoxLayout);

        spinboxIteration = new QSpinBox(widgetIterations);
        spinboxIteration->setMinimum(1);
        spinboxIteration->setMaximum(mFitData.iterationResCount());
        spinboxIteration->setValue(mFitData.iterationResCount());

        QLabel *labelIterations = new QLabel(QString("/ %0").arg(QString::number(mFitData.iterationResCount())), widgetIterations);

        widgetIterations->layout()->setMargin(0);
        widgetIterations->layout()->addWidget(spinboxIteration);
        widgetIterations->layout()->addWidget(labelIterations);

        treeWidget()->setItemWidget(this, 3, widgetIterations);

        setTextAlignment(4,Qt::AlignCenter);
        setText(4, QString::number(mFitData.modeledSignal(mFitData.iterationResCount()-1).dataDiameter.first(), 'f', 1).append(" nm"));

        setTextAlignment(5,Qt::AlignCenter);
        setText(5, QString::number(mFitData.getParameterCurve(4).last(), 'f', 0).append(" K"));

        setTextAlignment(6,Qt::AlignCenter);
        setText(6, QString::number(mFitData.getParameterCurve(6).last(), 'f', 0).append(" K"));

        connect(checkboxFitRun, SIGNAL(stateChanged(int)), SLOT(onStateChanged(int)));
        connect(spinboxIteration, SIGNAL(valueChanged(int)), SLOT(onSpinboxIterationChanged(int)));

        connect(visibilityData, SIGNAL(visibilityToggled(bool)), SLOT(onDataVisibilityToggled(bool)));
        connect(visibilityModel, SIGNAL(visibilityToggled(bool)), SLOT(onModelVisibilityToggled(bool)));

        connect(buttonText, SIGNAL(clicked(bool)), SLOT(onButtonTextClicked()));
    }

    if(visualizationWidget)
    {        
        connect(visualizationWidget->checkboxEvaporation, SIGNAL(stateChanged(int)), SLOT(onCBEvaporationStateChanged(int)));
        connect(visualizationWidget->checkboxConduction, SIGNAL(stateChanged(int)), SLOT(onCBConductionStateChanged(int)));
        connect(visualizationWidget->checkboxRadiation, SIGNAL(stateChanged(int)), SLOT(onCBRadiationStateChanged(int)));
    }

    int iterationStep = mFitData.iterationResCount()-1;


    //build data curve (channel 0)
    QString curveNameD = QString("%0 (Signal: %1, T%2)")
            .arg(mFitData.mrun()->getName())
            .arg(mFitData.mpoint()+1)
            .arg(mFitData.temperatureChannelID());
    curveData = new FT_PlotCurve(curveNameD,
                                 visualizationWidget->temperaturePlot,
                                 mFitData.dataSignal());
    curveData->setPen(Qt::darkGreen, 2);
    curveData->getCGInstance()->registerCurve(curveData, 0);


    //build temperature curve (channel 1)
    QString curveNameT = QString("%0 (Signal: %1, T%2) [Peak: %3, Start Dp: %4 nm, Gas Temperature: %5 K]")
            .arg(mFitData.mrun()->getName())
            .arg(mFitData.mpoint()+1)
            .arg(mFitData.temperatureChannelID())
            .arg(mFitData.iterationResult(iterationStep).at(6))
            .arg(round(mFitData.iterationResult(iterationStep).at(2)*10.0)*0.1)
            .arg(mFitData.iterationResult(iterationStep).at(4));
    curveTemperature = new FT_PlotCurve(curveNameT,
                                        visualizationWidget->temperaturePlot,
                                        mFitData.modeledSignal(iterationStep));
    curveTemperature->setPen(Qt::magenta, 2);
    curveTemperature->getCGInstance()->registerCurve(curveTemperature, 1);


    //build particle diameter curve (channel 2)
    QString curveNamePD = QString("%0 (Signal: %1, T%2) [Peak: %3, Start Dp: %4 nm, Gas Temperature: %5 K]")
            .arg(mFitData.mrun()->getName())
            .arg(mFitData.mpoint()+1)
            .arg(mFitData.temperatureChannelID())
            .arg(mFitData.iterationResult(iterationStep).at(6))
            .arg(round(mFitData.iterationResult(iterationStep).at(2)*10.0)*0.1)
            .arg(mFitData.iterationResult(iterationStep).at(4));
    Signal signalPD = mFitData.modeledSignal(iterationStep);
    signalPD.data.clear();
    for(int i = 0; i < signalPD.dataDiameter.size(); i++)
        signalPD.data.push_back(signalPD.dataDiameter.at(i));
    curveParticleDiameter = new FT_PlotCurve(curveNamePD,
                                             visualizationWidget->particleDiameterPlot,
                                             signalPD);
    curveParticleDiameter->setPen(Qt::magenta, 2);
    curveParticleDiameter->getCGInstance()->registerCurve(curveParticleDiameter, 2);


    //build particle start diameter curve (channel 3)
    Signal signalSD;
    signalSD.type = Signal::RAW;
    signalSD.data = mFitData.getParameterCurve(2);
    signalSD.dt = 1e-9;
    signalSD.start_time = 0.0;
    curveParticleStartDiameter = new FT_PlotCurve(fitData.mrun()->name,
                                                  visualizationWidget->particleStartDiameterPlot,
                                                  signalSD);
    curveParticleStartDiameter->setPen(Qt::magenta, 2);
    curveParticleStartDiameter->getCGInstance()->registerCurve(curveParticleStartDiameter, 3);


    //build gas temperature curve (channel 4)
    Signal signalGT;
    signalGT.type = Signal::RAW;
    signalGT.data = mFitData.getParameterCurve(4);
    signalGT.dt = 1e-9;
    signalGT.start_time = 0.0;
    curveGasTemperature = new FT_PlotCurve(fitData.mrun()->name,
                                           visualizationWidget->gasTemperaturePlot,
                                           signalGT);
    curveGasTemperature->setPen(Qt::magenta, 2);
    curveGasTemperature->getCGInstance()->registerCurve(curveGasTemperature, 4);

    //build fit error curve (channel 5)
    Signal signalFE;
    signalFE.type = Signal::RAW;
    signalFE.data = mFitData.getParameterCurve(0);
    signalFE.dt = 1e-9;
    signalFE.start_time = 0.0;
    curveFitError = new FT_PlotCurve(fitData.mrun()->name,
                                     visualizationWidget->fitErrorPlot,
                                     signalFE);
    curveFitError->setPen(Qt::magenta, 2);
    curveFitError->getCGInstance()->registerCurve(curveFitError, 5);

    // heat transfer rates

    // Evaporation: red - channel 7
    if(mFitData.fitRun()->modelingSettings(-1)->heatTransferModel()->useEvaporation)
    {
        curvesEvaporation.insert(iterationStep, mFitData.getEvaporationCurve(iterationStep));
        curveEvaporation = new FT_PlotCurve(fitData.mrun()->name + " Evaporation",
                                            visualizationWidget->heatTransferRates,
                                            curvesEvaporation.value(iterationStep));
        curveEvaporation->setPen(Qt::red, 2);
        curveEvaporation->getCGInstance()->registerCurve(curveEvaporation, 7);
    }
    else
    {
        curveEvaporation = new FT_PlotCurve(fitData.mrun()->name + " Evaporation disabled",
                                            visualizationWidget->heatTransferRates,
                                            Signal());
    }

    // Conduction: green - channel 8
    if(mFitData.fitRun()->modelingSettings(-1)->heatTransferModel()->useConduction)
    {
        curvesConduction.insert(iterationStep, mFitData.getConductionCurve(iterationStep));
        curveConduction = new FT_PlotCurve(fitData.mrun()->name + " Conduction",
                                           visualizationWidget->heatTransferRates,
                                           curvesConduction.value(iterationStep));
        curveConduction->setPen(Qt::green, 2);
        curveConduction->getCGInstance()->registerCurve(curveConduction, 8);
    }
    else
    {
        curveConduction = new FT_PlotCurve(fitData.mrun()->name + " Conduction disabled",
                                            visualizationWidget->heatTransferRates,
                                            Signal());
    }

    // Radiation: blue - channel 9
    if(mFitData.fitRun()->modelingSettings(-1)->heatTransferModel()->useRadiation)
    {
        curvesRadiation.insert(iterationStep, mFitData.getRadiationCurve(iterationStep));
        curveRadiation = new FT_PlotCurve(fitData.mrun()->name + " Radiation",
                                          visualizationWidget->heatTransferRates,
                                          curvesRadiation.value(iterationStep));
        curveRadiation->setPen(Qt::blue, 2);
        curveRadiation->getCGInstance()->registerCurve(curveRadiation, 9);
    }
    else
    {
        curveRadiation = new FT_PlotCurve(fitData.mrun()->name + " Radiation disabled",
                                            visualizationWidget->heatTransferRates,
                                            Signal());
    }

    curveData->getCGInstance()->updateCurveColors();

    connect(mFitData.mrun(), SIGNAL(destroyed(QObject*)), SLOT(onMRunDeleted()));
}


void FT_RunListFitDataItem::setHeatTransferRateCurves(int iteration)
{
    if(!curvesEvaporation.contains(iteration))
        curvesEvaporation.insert(iteration, mFitData.getEvaporationCurve(iteration));
    curveEvaporation->setSignal(curvesEvaporation.value(iteration));

    if(!curvesConduction.contains(iteration))
        curvesConduction.insert(iteration, mFitData.getConductionCurve(iteration));
    curveConduction->setSignal(curvesConduction.value(iteration));

    if(!curvesRadiation.contains(iteration))
        curvesRadiation.insert(iteration, mFitData.getRadiationCurve(iteration));
    curveRadiation->setSignal(curvesRadiation.value(iteration));
}


bool FT_RunListFitDataItem::isChecked()
{
    return checkboxFitRun->isChecked();
}


void FT_RunListFitDataItem::setChecked(bool checked)
{
    checkboxFitRun->blockSignals(true);
    checkboxFitRun->setChecked(checked);
    checkboxFitRun->blockSignals(false);

    this->blockSignals(true);
    setCurveState(checked);
    this->blockSignals(false);
}


void FT_RunListFitDataItem::setIteration(int iteration)
{
    if(iteration >= 0 && iteration < mFitData.iterationResCount())
        spinboxIteration->setValue(iteration+1);
}


int FT_RunListFitDataItem::getIteration()
{
    return spinboxIteration->value();
}


void FT_RunListFitDataItem::cleanup()
{
    curveData->disable();
    curveTemperature->disable();
    curveParticleDiameter->disable();
    curveParticleStartDiameter->disable();
    curveGasTemperature->disable();
    curveFitError->disable();

    curveEvaporation->disable();
    curveConduction->disable();
    curveRadiation->disable();

    curveData->getCGInstance()->unregisterCurve(curveData, 0);
    curveTemperature->getCGInstance()->unregisterCurve(curveTemperature, 1);
    curveParticleDiameter->getCGInstance()->unregisterCurve(curveParticleDiameter, 2);
    curveParticleStartDiameter->getCGInstance()->unregisterCurve(curveParticleStartDiameter, 3);
    curveGasTemperature->getCGInstance()->unregisterCurve(curveGasTemperature, 4);
    curveFitError->getCGInstance()->unregisterCurve(curveFitError, 5);

    curveEvaporation->getCGInstance()->unregisterCurve(curveEvaporation, 7);
    curveConduction->getCGInstance()->unregisterCurve(curveConduction, 8);
    curveRadiation->getCGInstance()->unregisterCurve(curveRadiation, 9);

    delete curveData;
    delete curveTemperature;
    delete curveParticleDiameter;
    delete curveParticleStartDiameter;
    delete curveGasTemperature;
    delete curveFitError;

    delete curveEvaporation;
    delete curveConduction;
    delete curveRadiation;
}


void FT_RunListFitDataItem::onSpinboxIterationChanged(int value)
{
    if(curveTemperature)
    {
        QString curveNameT = QString("%0 (Signal: %1, T%2) [Peak: %3, Start Particle Diameter: %4 nm, Gas Temperature: %5 K")
                .arg(mFitData.mrun()->getName())
                .arg(mFitData.mpoint()+1)
                .arg(mFitData.temperatureChannelID())
                .arg(mFitData.iterationResult(value-1).at(6))
                .arg(round(mFitData.iterationResult(value-1).at(2)*1E10)*0.1)
                .arg(mFitData.iterationResult(value-1).at(4));

        if(mFitData.iterationResCount() != value)
            curveNameT.append(QString(", Iteration: %0").arg(value));

        curveNameT.append("]");

        curveTemperature->setSignal(mFitData.modeledSignal(value-1));
        curveTemperature->setTitle(curveNameT);
    }

    if(curveParticleDiameter)
    {
        QString curveNamePD = QString("%0 (Signal: %1, T%2) [Start Particle Diameter: %3 m, Gas Temperature : %4 K")
                .arg(mFitData.mrun()->getName()).arg(mFitData.mpoint()+1).arg(mFitData.temperatureChannelID())
                .arg(mFitData.iterationResult(value-1).at(0)).arg(mFitData.iterationResult(value-1).at(1));

        if(mFitData.iterationResCount() != value)
            curveNamePD.append(QString(", Iteration: %0").arg(value));

        curveNamePD.append("]");

        Signal signalPD = mFitData.modeledSignal(value-1);
        signalPD.data.clear();
        for(int i = 0; i < signalPD.dataDiameter.size(); i++)
            signalPD.data.push_back(signalPD.dataDiameter.at(i));

        curveParticleDiameter->setSignal(signalPD);
        curveParticleDiameter->setTitle(curveNamePD);
    }

    setHeatTransferRateCurves(value-1);
}


void FT_RunListFitDataItem::onStateChanged(int state)
{
    setCurveState(state == Qt::Checked);

    emit stateChanged();
    //emit treeWidget()->itemClicked(this, 0);
}


void FT_RunListFitDataItem::onModelVisibilityToggled(bool checked)
{
    if(checkboxFitRun->isChecked() && checked)
        curveTemperature->enable();
    else
        curveTemperature->disable();
    curveTemperature->getCGInstance()->updateCurveColors(1);
}


void FT_RunListFitDataItem::onDataVisibilityToggled(bool checked)
{
    if(checkboxFitRun->isChecked() && checked)
        curveData->enable();
    else
        curveData->disable();
    curveData->getCGInstance()->updateCurveColors(0);
}


void FT_RunListFitDataItem::onButtonTextClicked()
{
    emit treeWidget()->itemClicked(this, 0);
}


void FT_RunListFitDataItem::onCBEvaporationStateChanged(int state)
{
    if(state == Qt::Checked && isChecked())
        curveEvaporation->enable();
    else
        curveEvaporation->disable();
}


void FT_RunListFitDataItem::onCBConductionStateChanged(int state)
{
    if(state == Qt::Checked && isChecked())
        curveConduction->enable();
    else
        curveConduction->disable();
}


void FT_RunListFitDataItem::onCBRadiationStateChanged(int state)
{
    if(state == Qt::Checked && isChecked())
        curveRadiation->enable();
    else
        curveRadiation->disable();
}


void FT_RunListFitDataItem::setCurveState(bool enabled)
{
    if(enabled)
    {
        if(curveData && visibilityData->isChecked())
            curveData->enable();
        if(curveTemperature && visibilityModel->isChecked())
            curveTemperature->enable();
        if(curveParticleDiameter)
            curveParticleDiameter->enable();
        if(curveParticleStartDiameter)
            curveParticleStartDiameter->enable();
        if(curveGasTemperature)
            curveGasTemperature->enable();
        if(curveFitError)
            curveFitError->enable();

        if(curveEvaporation && mVisualizationWidget->checkboxEvaporation->isChecked())
            curveEvaporation->enable();
        if(curveConduction && mVisualizationWidget->checkboxConduction->isChecked())
            curveConduction->enable();
        if(curveRadiation && mVisualizationWidget->checkboxRadiation->isChecked())
            curveRadiation->enable();
    }
    else
    {
        if(curveData)
            curveData->disable();
        if(curveTemperature)
            curveTemperature->disable();
        if(curveParticleDiameter)
            curveParticleDiameter->disable();
        if(curveParticleStartDiameter)
            curveParticleStartDiameter->disable();
        if(curveGasTemperature)
            curveGasTemperature->disable();
        if(curveFitError)
            curveFitError->disable();

        if(curveEvaporation)
            curveEvaporation->disable();
        if(curveConduction)
            curveConduction->disable();
        if(curveRadiation)
            curveRadiation->disable();
    }
    curveData->getCGInstance()->updateCurveColors();
}


void FT_RunListFitDataItem::onMRunDeleted()
{
    cleanup();
    delete this;
}


