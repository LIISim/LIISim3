#ifndef PARAMETERANALYSIS_H
#define PARAMETERANALYSIS_H

#include "signalplottool.h"

#include "../../utils/baseplotwidgetqwt.h"
#include "../../utils/labeledcombobox.h"

#include <QTableWidget>
#include <QToolButton>
#include <QMutex>
#include <QThread>
#include <QProgressBar>
#include "../../utils/extendedtablewidget.h"

Q_DECLARE_METATYPE(Signal::SType)

class ParameterAnalysis;

class ParameterAnalysisUpdater : public QThread
{
    Q_OBJECT
public:
    ParameterAnalysisUpdater(ParameterAnalysis *parent);
    void run() Q_DECL_OVERRIDE;

    void lastCurveOnly();
    void setSelectionChanged();

private:
    bool updateLastCurveOnly;
    bool selectionChanged;

signals:
    void updaterFinished();
    void updaterProgress(int value);
};


class ParameterAnalysisCurve : public QObject
{
    Q_OBJECT

public:
    explicit ParameterAnalysisCurve(QObject *parent = 0);

    BasePlotCurve *curve;

    QString runNames;

    QString parameterX;
    QString parameterY;

    QVariant sourceX;
    QVariant sourceY;

    int sourceChannelX;
    int sourceChannelY;

    bool visible;
    bool ownColor;
    QColor color;

    QVector<double> xData;
    QVector<double> yData;

    //gui elements
    QToolButton *buttonVisibility;
    QPushButton *buttonRemove;
    QPushButton *buttonColor;

signals:
    void visibilityChanged();
    void colorChanged();

    void removeRequest(ParameterAnalysisCurve *source);

public slots:
    void onButtonVisibilityClicked();
    void onButtonRemoveReleased();
    void onButtonColorReleased();
    void onColorSelected(const QColor &color);

};

//---

class ParameterAnalysis : public SignalPlotTool
{
    friend class ParameterAnalysisUpdater;

    Q_OBJECT
public:
    explicit ParameterAnalysis(QWidget *parent = 0);

private:
    QMutex *updaterLock;

    ParameterAnalysisUpdater *updater;

    MRun *currentMRun;
    MPoint *currentMPoint;

    BasePlotWidgetQwt* paramPlot;

    LabeledComboBox *comboboxSignalType1;
    LabeledComboBox *comboboxSignalType2;

    LabeledComboBox *comboboxParameter1;
    LabeledComboBox *comboboxParameter2;

    QComboBox *comboboxChannelX;
    QComboBox *comboboxChannelY;

    QCheckBox *autoPlotScaling;
    QCheckBox *checkboxLinkChannel;
    QCheckBox *checkboxUpdateOnSelection;
    QCheckBox *checkboxUpdateOnSignalChange;

    QProgressBar *progressBarUpdater;

    QPushButton *buttonAddAllChannel;

    QPushButton *buttonAddToPlot;
    QPushButton *buttonUpdateCurves;

    QTableWidget *tableCurves;
    ExtendedTableWidget *tableData;

    QToolButton *buttonUpdateWarning;

    QSplitter *splitterLeft;
    QSplitter *splitterRight;

    double xStart;
    double xEnd;

    double xStart1;
    double xEnd1;

    virtual void handleSignalDataChanged();
    virtual void handleSelectedRunsChanged(QList<MRun *> &runs);
    virtual void handleCurrentRunChanged(MRun *run);
    virtual void handleSelectedChannelsChanged(QList<int>& ch_ids);

    virtual void onToolActivation();

    QList<ParameterAnalysisCurve*> curves;

    ParameterAnalysisCurve* buildCurveObject(QString parameterX, QString parameterY, QVariant signalTypeX, QVariant signalTypeY, int channelX, int channelY);

    PlotAnalysisTool *patAvg1;
    PlotAnalysisTool *patAvg2;

    void updateChannelCount();
    void updateView();
    void updateCurveTable();
    void updateDataTable();
    void updatePlot();
    void updateColors();

    void updateParameterChoicesX();
    void updateParameterChoicesY();

    QString identifier_udp;
    QString identifier_settings;
    QString identifier_raw;
    QString identifier_absolute;
    QString identifier_temperature;

    QString identifier_laser_fluence;
    QString identifier_filter;
    QString identifier_avg_data_1;
    QString identifier_avg_data_2;
    QString identifier_integral_1;
    QString identifier_integral_2;
    QString identifier_fit_factor;
    QString identifier_pmt_gain_voltage;
    QString identifier_pmt_reference_voltage;
    QString identifier_mrun_list;

    const static QString identifier_splitterLeft;
    const static QString identifier_splitterRight;
    const static QString identifier_splitterMiddle;

    unsigned int curveCounter;

public slots:
    void onUpdaterProgress(int value);
    void onUpdaterFinished();

private slots:
    void onRangeSelected(double start, double end);

    void onComboboxCurrentIndexChanged();

    void onGuiSettingsChanged();

    void onButtonAddToPlotReleased();

    void onButtonAddAllChannelClicked();

    void onCheckboxLinkChannelStateChanged(int state);

    void onTableCurveDataCellChanged(int row, int column);

    void onPACVisibilityChanged();
    void onPACColorChanged();
    void onCheckboxAutoPlotScalingStateChanged(int state);

    void removeCurve(ParameterAnalysisCurve *curve);

    void updateCurves();

    void onSplitterMoved();

};

#endif // PARAMETERANALYSIS_H
