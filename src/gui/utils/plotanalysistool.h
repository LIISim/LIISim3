#ifndef PLOTANALYSISTOOL_H
#define PLOTANALYSISTOOL_H

#include <QObject>
#include <QPushButton>
#include <QList>

class QwtPlot;
class QwtPlotMarker;
class QwtPlotPicker;
class QwtPlotZoneItem;

#include "../../signal/signal.h"

#include "customQwtPlot/baseplotcurve.h"

class PlotAnalysisTool : public QObject
{
    Q_OBJECT

    public:
        explicit PlotAnalysisTool(QwtPlot* plot,
                              QList<BasePlotCurve*>* curves,
                              bool updateOnMove,
                              QObject *parent = 0);

        virtual ~PlotAnalysisTool();

        void setEnabled(bool enabled);
        void setVisible(bool visible);
        void setSignalType(Signal::SType signalType);

        QList<BasePlotCurve*>* getCurves() { return curves; }

        QwtPlot* plot;
        Signal::SType m_stype;
        QList<QwtPlotMarker*> indicators;

        // manual setting of marker position
        void setMarkers(double xStart, double xEnd);

        void setUpdateOnMove(bool state);

        void updateData();
        void updateInfoBox(QString labelText);

        virtual void updatePlot(double xStart, double xEnd);
        virtual void hideCustom() {}

    private:

        bool updateOnMove;

        QList<BasePlotCurve*>* curves;
        bool m_enabled;
        bool m_visible;        

        inline bool enabled(){ return m_enabled;}
        inline bool visible(){ return m_visible;}
        inline Signal::SType signalType(){ return m_stype;}

        QwtPlotPicker* rectPicker;
        QwtPlotMarker* xMarker1;
        QwtPlotMarker* xMarker2;
        QwtPlotZoneItem* zone;
        QPushButton* hide;


    signals:

        void dataSelected(double xStart, double xEnd);

        void hidden();

    public slots:

        // public slots (should be called when necessary!)
        void onCurveDataChanged();
        void onPlotResized(QResizeEvent *event);

    private slots:

        // handle user interaction
        void onRectPickerMoved(const QPointF &point);        
        void onHideButtonReleased();

        void onMouseReleaseUpdate();
        void onMouseClickUpdate();

        // handle QwtPlot signals
        void onXscaleChanged();
        void onYscaleChanged();
};

#endif // PLOTANALYSISTOOL_H
