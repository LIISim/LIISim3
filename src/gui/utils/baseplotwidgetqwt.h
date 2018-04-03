#ifndef BASEPLOTWIDGETQWT_H
#define BASEPLOTWIDGETQWT_H

#include <qwt_plot.h>

// text
#include <qwt_plot_textlabel.h>

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QAction>
#include <QList>

// color
#include "colormap.h"

// tools
#include "plotavgtool.h"
#include "plotfittool.h"
#include "plotanalysistool.h"

// plot
class QwtPlot;
class BasePlotCurve;
class CustomPlotOptionsWidget;

// legend
class QwtPlotLegendItem;

// data table
class DataTableWidget;

// mouse controls
class QwtPlotPanner;
class CustomPlotMagnifier;
class CustomPlotPicker;
class QwtPlotMarker;
class PlotZoomer;

class BasePlotWidgetQwt : public QWidget
{
    Q_OBJECT

    public:
        explicit BasePlotWidgetQwt(QWidget *parent = 0);

        enum PlotZoomMode{
            PLOT_PAN,
            DATA_CURSOR,
            ZOOM_RECT,
            ZOOM_RESET,
            AVG_RECT,
            FIT_RECT        
        };

        enum PlotType {
            LINE_CROSSES,
            LINE,
            DOTS_SMALL,
            DOTS_MEDIUM,
            DOTS_LARGE
        };

        inline QwtPlot* plot(){ return qwtPlot; }

        PlotZoomMode zoomMode(){ return zmode; }

        QList<QAction*> toolActions();

        QAction* toolAction(PlotZoomMode mode);
        QAction* toolActionDataCursor();
        void addPlotAnalyzer(QString name, QString tooltip, bool enabled, bool updateOnMove);

        PlotAnalysisTool* addPlotAnalysisTool(QString name, QString tooltip, bool updateonMove);

        virtual QList<BasePlotCurve*> getCurves(){ return curves; }

        void addText(QString text);
        void appendText(QString text);
        void clearText();

        void setPlotType(PlotType type);

        ~BasePlotWidgetQwt();


        void setPlotLabelText(QString text);
        void setPlotLabelAlignment(int alignment);
        void plotTextLabelVisible(bool visible);

    protected:

        // plot
        QwtPlot* qwtPlot;

        /** @brief font for plot title */
        QFont titleFont;
        /** @brief font for axis units and titles */
        QFont axisFont;

        PlotType plotType;
        void updateToPlotType();

        QwtPlotTextLabel *titleItem;

        // curves
        QList<BasePlotCurve*> curves;
        BasePlotCurve* selectedCurve;
        bool selectClosestCurve(const QPoint& point);

        /**
         * @brief currentColor current color used for signal plotting.
         * This color is only used if no autoColor is used when adding a signal!
         */
        QColor currentColor;
        ColorMap colmap;

        /** @brief line width of signal curve */
        double curveLineWidth;
        /** @brief render signal curved antialiased if true */
        bool   curveRenderAntialiased;

        // axis
        bool linkXscale;
        bool linkYscale;

        // legend
        /** @brief legend display */
        QwtPlotLegendItem *legendItem;

        // layout
        QGridLayout *layMain;
        //CustomPlotOptionsWidget* optionsWidget;
        QHBoxLayout *bottomBarLayout;

        // plot data cursor
        bool dataCursorEnabled;
        int dataCursorSampleIndex;
        QLabel *dataCursorColorLabel;
        QLabel *dataCursorTextLabel;
        void setDataCursorPosition(const QPointF& position);
        QwtPlotMarker *pointMarker;

        // data table
        DataTableWidget* dataTableWindow;

        // mouse controls
        PlotZoomMode zmode;

        QwtPlotPanner *z_panner;
        //QwtPlotZoomer *z_rectZoomer;
        PlotZoomer *z_rectZoomer;

        CustomPlotMagnifier *z_mag;
        CustomPlotMagnifier *z_mag_x;
        CustomPlotMagnifier *z_mag_y;

        CustomPlotPicker *z_dataCursor;
        PlotAvgTool* avgTool;
        PlotFitTool* fitTool;
        PlotAnalysisTool* plotAnalysisTool;
        QList<PlotAnalysisTool*> listPlotAnalysisTools;
        QList<QAction*> listPlotAnalysisToolActions;

        // tool actions

        void addToolAction(QAction *action, bool checkable);

        QActionGroup *toolActionGroup;

        QAction* toolAction_PlotPan;        
        QAction* toolAction_DataCursor;
        QAction* toolAction_RectZoom;
        QAction* toolAction_ZoomReset;
        QAction* toolAction_FitTool;
        QAction* toolAction_AvgTool;
        QAction* toolAction_PlotAnalysisTool;

        // context menu actions
        QAction* actionChangeScaleTypeLogX;
        QAction* actionChangeScaleTypeLogY;

        QActionGroup* plotTypeActions;
        QAction* actionShowDataPoints;
        QAction* actionShowDataDotsSize1;
        QAction* actionShowDataDotsSize2;
        QAction* actionShowDataDotsSize3;
        QAction* actionShowDataLine;

        QAction* actionShowDataTable;
        QAction* actionShowLegend;

        QwtPlotTextLabel *plotLabel;
        QwtText *plotlabelText;

        QString dataTableRunName;
        QString dataTableToolName;

    private:
        QList<QAction*> m_actions;

    signals:

        /**
         * @brief curveDataChanged This signal is emitted when the number of curves
         * or the data of a curve has been changed. (also see handleCurveDataChanged())
         */
        void curveDataChanged();

        /**
         * @brief xViewChanged is emitted if x-axis-scale linking is enabled
         * and the current x-axis-scaling has been edited
         * @param xmin new minimum x-value
         * @param xmax new maximum x-value
         */
        void xViewChanged(double xmin, double xmax);

        /**
         * @brief yViewChanged is emitted if y-axis-scale linking is enabled
         * and the current y-axis-scaling has been edited
         * @param ymin new minimum y-value
         * @param ymax new maximum y-value
         */
        void yViewChanged(double ymin, double ymax);

        /**
         * @brief currentToolChanged This signal is emitted when
         * the user changed the current Plot tool
         * @param mode new Plot tool
         */
        void currentToolChanged(BasePlotWidgetQwt::PlotZoomMode mode);

        /**
         * @brief dataSampleMarkerMoved This signal is emitted if the signal data marker has been moved.
         * @param curveTitle title of current curve
         * @param curveColor color of current curve
         * @param markerPosition current position of marker
         */
        void dataSampleMarkerMoved(const QString & curveTitle,
                                   const QColor & curveColor,
                                   const QPointF & markerPosition);

        /**
         * @brief rangeSelected when plotAnalysisTool range selection is finished
         * @param xStart
         * @param xEnd
         */
        void rangeSelected(double xStart, double xEnd);

        void plotTypeChanged(BasePlotWidgetQwt::PlotType type);

    public slots:

        // curves
        void handleCurveDataChanged(bool replot = true);
        void registerCurve(BasePlotCurve *curve);
        void unregisterCurve(BasePlotCurve *curve);
        virtual void detachAllCurves();

        // plot
        void setPlotTitle(const QString & plotTitle);
        void setPlotAxisTitles(const QString & xTitle, const QString & yTitle);

        // scaling
        void setXView(double xmin, double xmax);
        void setYView(double ymin, double ymax);
        void setView(double xmin, double xmax, double ymin, double ymax);
        QRectF getView();
        void setEnabledXViewLink(bool e);
        void setEnabledYViewLink(bool e);
        void setXLogScale(bool e);
        void setYLogScale(bool e);


        // legend
        void setMaxLegendColumns(int cols);

        // color
        void setCurrentColor(QColor color);

        // tools: general
        void setZoomMode(PlotZoomMode zMode);        
        void resizeEvent(QResizeEvent *event);
        void keyPressEvent(QKeyEvent *event);

        // tools: data cursor
        void toggleDataCursor();

        void setDataTableRunName(QString name);
        void setDataTableToolName(QString name);

    protected slots:

        // plot
        void onXscaleChanged();
        void onYscaleChanged();

        // tools
        void onToolActionTriggered();
        void onActionPlotAnalysisTool();

        void onActionPlotAnalysisToolTriggered();

        // tools: data cursor
        void onDataCursorSelection(const QPoint &point);

        // context menu
        virtual void onContextMenuEvent(QPoint pos);
        virtual void onShowDataTable();
        void onActionShowLegend(bool show);

        void onChangeScaleTypeLogX(bool log);
        void onChangeScaleTypeLogY(bool log);

        void onPlotTypeActionTriggerd(QAction* action = 0);

};

#endif // BASEPLOTWIDGETQWT_H
