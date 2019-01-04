#ifndef SIGNALPLOTTOOL_H
#define SIGNALPLOTTOOL_H

#include "../atoolbase.h"

#include <QList>
#include <QGridLayout>
#include <QToolBar>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QSplitter>
#include <QCheckBox>

#include "../../dataItemViews/treeview/dataitemtreeview.h"
#include "../../signal/signal.h"
#include "../../utils/signalplotwidgetqwt.h"
#include "../../utils/labeledcombobox.h"

#include "sptplotitem.h"


/**
 * @brief The SignalPlotTool class implements the Analysistools base class.
 * The purpose of this tool is to create a plot of signals of different
 * measurement runs.
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class SignalPlotTool : public AToolBase
{
    Q_OBJECT
public:
    explicit SignalPlotTool(QWidget *parent = 0);

    virtual ~SignalPlotTool();

    virtual void handleSignalDataChanged();
    virtual void handleCurrentRunChanged(MRun* run);
    virtual void handleSelectedRunsChanged(const QList<MRun*>& runs);
    virtual void handleSelectedStypeChanged(Signal::SType stype);
    virtual void handleSelectedChannelsChanged(const QList<int> &ch_ids);

    virtual QList<QAction*> toolbarActions();

    virtual void onToolActivation();

protected:

    /// @brief map of SPTplot items. key: MRun ID
    QMap<int, SPTplotItem*> m_plotItems;

    // GUI elements

    QHBoxLayout* mainLayout;
    QSplitter* horizontalSplitter;
    QGridLayout* layoutRightGrid;
    SignalPlotWidgetQwt* plotter;

    QList<QCheckBox*> channelCheckboxes;

    QMap<int, QCheckBox*> channelToCheckbox;

    void updateChannelCheckboxes();
    QHBoxLayout *channelCheckboxesLayout;
    QToolBar *plotterToolbar;
    LabeledComboBox *comboboxSignalType;

    QWidget *plotterWidget;

    QList<int> channelIDs;

    void updateTempChannelTooltips();

signals:

    void mRunSelectionUpdated();

private slots:
    void onPlotTypeChanged(BasePlotWidgetQwt::PlotType type);
    void onGuiSettingsChanged();

    void onComboboxSignalTypeIndexChanged(int index);
    void onCheckboxChannelIDStateChanged(int state);

public slots:
    void updateMRunSelection();

};

#endif // SIGNALPLOTTOOL_H
