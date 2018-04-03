#ifndef ATOOLTEMPERATUREFIT_H
#define ATOOLTEMPERATUREFIT_H

#include "./signalplottool.h"
#include "../../core.h"

#include <QObject>
#include <QTableWidget>

#include "../../utils/baseplotspectrogramwidgetqwt.h"

class NumberLineEdit;

class AToolTemperatureFit : public SignalPlotTool
{
    Q_OBJECT

    public:
        explicit AToolTemperatureFit(QWidget *parent = 0);

    private:
        MRun *currentMRun;
        MPoint *currentMPoint;
        Material material_spec;

        int tempChannelID;
        QList<int> availableTChannels;

        bool statAutoX;
        bool statAutoY;
        bool statAutoZ;

        int statResX;
        int statResY;

        BasePlotWidgetQwt *fitPlot;
        BasePlotWidgetQwt *resultPlot;
        BasePlotWidgetQwt *covarPlot;
        BasePlotCurve *curve_fit;

        BasePlotSpectrogramWidgetQwt *statisticsPlot;
        BasePlotCurve *curve_stats;
        BasePlotCurve *curve_stats_init;

        QTabWidget *fitResultTabWidget;

        QTableWidget *fitResultTableWidget;

        QComboBox *cbResultsType;
        QComboBox *cbShowResults;
        QComboBox *cbTChannel;
        QCheckBox *checkBoxBPAnalysis;

        QCheckBox *checkBoxStatAutoX;
        QCheckBox *checkBoxStatAutoY;
        QCheckBox *checkBoxStatAutoZ;

        NumberLineEdit *input_xmin;
        NumberLineEdit *input_xmax;
        NumberLineEdit *input_ymin;
        NumberLineEdit *input_ymax;
        NumberLineEdit *input_zmin;
        NumberLineEdit *input_zmax;
        NumberLineEdit *input_resolutionX;
        NumberLineEdit *input_resolutionY;
        QPushButton *buttonUpdateStatSettings;

        QPointF plot_selection;
        int it_selection;

        // container for current fit results
        QList<FitIterationResult> fir;

        QComboBox *comboboxMRun;

        QSplitter *verticalSplitterLeft;
        QSplitter *verticalSplitterRight;
        QSplitter *bottomRightSplitter;

        static const QString identifier_splitterLeftV;
        static const QString identifier_splitterRightV;
        static const QString identifier_bottomSplitterH;
        static const QString identifier_splitterMiddleH;

        void plotTemperatureFit(double T, double C, Material mat, QString sourceEm,
                                QString name, QPen pen = QPen(Qt::black, 1, Qt::DashLine));
        void plotResultsIterations(int idx);
        void plotResultsFinal(int idx);
        void plotCovariance();

        void processDataInterval();

        Signal getCurrentTChannel(MPoint *mp);

        virtual void handleSignalDataChanged();
        virtual void handleSelectedRunsChanged(QList<MRun *> &runs);
        virtual void handleSelectedChannelsChanged(QList<int>& ch_ids);
        virtual void handleSelectedStypeChanged(Signal::SType stype);

        virtual void onToolActivation();

        QList<int> addedRows;

    signals:

    public slots:

    private slots:
        void onPointSelected(const QString, const QColor,
                             const QPointF & markerPosition);

        void onCellClicked(int row, int col = 0);

        void scrollResultsUp();
        void scrollResultsDown();

        void onClearPlotButtonReleased();
        void onResetPlotButtonReleased();

        void onFitResultTabChanged(int idx);

        void onCbResultsTypeChanged(int idx);
        void onCbShowResultsChanged(int idx);
        void onCbTChannelChanged(QString idx);
        void onCheckBoxBPAnalysisSelected(int state);
        void onCheckBoxStatAutoX(int state);
        void onCheckBoxStatAutoY(int state);
        void onCheckBoxStatAutoZ(int state);

        void onInputZChanged();
        //void onInputAxesChanged();
        //void onInputResolutionChanged();
        void onRangeChangedZ(double, double);

        void onStatisticsPlotClicked(double x, double y);

        void onXViewChanged(double xmin, double xmax);

        void keyReleaseEvent(QKeyEvent *event);

        void onComboboxMRunIndexChanged();
        void onButtonUpdateSettingsClicked();

        void onSplitterMoved();

        void onGuiSettingsChanged();

};

#endif // ATOOLTEMPERATUREFIT_H
