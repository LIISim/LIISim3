#ifndef FT_RESULTVISUALIZATION_H
#define FT_RESULTVISUALIZATION_H

#include <QWidget>
#include <QToolBar>
#include <QTabWidget>

#include "../utils/signalplotwidgetqwt.h"

class FT_ResultVisualization : public QWidget
{
    Q_OBJECT

    friend class FT_FitList;
    friend class FT_RunListFitDataItem;
    friend class FT_SimRunTreeItem;
public:
    FT_ResultVisualization(QWidget *parent = 0);

private:
    SignalPlotWidgetQwt *temperaturePlot;
    SignalPlotWidgetQwt *particleDiameterPlot;
    SignalPlotWidgetQwt *particleStartDiameterPlot;
    SignalPlotWidgetQwt *gasTemperaturePlot;
    SignalPlotWidgetQwt *fitErrorPlot;

    //Heat Transfer Rates Plot
    SignalPlotWidgetQwt *heatTransferRates;

    QCheckBox *checkboxConduction;
    QCheckBox *checkboxEvaporation;
    QCheckBox *checkboxRadiation;

    int curveCount;

private slots:
    void onItemAttached(QwtPlotItem *plotItem, bool on);

};

#endif // FT_RESULTVISUALIZATION_H
