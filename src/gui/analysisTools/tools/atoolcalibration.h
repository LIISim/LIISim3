#ifndef ATOOLCALIBRATION_H
#define ATOOLCALIBRATION_H

#include "./signalplottool.h"

#include <QList>
#include <QGridLayout>
#include <QToolBar>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QSplitter>
#include <QTableWidget>

#include "../../dataItemViews/treeview/dataitemtreeview.h"
#include "../../signal/signal.h"
#include "../../utils/baseplotwidgetqwt.h"
#include "../../utils/labeledcombobox.h"
#include "../../utils/liisettingscombobox.h"

#include "sptplotitem.h"

class NumberLineEdit;

class AToolCalibration : public SignalPlotTool
{
    Q_OBJECT

    public:
        explicit AToolCalibration(QWidget *parent = 0);

    private:

        BasePlotWidgetQwt* calPlot;

        QTableWidget *calSettingsTableWidget;
        QTableWidget *calResultTableWidget;

        QCheckBox *checkBoxManAdd;
        QCheckBox *checkBoxShowFit;
        QCheckBox *checkBoxShowCurrentCal;
        LIISettingsComboBox *cbliisettings;
        QCheckBox *checkBoxNormalize;
        NumberLineEdit *input_norm_ref;

        PlotAnalysisTool *plotAnalysisTool;

        void processDataInterval();

        double xStart;
        double xEnd;

        BasePlotCurve* getCurrentCalibrationCurve(int chID);

        virtual void onToolActivation();

    public slots:
        void onMRunSelectionUpdated();

    private slots:

        void onRangeSelected(double xstart, double xend);

        void onClearPlotButtonReleased();
        void onAddPlotButtonReleased();

        void copyToClipboard();

        void onChManAddStateChanged(int state);
        void liisettingschanged(int index);
        void onGuiSettingsChanged();

        void onPlotTypeChanged(BasePlotWidgetQwt::PlotType type);
};

#endif // ATOOLCALIBRATION_H
