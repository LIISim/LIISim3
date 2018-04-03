#include "ft_simruntreeitem.h"

#include "ft_resultvisualization.h"
#include "ft_plotcurve.h"
#include "../../calculations/fit/simrun.h"
#include "../utils/colorgenerator.h"

#include "../../calculations/numeric.h"

FT_SimRunTreeItem::FT_SimRunTreeItem(FT_ResultVisualization *visualizationWidget,
                                     SimRun *simRun,
                                     QTreeWidgetItem *parent) : QObject(),
                                        QTreeWidgetItem(parent, QTreeWidgetItem::UserType+12)
{
    mVisualizationWidget = visualizationWidget;
    mSimRun = simRun;    
    curveSimRun = nullptr;
    curveParticleDiameter = nullptr;

    // heat transfer rates    
    curveEvaporation = nullptr;
    curveConduction = nullptr;
    curveRadiation = nullptr;

    switch(mSimRun->numericSettings()->odeSolverIdx())
    {
        case Numeric::ODE_Algorithm::EULER:
            m_plotChannel = 61;
            break;
        case Numeric::ODE_Algorithm::RKD5_OPT:
            m_plotChannel = 62;
            break;
        case Numeric::ODE_Algorithm::RKD5:
            m_plotChannel = 63;
            break;
        default:
            m_plotChannel = 6;
    }

    // layout
    checkboxSimRun = new QCheckBox();
    checkboxSimRun->setChecked(true);

    buttonText = new QPushButton(QString("Simulation %0").arg(mSimRun->mSimcounter));
    buttonText->setFlat(true);

    QWidget *widgetBoxButton = new QWidget;
    QHBoxLayout *layoutBoxButton = new QHBoxLayout;
    layoutBoxButton->setMargin(0);
    widgetBoxButton->setLayout(layoutBoxButton);
    layoutBoxButton->addWidget(checkboxSimRun);
    layoutBoxButton->addWidget(buttonText, 0, Qt::AlignLeft);
    layoutBoxButton->addStretch(-1);

    if(parent && treeWidget())
    {
        treeWidget()->setItemWidget(this, 0, widgetBoxButton);

        setTextAlignment(4,Qt::AlignCenter);
        setText(4, QString::number(mSimRun->fitSettings()->fitParameters().at(0).value(), 'f', 1).append(" nm"));

        setTextAlignment(5,Qt::AlignCenter);
        setText(5, QString::number(mSimRun->fitSettings()->fitParameters().at(1).value(), 'f', 0).append(" K"));

        setTextAlignment(6,Qt::AlignCenter);
        setText(6, QString::number(mSimRun->fitSettings()->fitParameters().at(2).value(), 'f', 0).append(" K"));
    }

    //build data curve (channel 6)
    curveSimRun = new FT_PlotCurve(QString("Simulation %0").arg(mSimRun->mSimcounter),
                                   visualizationWidget->temperaturePlot, mSimRun->sim_trace);

    //curveSimRun->setPen(Qt::darkCyan, 2); still used?
    curveSimRun->getCGInstance()->registerCurve(curveSimRun, m_plotChannel);
    curveSimRun->getCGInstance()->updateCurveColors(m_plotChannel);

    //build particle diameter curve (channel 2)
    Signal signalPD = mSimRun->sim_trace;
    signalPD.data.clear();
    for(int i = 0; i < signalPD.dataDiameter.size(); i++)
        signalPD.data.push_back(signalPD.dataDiameter.at(i));

    curveParticleDiameter = new FT_PlotCurve(QString("Simulation %0").arg(mSimRun->mSimcounter),
                                             visualizationWidget->particleDiameterPlot, signalPD);
    curveParticleDiameter->setPen(Qt::magenta, 2);
    curveParticleDiameter->getCGInstance()->registerCurve(curveParticleDiameter, 2);


    // build heat transfer rate curves

    // Evaporation: red - channel 7
    if(simRun->modelingSettings()->heatTransferModel()->useEvaporation)
    {
        curveEvaporation = new FT_PlotCurve(QString("Simulation %0: Evaporation").arg(mSimRun->mSimcounter),
                                            visualizationWidget->heatTransferRates,
                                            mSimRun->getEvaporationCurve());
        curveEvaporation->setPen(Qt::red, 2);
        curveEvaporation->getCGInstance()->registerCurve(curveEvaporation, 7);
    }
    else
    {
        curveEvaporation = new FT_PlotCurve(QString("Simulation %0: Evaporation disabled").arg(mSimRun->mSimcounter),
                                            visualizationWidget->heatTransferRates,
                                            Signal());
    }

    // Conduction: green - channel 8
    if(simRun->modelingSettings()->heatTransferModel()->useConduction)
    {
        curveConduction = new FT_PlotCurve(QString("Simulation %0: Conduction").arg(mSimRun->mSimcounter),
                                            visualizationWidget->heatTransferRates,
                                            mSimRun->getConductionCurve());
        curveConduction->setPen(Qt::green, 2);
        curveConduction->getCGInstance()->registerCurve(curveConduction, 8);
    }
    else
    {
        curveConduction = new FT_PlotCurve(QString("Simulation %0: Conduction disabled").arg(mSimRun->mSimcounter),
                                            visualizationWidget->heatTransferRates,
                                            Signal());
    }

    // Radiation: blue - channel 9
    if(simRun->modelingSettings()->heatTransferModel()->useRadiation)
    {
        curveRadiation = new FT_PlotCurve(QString("Simulation %0: Radiation").arg(mSimRun->mSimcounter),
                                            visualizationWidget->heatTransferRates,
                                            mSimRun->getRadiationCurve());
        curveRadiation->setPen(Qt::blue, 2);
        curveRadiation->getCGInstance()->registerCurve(curveRadiation, 9);
    }
    else
    {
        curveRadiation = new FT_PlotCurve(QString("Simulation %0: Radiation disabled").arg(mSimRun->mSimcounter),
                                            visualizationWidget->heatTransferRates,
                                            Signal());
    }


    // select SimRun after creation
    onStateChanged(Qt::Checked);

    connect(checkboxSimRun, SIGNAL(stateChanged(int)), SLOT(onStateChanged(int)));
    connect(buttonText, SIGNAL(clicked(bool)), SLOT(onButtonTextClicked()));

    if(visualizationWidget)
    {        
        connect(visualizationWidget->checkboxEvaporation, SIGNAL(stateChanged(int)), SLOT(onCBEvaporationStateChanged(int)));
        connect(visualizationWidget->checkboxConduction, SIGNAL(stateChanged(int)), SLOT(onCBConductionStateChanged(int)));
        connect(visualizationWidget->checkboxRadiation, SIGNAL(stateChanged(int)), SLOT(onCBRadiationStateChanged(int)));
    }
}


void FT_SimRunTreeItem::cleanup()
{
    curveSimRun->disable();
    curveParticleDiameter->disable();

    curveEvaporation->disable();
    curveConduction->disable();
    curveRadiation->disable();

    curveParticleDiameter->getCGInstance()->unregisterCurve(curveParticleDiameter, 2);

    curveSimRun->getCGInstance()->unregisterCurve(curveSimRun, m_plotChannel);

    curveEvaporation->getCGInstance()->unregisterCurve(curveEvaporation, 7);
    curveConduction->getCGInstance()->unregisterCurve(curveConduction, 8);
    curveRadiation->getCGInstance()->unregisterCurve(curveRadiation, 9);

    delete curveSimRun;
    delete curveParticleDiameter;

    delete curveEvaporation;
    delete curveConduction;
    delete curveRadiation;
}


void FT_SimRunTreeItem::onStateChanged(int state)
{
//    if(state == Qt::Checked && curveSimRun)
//        curveSimRun->enable();
//    else if(curveSimRun)
//        curveSimRun->disable();

//    curveSimRun->getCGInstance()->updateCurveColors(6);
//    emit treeWidget()->itemChanged(this, 0);
    setCurveState(state == Qt::Checked);
}


void FT_SimRunTreeItem::onButtonTextClicked()
{
    emit treeWidget()->itemClicked(this, 0);
}


void FT_SimRunTreeItem::onCBEvaporationStateChanged(int state)
{
    if(state == Qt::Checked)
        curveEvaporation->enable();
    else
        curveEvaporation->disable();
}


void FT_SimRunTreeItem::onCBConductionStateChanged(int state)
{
    if(state == Qt::Checked)
        curveConduction->enable();
    else
        curveConduction->disable();
}


void FT_SimRunTreeItem::onCBRadiationStateChanged(int state)
{
    if(state == Qt::Checked)
        curveRadiation->enable();
    else
        curveRadiation->disable();
}


void FT_SimRunTreeItem::setCurveState(bool enabled)
{
    if(enabled)
    {
        if(curveSimRun)
            curveSimRun->enable();
        if(curveParticleDiameter)
            curveParticleDiameter->enable();

        if(curveEvaporation && mVisualizationWidget->checkboxEvaporation->isChecked())
            curveEvaporation->enable();
        if(curveConduction && mVisualizationWidget->checkboxConduction->isChecked())
            curveConduction->enable();
        if(curveRadiation && mVisualizationWidget->checkboxRadiation->isChecked())
            curveRadiation->enable();
    }
    else
    {
        if(curveSimRun)
            curveSimRun->disable();
        if(curveParticleDiameter)
            curveParticleDiameter->disable();

        if(curveEvaporation)
            curveEvaporation->disable();
        if(curveConduction)
            curveConduction->disable();
        if(curveRadiation)
            curveRadiation->disable();
    }

    curveSimRun->getCGInstance()->updateCurveColors(m_plotChannel);
}
