#ifndef ATOOLBASE_H
#define ATOOLBASE_H

#include <QWidget>
#include <QMap>
#include <QColor>
#include <QList>
#include <QString>
#include <QToolBar>
#include "../../signal/signal.h"
#include "../../general/LIISimException.h"

class SignalManager;
class MRun;

/**
 * @brief The AToolBase class serves as an abstract base class for several
 * measurement analysis GUIs.
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class AToolBase : public QWidget
{
    Q_OBJECT


public:
    explicit AToolBase(QWidget *parent = 0);

    virtual ~AToolBase(){}

    QString title();


    /**
     * @brief handleSignalDataChanged defines what the GUI should
     * do when signal data has been modified
     * Subclasses have to implement this method.
     */
    virtual void handleSignalDataChanged() = 0;

    /**
     * @brief handleCurrentRunChanged defines what the GUI should
     * do when the currently highlighted MRun has changed.
     * Subclasses have to implement this method.
     * @param run selected MRun object
     */
    virtual void handleCurrentRunChanged(MRun* run) = 0;

    /**
     * @brief handleSelectedRunsChanged defines what the gui should
     * do when the currently selected MRuns changed.
     * Subclasses have to implement this method.
     * @param runs List of MRun-IDs
     */
    virtual void handleSelectedRunsChanged(const QList<MRun*>& runs) = 0;

    /**
     * @brief handleSelectedStypeChanged defines what the gui should
     * do when the currently selected signal type changed.
     * Subclasses have to implement this method.
     * @param stype new Signal::SType
     */
    virtual void handleSelectedStypeChanged(Signal::SType stype) = 0;

    /**
     * @brief handleSelectedChannelsChanged defines what the gui should
     * do when the Channel selection changed.
     * Subclasses have to implement this method.
     * @param ch_ids List of selected Channel-IDs
     */
    virtual void handleSelectedChannelsChanged(const QList<int>& ch_ids) = 0;

    /**
     * @brief toolbarActions returns a list of
     * QActions which will be added to the AnalysisTools
     * Toolbar
     * Subclasses have to implement this method.
     * @return
     */
    virtual QList<QAction*> toolbarActions() = 0;

    virtual QIcon icon();

    virtual void onToolActivation() = 0;

    void activateTool();
    void deactivateTool();

    MRun* currentRun();
    QList<MRun*> selectedRuns();
    QList<int> selectedRunIds();
    Signal::SType selectedSignalType();
    QList<int> selectedChannelIds();

protected:

    /// @brief title (shown in tabbar)
    QString m_title;

    /// @brief path to icon file
    QString m_iconLocation;

private:

    int m_currentRunId;
    QList<int> m_selectedRunIds;
    Signal::SType m_selectedStype;
    QList<int> m_selectedChannelIds;
    bool m_toolActive;

signals:
    void signalTypeChanged(Signal::SType signalType);
    void channelIDsChanged(QList<int> channelIDs);

public slots:

    void onCurrentRunChanged(int run_id);
    void onSelectedRunsChanged(QList<int>& run_ids);
    void onSelectedStypeChanged(Signal::SType stype);
    void onSelectedChannelsChanged(QList<int>& ch_ids);

};

#endif // ATOOLBASE_H
