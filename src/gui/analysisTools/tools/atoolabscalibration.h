#ifndef ATOOLABSCALIBRATION_H
#define ATOOLABSCALIBRATION_H


#include "./signalplottool.h"

#include <QTableWidget>

#include "../../utils/spectracombobox.h"

#include "../../utils/baseplotwidgetqwt.h"

class AToolAbsCalibration : public SignalPlotTool
{
    Q_OBJECT

    public:
        explicit AToolAbsCalibration(QWidget *parent = 0);

    private:

        MRun *currentMRun;
        MPoint *currentMPoint;

        Spectrum refSpectrum;

        BasePlotWidgetQwt* calPlot;

        QTableWidget *calResultTableWidget;
        QTableWidget *calRightSettingsTableWidget;
        QTableWidget *calSpectraTableWidget;


        SpectraComboBox *cbSpectra;

        PlotAnalysisTool *plotAnalysisTool;

        void processDataInterval();

        double xStart;
        double xEnd;

        virtual void handleSelectedRunsChanged(QList<MRun *> &runs);

        virtual void onToolActivation();

    public slots:

        void onMRunSelectionUpdated();

    private slots:

         void onRangeSelected(double xstart, double xend);

         void onClearPlotButtonReleased();

         void refSpectrumChanged(int index);

         void onGuiSettingsChanged();

         void copyToClipboard();

};

#endif // ATOOLABSCALIBRATION_H
