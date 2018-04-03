#ifndef SPTPLOTITEM_H
#define SPTPLOTITEM_H

#include <QObject>
#include <QMap>

#include "../../models/dataitemobserverobject.h"
#include "../../signal/mrun.h"


class SignalPlotWidgetQwt;
class SignalPlotCurve;


/**
 * @brief The SPTplotItem class manages a list of SignalPlotCurve(QwtPlotCurve) for each channel
 * of a certain measurement run.
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class SPTplotItem : public DataItemObserverObject
{
    Q_OBJECT
public:

    explicit SPTplotItem(MRun* run,SignalPlotWidgetQwt* plot,QObject *parent = 0);

    ~SPTplotItem();

    void setSelectedChannelIDs(const QList<int> & chIDs);
    void setSelectedSigIndex(int index);
    void setSelectedSignalType(const Signal::SType & stype);

    void updateCurveData();

private:

    /// @brief internal data pointer
    MRun* m_run;

    /// @brief map of curves for each channel
    QMap<int,SignalPlotCurve*> m_curves;

    /// @brief list of channel IDs which should be visible in plot
    QList<int> m_selectedChIDs;

    /// @brief line width of curve in plot
    double m_curveLineWidth = 1.5;

    /// @brief index of selected MPoint
    int m_selectedSigIdx;

    /// @brief signal type which should be shown in plot
    Signal::SType m_selectedSType;

    /// @brief parent plot for curves
    SignalPlotWidgetQwt* m_plot;

    SignalPlotCurve* addCurve(int chid);

signals:

public slots:

protected slots:

    void onDataChanged(int pos, QVariant value);
    void onDataDestroyed();
};

#endif // SPTPLOTITEM_H
