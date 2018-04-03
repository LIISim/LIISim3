#include "sptplotitem.h"

#include <QDebug>

#include <qwt_plot.h>

#include "../../utils/signalplotwidgetqwt.h"
#include "../../general/LIISimException.h"
#include "../../utils/customQwtPlot/signalplotcurve.h"
#include "../../signal/mrungroup.h"

/**
 * @brief SPTplotItem::SPTplotItem Constructor
 * @param run measurement run which should be visualized
 * @param plot QwtPlot object where the curves should be shown
 * @param parent parent object
 */
SPTplotItem::SPTplotItem(MRun *run, SignalPlotWidgetQwt *plot, QObject *parent)
    :  DataItemObserverObject(run, parent)
{
    // init members
    m_run = run;
    m_selectedSigIdx = 0;
    m_plot = plot;
    m_selectedSType = Signal::RAW;

    QList<int> chids = m_run->channelIDs(m_selectedSType);

    for(int i = 0; i < chids.size(); i++)
    {
        addCurve(chids[i]);
    }
}


SignalPlotCurve *SPTplotItem::addCurve(int chid)
{
    QString title;

    if(m_selectedSType == Signal::TEMPERATURE)
        title = m_run->getName() + QString(" T%0 ").arg(chid);
    else
        title = m_run->getName() + QString(" Ch %0").arg(chid);

    QColor col = m_run->data(1).value<QColor>();

    if(m_run->group()->colorMap()->gray)
        col = m_run->group()->colorMap()->generateChannelColor(chid, col);
    else
        col = m_run->group()->colorMap()->generateChannelColor(chid, m_run->getNoChannels(m_selectedSType), col);

    /*else if(m_run->group()->colorMap()->getCurrentStyle() == ColorMap::STYLE::CUBEHELIX)
    {
        if((col.redF() + col.greenF() + col.blueF()) > 2.0)
            col = col.darker(100 + float(chid-1) * 85 / float(m_run->getNoChannels(m_selectedSType)));
        else
            col = col.lighter(70 + float(chid-1) * 70 / float(m_run->getNoChannels(m_selectedSType)));
    }
    else
    {
        // vary color for different channel ids
        float range = 100.0f; // higher values => darker color for higher channel-ids
        col = col.darker(70 + float(chid-1) * range/float(m_run->getNoChannels(m_selectedSType)));
    }*/

    SignalPlotCurve* curve = new SignalPlotCurve(title);

    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve->setPen(col,m_curveLineWidth);
    m_curves.insert(chid,curve);
    return curve;
}

/**
 * @brief SPTplotItem::~SPTplotItem Destructor, detaches all curves from plot and deletes them.
 */
SPTplotItem::~SPTplotItem()
{
    QList<SignalPlotCurve*> curves = m_curves.values();
    while(!curves.isEmpty())
    {
         SignalPlotCurve* curve = curves.takeFirst();
         m_plot->unregisterCurve(curve);
         if(!curve)
             continue;

         delete curve;
    }
}


/**
 * @brief SPTplotItem::setSelectedChannelIDs update the channel selection.
 * @param chIDs list of channel IDs which should be shown in plot
 * @details This method does NOT update the signal data of a curve (see SPTplotItem::updateCurveData())
 * nor tell the plot to update its curves (see QwtPlot::replot())!
 */
void SPTplotItem::setSelectedChannelIDs(const QList<int> &chIDs)
{
    this->m_selectedChIDs = chIDs;

    QSet<int> chidsToTest = m_curves.keys().toSet() + chIDs.toSet();
    QList<int> chidsToTestList = chidsToTest.toList();

    for(int i = 0; i < chidsToTestList.size(); i++)
    {
        int chid = chidsToTestList.at(i);

        if(m_selectedChIDs.contains(chid))
        {
            SignalPlotCurve* curve = m_curves.value(chid, 0);

            if(!curve)
                curve = addCurve(chid);

            m_plot->registerCurve(curve);
        }
        else
        {
            SignalPlotCurve* curve = m_curves.value(chid, 0);

            if(!curve)
                continue;

            m_plot->unregisterCurve(curve);
        }
    }
}


void SPTplotItem::setSelectedSigIndex(int index)
{
    m_selectedSigIdx = index;
}


/**
 * @brief SPTplotItem::setSelectedSignalType set the current signal type which should be shown in plot.
 * @param stype new signal type
 * @details This method does NOT update the signal data of a curve (see SPTplotItem::updateCurveData())
 * nor tell the plot to update its curves (see QwtPlot::replot())!
 */
void SPTplotItem::setSelectedSignalType(const Signal::SType &stype)
{
    m_selectedSType = stype;
    if(!m_run)return;

    // update channel id selection, based on available channel ids for signal type
    QList<int> avChIds = m_run->channelIDs(m_selectedSType);
    QList<int> newChIds;

    for(int i = 0; i < m_selectedChIDs.size(); i++)
    {
        int cid = m_selectedChIDs[i];
        if(avChIds.contains(cid))
            newChIds << cid;
    }
    if(newChIds != m_selectedChIDs)
        setSelectedChannelIDs(newChIds);

    onDataChanged(0, 0);
}


/**
 * @brief SPTplotItem::updateCurveData update the internal signal data for all active channel curves.
 * @details This method must be called explicitly!
 */
void SPTplotItem::updateCurveData()
{
    if(!m_run)
        return;

    try{

        MPoint* mp = m_run->getPost(m_selectedSigIdx);

        for(int i = 0; i < m_selectedChIDs.size(); i++)
        {
            int chid = m_selectedChIDs.at(i);
            BasePlotCurve* curve = m_curves.value(chid, 0);

            if(!curve)
                curve = addCurve(chid);

            Signal s = mp->getSignal(chid, m_selectedSType);

            // generate time axis!
            QVector<double> xData;
            for(int i = 0; i < s.data.size(); i++)
            {
                xData.push_back((s.start_time + i * s.dt) * 1e9);
            }
            curve->setSamples(xData, s.data);

        }
    }
    catch(LIISimException e)
    {
        qDebug() << e.what();
    }
}


/**
 * @brief SPTplotItem::onDataChanged handles the MRun::dataChanged() signal
 * @param pos position of modified data in datavector
 * @param value new value
 */
void SPTplotItem::onDataChanged(int pos, QVariant value)
{
    if(!m_run)
        return;

    for(int i = 0; i < m_run->getNoChannels(m_selectedSType); i++)
    {
         BasePlotCurve* curve = m_curves.value(i+1);
         if(!curve)
             continue;

         if(pos == 0) // runname changed
         {
             QString title;

             if(m_selectedSType == Signal::TEMPERATURE)
                title = m_run->getName() + QString(" T%0 ").arg(i+1);
             else
                title = m_run->getName() + QString(" Ch %0").arg(i+1);

             curve->setTitle(title);
         }
         else if(pos == 1) // color changed
         {
             QColor col = m_run->data(1).value<QColor>();

             col = m_run->group()->colorMap()->generateChannelColor(i+1, col);

             curve->setPen(col,m_curveLineWidth);
         }
    }

    if(pos == 2) //selected signal index changed
    {
        int newSigIdx = value.toInt();
        if( newSigIdx >= 0 && newSigIdx < m_run->sizeAllMpoints())
        {
            m_selectedSigIdx =  newSigIdx;
            updateCurveData();
        }
    }

    m_plot->handleCurveDataChanged();
}


/**
 * @brief SPTplotItem::onDataDestroyed handles the MRun::destroyed() signal.
 * @details Resets the internal data pointer to zero.
 */
void SPTplotItem::onDataDestroyed()
{
    m_run = 0;
    DataItemObserverObject::onDataDestroyed();

    //do not autodelete, let SignalPlotTool delete items
    //this->deleteLater();
}
