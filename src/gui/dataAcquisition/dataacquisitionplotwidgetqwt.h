#ifndef DATAACQUISITIONPLOTWIDGETQWT_H
#define DATAACQUISITIONPLOTWIDGETQWT_H

#include "../utils/baseplotwidgetqwt.h"

//#include <QContextMenuEvent>
#include <QAction>
#include <QCheckBox>

#include <qwt_plot_zoneitem.h>
#include <qwt_plot_textlabel.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>

class SignalPlotCurve;
class StreamPoint;
class DA_ReferenceElementWidget;

class DataAcquisitionPlotWidget : public BasePlotWidgetQwt
{
    Q_OBJECT

    public:
        enum ViewMode
        {
            STATIC,
            STREAM
        };

        explicit DataAcquisitionPlotWidget(QWidget *parent = 0);

    private:

        Signal::SType stype;

        /** @brief number of channels (needed for color generation) */
        int noChannels;

        /** @brief list of channel ids (needed for checkboxes etc...) */
        QList<int> channelIDs;

        QwtPlotMarker *triggerMark;

        // list of checkboxes for channel visibility
        QAction* actionShowChannelCBs;
        QList<QCheckBox*> channelCBs;
        QGridLayout* channelCBlayout;

        bool actionShowChannelCBs_visible;

        QList<SignalPlotCurve*> static_curves;
        QList<SignalPlotCurve*> stream_curves;
        QList<SignalPlotCurve*> reference_curves;

        ViewMode viewMode;

        QList<int> streamPlotOrder;

    public slots:

        void setSignalType(Signal::SType stype);

        void setViewMode(ViewMode mode);

        void setChannels(QList<int> chids);

        void addSignal(const Signal & signal, const QString &curveName, bool autoColor = true);

        void updateStreamPoint(StreamPoint *point);

        void updateReferences(const QList<QPair<DA_ReferenceElementWidget*, Signal>> references);

        void detachAllCurves();

        void setTriggerMarker(double xValue, double yValue);

        void setShowChannelCheckBoxes(bool state);
        void setActionShowChannelCBsVisible(bool state);

        void switchStreamSignalLayer(unsigned int curve, bool up);

        void addStaticSignal(const Signal &signal, const QString &curveName, bool autoColor = true);

        void detachStaticCurves();

protected slots:
    void addStreamSignal(const Signal &signal, const QString &curveName, QColor curveColor);
    void addReferenceSignal(const Signal &signal, const QString &curveName, QColor curveColor);


private slots:

    QColor generateNewColor(const Signal &signal);

    void onContextMenuEvent(QPoint pos);

    void onActionShowChannelCBs(bool state);
    void onChannelVisCheckBoxToggled(bool state);
};

#endif // DATAACQUISITIONPLOTWIDGETQWT_H
