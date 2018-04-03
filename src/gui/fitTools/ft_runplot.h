#ifndef FT_RUNPLOT_H
#define FT_RUNPLOT_H

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QList>

#include "../utils/signalplotwidgetqwt.h"
#include "../../signal/mrun.h"
#include "../utils/colorgenerator.h"

#include "../utils/labeledlineedit.h"

class FT_RunPlot : public QWidget
{
    Q_OBJECT
public:
    FT_RunPlot(QWidget *parent = 0);
    ~FT_RunPlot();

    void update(QList<MRun*> runList);

    QList<int> getSelectedChannel();

    bool rangeValid();
    double getRangeStart();
    double getRangeEnd();

    Signal::SType getSelectedSource();

private:
    void updatePlot();

    SignalPlotWidgetQwt *plot;

    ColorGenerator colorGenerator;

    QComboBox *comboboxMode;

    QHBoxLayout *layoutChannelSelection;

    QList<int> activeTempChannel;
    QList<int> activeRawChannel;
    QList<int> activeAbsChannel;

    QList<QCheckBox*> channelCheckboxes;

    QList<MRun*> selectedMRuns;

    PlotAnalysisTool* patRange;
    LabeledLineEdit* le_fitStart;
    LabeledLineEdit* le_fitEnd;

    bool rangeSelected;

    double xStart;
    double xEnd;

    const QString identifier_temperature;
    const QString identifier_intensity_raw;
    const QString identifier_intensity_abs;

    const QString identifierGroup;
    const QString identifierFitStart;
    const QString identifierFitEnd;

private slots:
    void onModeComboboxIndexChanged();
    void onChannelCheckboxStateChanged();

    void onRangeSelected(double start, double stop);
    void onRangeHidden();

    void updateRangeFromLineEdit();

    void onRunDataChanged(int pos, QVariant data);

    void onGUISettingsChanged();

signals:
    void startFittingClicked();
    void startSimulationClicked();
    void cancelClicked();

};

#endif // FT_RUNPLOT_H
