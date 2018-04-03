#ifndef DATAACQUISITIONWINDOW_H
#define DATAACQUISITIONWINDOW_H

#include <QWidget>
#include <QToolBar>
#include <QGridLayout>
#include <QSplitter>
#include <QAction>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QWidgetAction>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>

#include "./models/dataitem.h"
#include "../dataItemViews/treeview/dataitemtreeview.h"
#include "../utils/signalplotwidgetqwt.h"
#include "./settings/picoscopesettings.h"
#include "./io/picoscope.h"
#include "./models/datamodel.h"
#include "./signal/mrungroup.h"
#include "./signal/mrun.h"
#include "./signal/mpoint.h"
#include "../utils/liisettingscombobox.h"
#include "./gui/utils/ribbontoolbox.h"
#include "picoscopesettingswidget.h"
#include "da_exportsettingswidget.h"
#include "da_runsettingswidget.h"
#include "../utils/mrundetailswidget.h"
#include "signal/mrungroup.h"
#include "signal/streampoint.h"
#include "da_referencewidget.h"
#include "da_blocksequence.h"
#include "dataacquisitionplotwidgetqwt.h"

#define TEST_2

class DataAcquisitionWindow : public QWidget
{
    friend class DA_BlockSequenceWorker;

    Q_OBJECT

public:
    DataAcquisitionWindow(QWidget *parent = 0);
    ~DataAcquisitionWindow();

    QToolBar* ribbonToolbar() { return rtbRibbonToolbar; }

private:

    bool isBusy = false;

    bool autoStartStreaming = true;

    /* Toolbar elements */
    QToolBar *rtbRibbonToolbar;

    QAction *actionLoadSettings;
    QAction *actionSaveSettings;

    QAction *actionLoadGain;

    QAction *actionOpenDevice;
    QAction *actionCloseDevice;

    RibbonToolBox *rbtStreamingMode;
    QAction *actionStartStreaming;
    QAction *actionStopStreaming;
    QAction *actionClearStreamingAvg;
    QLabel *avgCounterLabel;

    QGroupBox *groupStreamingMode;
    QButtonGroup *buttongroupStreamingMode;
    QRadioButton *rbStreamingSingleMeasurement;
    QRadioButton *rbStreamingAverage;
    QRadioButton *rbStreamingBoth;
    QSpinBox *spinboxAveragingBufferSize;
    QPushButton *buttonOverflow;

    QLabel *labelRunName;
    QSpinBox *spinboxSignal;
    QLabel *labelSignalCount;

    QLabel *labelConnection;
    QPushButton *buttonError;
    QLabel *labelStreaming;
    QLabel *labelTriggered;
    QLabel *labelRunningBlock;
    QLabel *labelProcessing;
#ifdef LIISIM_NIDAQMX
    QAction *actionShowDeviceManager;
    QLabel *labelAnalogOutputState;
    QLabel *labelAnalogInputState;
    QLabel *labelDigitalOutputState;
    QToolButton *toolbuttonStartAnalogOutput;
    QToolButton *toolbuttonStopAnalogOutput;
    QToolButton *toolbuttonStartAnalogInput;
    QToolButton *toolbuttonStopAnalogInput;
    QToolButton *toolbuttonStartDigitalOutput;
    QToolButton *toolbuttonStopDigitalOutput;

    QToolButton *toolbuttonDigitalOut1;
    QToolButton *toolbuttonDigitalOut2;
    QToolButton *toolbuttonDigitalOut3;
    QToolButton *toolbuttonDigitalOut4;
    QToolButton *toolbuttonDigitalOut5;
    QToolButton *toolbuttonDigitalOut6;
    QToolButton *toolbuttonDigitalOut7;
    QToolButton *toolbuttonDigitalOut8;
    QToolButton *toolbuttonDigitalOut9;
#endif
    QAction *actionPlotPan;
    QAction *actionPlotRectZoom;
    QAction *actionPlotZoomReset;

    QLabel *labelActiveTriggers;
    QAction *actionChangeTrigger;

    DA_ReferenceWidget *referenceWidget;

    /* Window elements */
    QHBoxLayout *hbLayout;
    QSplitter *hSplitter;
    DataItemTreeView *dataTreeView;
    MRunDetailsWidget *mrunDetailsView;

    PicoScopeSettingsWidget* psSettingsWidget;
    DA_ExportSettingsWidget* exportSettingsWidget;
    DA_RunSettingsWidget *runSettingsWidget;

    QGridLayout *gLayout;

    LIISettingsComboBox *cbliisettings;

    QPushButton *pbRunBlockSequence;
    QPushButton *pbRunBlock;
    QPushButton *pbRunCBuffer;

    /* Graph */
    //SignalPlotWidgetQwt *signalPlot;
    DataAcquisitionPlotWidget *signalPlot;

    /* Data */
    DataModel *dataModel;
    MRun *selectedRun;
    PicoScopeSettings *picoscopeSettings;

    //Device
    PicoScope *picoscope;

    //Helper
    void updateToSelectedRun();

    void scalePlotStreaming();

    DA_BlockSequenceWorker blockSequenceWorker;

    QList<int> runIdsToPlotWhileStreaming;
    bool runIdsToPlotWhileStreamingChanged;

signals:
    void blockAcquisitionFinished();

public slots:
    void onGuiSettingsChanged();

private slots:
    //Toolbar ui slots
    void openDevice(bool checked);
    void closeDevice(bool checked);
    void startStreaming(bool checked);
    void stopStreaming(bool checked);
    void onAveragingBufferSizeChanged(int value);

    void plotZoomSelection();
    void errorClicked(bool clicked);
    void signalSpinnerChanged(int value);
    void onChangeTriggerClicked(bool clicked);

    void onStreamingModeToggled(QAbstractButton *button, bool state);

    void onPlotTypeChanged(BasePlotWidgetQwt::PlotType type);

    //Settings ui slots
    void liisettingschanged(int index);

    //Other
    void treeViewChanged(QList<QTreeWidgetItem*> selection);
    void onTreeViewVisualEnabledItemsChanged(QList<QTreeWidgetItem*> visualEnabledItems);

    void runBlock();
    void runBlockSequence();
    void saveCircularBuffer();
    void processBlockReady();
    void processBlockProcessingFinished(MRun *run);

    void onPsSettingsObjChanged();

    void handleProcessingProgress(float percent);

    void handlePicoScopeErrorMsg(QString message);
    void handlePicoScopeInfoMsg(QString message);

    void handlePicoScopeOpenStatusChanged(bool open);
    void handlePicoScopeStreamingStatusChanged(bool streaming);
    void handlePicoScopeTriggeredStatusChanged(bool triggered);
    void handlePicoScopeBlockrunStatusChanged(bool blockrun);
    void handlePicoScopeProcessingStatusChanged(bool processing);
    void handlePicoScopeErrorStatusChanged(bool error);

    void onStreamingData(StreamPoint point);
    void onReferenceRequest();

    void onBlockSequenceWorkerStopped();
    void onBlockSequenceWorkerStatusChanged(QString status);
#ifdef LIISIM_NIDAQMX
    void onShowDeviceManager();

    void onDigitalOutTriggered();
    void onDigitalOutputChanged();

    void onAnalogInputStateChanged(bool enabled);
    void onAnalogOutputStateChanged(bool enabled);
    void onDigitalOutputStateChanged(bool enabled);
#endif
};

#endif // DATAACQUISITIONWINDOW_H
