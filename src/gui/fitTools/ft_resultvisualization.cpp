#include "ft_resultvisualization.h"

#include <QVBoxLayout>

FT_ResultVisualization::FT_ResultVisualization(QWidget *parent) : QWidget(parent)
{
    curveCount = 0;

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    QTabWidget *tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    // 1) Temperature plot

    QWidget *temperaturePlotWidget = new QWidget(this);
    temperaturePlotWidget->setLayout(new QVBoxLayout);
    temperaturePlotWidget->layout()->setMargin(0);

    temperaturePlot = new SignalPlotWidgetQwt(this);
    temperaturePlot->plot()->setAutoReplot();
    temperaturePlot->setPlotAxisTitles("Time / ns", "Temperature / K");

    temperaturePlot->setPlotLabelText("<b>Fit Tools:</b><br>\n\
(1) Select at least one measurement run and a channel in the MRunList <br>\n\
(2) Make sure the signal only presents the signal decay. You can select a range for the fit manually. <br>\n \
\t for this press \'select range for fit\' and select section of the signal starting from the peak or later <br>\n\
(3) Adjust the settings in the right top box to your needs<br>\n\
(4) Press \'Start Fitting\' to perform a Fit Run or \'Simulate\' to perform a simulation");
    temperaturePlot->plotTextLabelVisible(true);

    QToolBar *toolbarTemperature = new QToolBar("Plot Tools");
    QList<QAction*> actionsT = temperaturePlot->toolActions();
    actionsT.removeAt(1);
    toolbarTemperature->addActions(actionsT);
    toolbarTemperature->actions().first()->triggered(true);

    temperaturePlotWidget->layout()->addWidget(toolbarTemperature);
    temperaturePlotWidget->layout()->addWidget(temperaturePlot);
    temperaturePlotWidget->layout()->setAlignment(toolbarTemperature, Qt::AlignRight);

    tabWidget->addTab(temperaturePlotWidget, "Temperature Traces");

    // 2) Heat Transfer Rates

    QWidget *heatTransferRatesPlotWidget = new QWidget(this);
    heatTransferRatesPlotWidget->setLayout(new QVBoxLayout);
    heatTransferRatesPlotWidget->layout()->setMargin(0);

    heatTransferRates = new SignalPlotWidgetQwt(this);
    heatTransferRates->plot()->setAutoReplot();
    heatTransferRates->setYLogScale(true);
    heatTransferRates->setPlotAxisTitles("Time / ns", "Heat transfer rate / J/s");

    QWidget *heatTransferRatesCheckboxes = new QWidget(this);
    heatTransferRatesCheckboxes->setLayout(new QHBoxLayout);
    heatTransferRatesCheckboxes->layout()->setMargin(2);
    heatTransferRatesCheckboxes->setStyleSheet("margin-top: 4px;");

    checkboxEvaporation = new QCheckBox("Evaporation", this);
    checkboxEvaporation->setChecked(true);
    checkboxConduction = new QCheckBox("Conduction", this);
    checkboxConduction->setChecked(true);
    checkboxRadiation = new QCheckBox("Radiation", this);
    checkboxRadiation->setChecked(true);

    heatTransferRatesCheckboxes->layout()->addWidget(new QLabel(" Show:", this));    
    heatTransferRatesCheckboxes->layout()->addWidget(checkboxEvaporation);
    heatTransferRatesCheckboxes->layout()->addWidget(checkboxConduction);
    heatTransferRatesCheckboxes->layout()->addWidget(checkboxRadiation);

    QToolBar *toolbarHeatTransferRates = new QToolBar("Plot Tools");
    QList<QAction*> actionsHTR = heatTransferRates->toolActions();
    actionsHTR.removeAt(1); // remove PlotFitTool
    toolbarHeatTransferRates->addActions(actionsHTR);
    toolbarHeatTransferRates->actions().first()->triggered(true);

    QWidget *plotOptionsWidget = new QWidget(this);
    plotOptionsWidget->setLayout(new QHBoxLayout);
    plotOptionsWidget->layout()->setMargin(0);

    plotOptionsWidget->layout()->addWidget(heatTransferRatesCheckboxes);
    plotOptionsWidget->layout()->addWidget(toolbarHeatTransferRates);
    plotOptionsWidget->layout()->setAlignment(heatTransferRatesCheckboxes, Qt::AlignLeft);
    plotOptionsWidget->layout()->setAlignment(toolbarHeatTransferRates, Qt::AlignRight);

    heatTransferRatesPlotWidget->layout()->addWidget(plotOptionsWidget);
    heatTransferRatesPlotWidget->layout()->addWidget(heatTransferRates);

    tabWidget->addTab(heatTransferRatesPlotWidget, "Heat Transfer Rates");


    // 3) Particle diameter plot

    QWidget *particleDiameterPlotWidget = new QWidget(this);
    particleDiameterPlotWidget->setLayout(new QVBoxLayout);
    particleDiameterPlotWidget->layout()->setMargin(0);

    particleDiameterPlot = new SignalPlotWidgetQwt(this);
    particleDiameterPlot->plot()->setAutoReplot();
    particleDiameterPlot->setPlotAxisTitles("Time / ns", "Particle Diameter / nm");

    QToolBar *toolbarParticleDiameter = new QToolBar("Plot Tools");
    QList<QAction*> actionsPD = particleDiameterPlot->toolActions();
    actionsPD.removeAt(1);
    toolbarParticleDiameter->addActions(actionsPD);
    toolbarParticleDiameter->actions().first()->triggered(true);

    particleDiameterPlotWidget->layout()->addWidget(toolbarParticleDiameter);
    particleDiameterPlotWidget->layout()->addWidget(particleDiameterPlot);
    particleDiameterPlotWidget->layout()->setAlignment(toolbarParticleDiameter, Qt::AlignRight);

    tabWidget->addTab(particleDiameterPlotWidget, "Particle Diameter (Trace)");


    // 4) Particle start diameter plot (iterations)

    QWidget *particleStartDiameterWidget = new QWidget(this);
    particleStartDiameterWidget->setLayout(new QVBoxLayout);
    particleStartDiameterWidget->layout()->setMargin(0);

    particleStartDiameterPlot = new SignalPlotWidgetQwt(this);
    particleStartDiameterPlot->plot()->setAutoReplot();
    particleStartDiameterPlot->setPlotAxisTitles("Fit Iteration", "Particle start diameter / nm");

    QToolBar *toolbarParticleStartDiameter = new QToolBar("Plot Tools");
    QList<QAction*> actionsPSD = particleStartDiameterPlot->toolActions();
    actionsPSD.removeAt(1);
    toolbarParticleStartDiameter->addActions(actionsPSD);
    toolbarParticleStartDiameter->actions().first()->triggered(true);

    particleStartDiameterWidget->layout()->addWidget(toolbarParticleStartDiameter);
    particleStartDiameterWidget->layout()->addWidget(particleStartDiameterPlot);
    particleStartDiameterWidget->layout()->setAlignment(toolbarParticleStartDiameter, Qt::AlignRight);

    tabWidget->addTab(particleStartDiameterWidget, "Particle Diameter (Iterations)");


    // 5) Gas temperature plot

    QWidget *gasTemperatureWidget = new QWidget(this);
    gasTemperatureWidget->setLayout(new QVBoxLayout);
    gasTemperatureWidget->layout()->setMargin(0);

    gasTemperaturePlot = new SignalPlotWidgetQwt(this);
    gasTemperaturePlot->plot()->setAutoReplot();
    gasTemperaturePlot->setPlotAxisTitles("Fit Iteration", "Gas temperature / K");

    QToolBar *toolbarGasTemperature = new QToolBar("Plot Tools");
    QList<QAction*> actionsGT = gasTemperaturePlot->toolActions();
    actionsGT.removeAt(1);
    toolbarGasTemperature->addActions(actionsGT);
    toolbarGasTemperature->actions().first()->triggered(true);

    gasTemperatureWidget->layout()->addWidget(toolbarGasTemperature);
    gasTemperatureWidget->layout()->addWidget(gasTemperaturePlot);
    gasTemperatureWidget->layout()->setAlignment(toolbarGasTemperature, Qt::AlignRight);

    tabWidget->addTab(gasTemperatureWidget, "Gas Temperature (Iterations)");


    // 6) Fit error plot

    QWidget *fitErrorWidget = new QWidget(this);
    fitErrorWidget->setLayout(new QVBoxLayout);
    fitErrorWidget->layout()->setMargin(0);

    fitErrorPlot = new SignalPlotWidgetQwt(this);
    fitErrorPlot->plot()->setAutoReplot();
    fitErrorPlot->setPlotAxisTitles("Fit Iteration", "Fit Error  (Chisquare) / -");

    QToolBar *toolbarFitError = new QToolBar("Plot Error");
    QList<QAction*> actionsFE = fitErrorPlot->toolActions();
    actionsFE.removeAt(1);
    toolbarFitError->addActions(actionsFE);
    toolbarFitError->actions().first()->triggered(true);

    fitErrorWidget->layout()->addWidget(toolbarFitError);
    fitErrorWidget->layout()->addWidget(fitErrorPlot);
    fitErrorWidget->layout()->setAlignment(toolbarFitError, Qt::AlignRight);

    tabWidget->addTab(fitErrorWidget, "Fit Error (Iterations)");

    connect(temperaturePlot->plot(), SIGNAL(itemAttached(QwtPlotItem*,bool)), SLOT(onItemAttached(QwtPlotItem*,bool)));
}


void FT_ResultVisualization::onItemAttached(QwtPlotItem *plotItem, bool on)
{
    if(on)
        curveCount++;
    else
        if(curveCount > 0)
            curveCount--;

    if(curveCount == 0)
    {
        temperaturePlot->plot()->blockSignals(true);
        temperaturePlot->plotTextLabelVisible(true);
        temperaturePlot->plot()->blockSignals(false);
    }
    else
    {
        temperaturePlot->plot()->blockSignals(true);
        temperaturePlot->plotTextLabelVisible(false);
        temperaturePlot->plot()->blockSignals(false);
    }
}

