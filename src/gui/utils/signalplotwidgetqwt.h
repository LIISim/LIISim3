#ifndef SIGNALPLOTWIDGETQWT_H
#define SIGNALPLOTWIDGETQWT_H


#include "baseplotwidgetqwt.h"

//#include <QContextMenuEvent>
#include <QAction>
#include <QCheckBox>

#include <qwt_plot_zoneitem.h>
#include <qwt_plot_textlabel.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>

class SignalPlotCurve;
class SignalPlotIntervalCurve;

/**
 * @brief The SignalPlotWidgetQwt class
 * @ingroup GUI-Utilities
 * @details A Plotting Wiget for multiple signal data based on the
 * Qwt Widget Library (Qt Widgets for Technical Applications, http://qwt.sourceforge.net/)
 *
 * TODO: Add method for adding multiple signals, to avoid unnecessary updates (eg. datatable ...)
 *
 */
class SignalPlotWidgetQwt : public BasePlotWidgetQwt
{
    Q_OBJECT

    public:
        explicit SignalPlotWidgetQwt(QWidget *parent = 0);

        inline Signal::SType getCurrentSignalType() { return stype; }

        QColor getSignalColor(int idx);
        QColor getTemperatureColor(int idx);

    private:

        Signal::SType stype;

        /** @brief number of channels (needed for color generation) */
        int noChannels;

        /** @brief list of channel ids (needed for checkboxes etc...) */
        QList<int> channelIDs;

        QwtPlotMarker *triggerMark;


        // standard deviation curve
        QList<SignalPlotIntervalCurve*> stdev_curves;

        // list of checkboxes for channel visibility
        QAction* actionShowChannelCBs;
        QAction* actionShowStdev;
        QList<QCheckBox*> channelCBs;
        QGridLayout* channelCBlayout;

        bool actionShowChannelCBs_visible;

        QList<int> activeChannels;

        //color generator specific elements
        QList<QColor> predefinedColorsSignal;
        QList<QColor> predefinedColorsTemp;

        int cgChannelCounter;

    public slots:

        void setSignalType(Signal::SType stype);

        void setChannels(QList<int> chids);

        void addSignal(const Signal & signal, const QString &curveName, bool autoColor = true);

        void detachAllCurves();

        void setTriggerMarker(double xValue, double yValue);

        void setShowChannelCheckBoxes(bool state);
        void setActionShowChannelCBsVisible(bool state);

    private slots:

        QColor generateNewColor(const Signal &signal);

        void onContextMenuEvent(QPoint pos);

        void onActionShowChannelCBs(bool state);
        void onChannelVisCheckBoxToggled(bool state);

        void onActionShowStdev(bool state);

        void onShowDataTable();
};

#endif // SIGNALPLOTWIDGETQWT_H
