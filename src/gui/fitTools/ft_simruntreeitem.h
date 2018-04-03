#ifndef FT_SIMRUNTREEITEM_H
#define FT_SIMRUNTREEITEM_H

#include <QTreeWidgetItem>
#include <QCheckBox>
#include <QPushButton>

class SimRun;
class FT_ResultVisualization;
class FT_PlotCurve;

class FT_SimRunTreeItem : public QObject, public QTreeWidgetItem
{
    friend class FT_FitList;

    Q_OBJECT
public:
    explicit FT_SimRunTreeItem(FT_ResultVisualization *visualizationWidget, SimRun *simRun, QTreeWidgetItem *parent);

    void cleanup();

private:
    void setCurveState(bool enabled);

    FT_ResultVisualization *mVisualizationWidget;
    QCheckBox *checkboxSimRun;
    QPushButton *buttonText;

    SimRun *mSimRun;

    int m_plotChannel;

    FT_PlotCurve *curveSimRun;
    FT_PlotCurve *curveParticleDiameter;

    FT_PlotCurve *curveEvaporation;
    FT_PlotCurve *curveConduction;
    FT_PlotCurve *curveRadiation;

private slots:
    void onStateChanged(int state);    
    void onButtonTextClicked();

    void onCBEvaporationStateChanged(int state);
    void onCBConductionStateChanged(int state);
    void onCBRadiationStateChanged(int state);

signals:
    void stateChanged();
};

#endif // FT_SIMRUNTREEITEM_H
