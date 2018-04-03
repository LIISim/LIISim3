#ifndef FT_RUNLISTFITDATAITEM_H
#define FT_RUNLISTFITDATAITEM_H

#include <QTreeWidgetItem>
#include <QCheckBox>
#include <QSpinBox>
#include <QMap>

#include "../../calculations/fit/fitdata.h"
#include "../utils/visibilitybutton.h"

class FT_ResultVisualization;
class FT_PlotCurve;

class FT_RunListFitDataItem : public QObject, public QTreeWidgetItem
{
    friend class FT_FitList;

    Q_OBJECT
public:
    explicit FT_RunListFitDataItem(FT_ResultVisualization *visualizationWidget, FitData fitData, QTreeWidgetItem *parent);

    bool isChecked();
    void setChecked(bool checked);
    void setIteration(int iteration);
    int getIteration();

    void cleanup();

private:
    void setCurveState(bool enabled);

    void setHeatTransferRateCurves(int iteration);

    FitData mFitData;

    FT_ResultVisualization *mVisualizationWidget;

    QCheckBox *checkboxFitRun;
    QPushButton *buttonText;

    QString tChannelInfo;
    QSpinBox *spinboxIteration;
    QWidget *widgetIterations;

    VisibilityButton *visibilityModel;
    VisibilityButton *visibilityData;

    FT_PlotCurve *curveData;
    FT_PlotCurve *curveTemperature;
    FT_PlotCurve *curveParticleDiameter;
    FT_PlotCurve *curveParticleStartDiameter;
    FT_PlotCurve *curveGasTemperature;
    FT_PlotCurve *curveFitError;

    QMap<int, Signal> curvesEvaporation;
    QMap<int, Signal> curvesConduction;
    QMap<int, Signal> curvesRadiation;

    FT_PlotCurve *curveEvaporation;
    FT_PlotCurve *curveConduction;
    FT_PlotCurve *curveRadiation;

private slots:
    void onSpinboxIterationChanged(int value);
    void onStateChanged(int state);

    void onModelVisibilityToggled(bool checked);
    void onDataVisibilityToggled(bool checked);
    void onButtonTextClicked();

    void onCBEvaporationStateChanged(int state);
    void onCBConductionStateChanged(int state);
    void onCBRadiationStateChanged(int state);

    void onMRunDeleted();

signals:
    void stateChanged();

    void changed();
};

#endif // FT_RUNLISTFITDATAITEM_H
