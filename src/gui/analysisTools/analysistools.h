#ifndef ANALYSISTOOLS_H
#define ANALYSISTOOLS_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QStringList>
#include <QList>
#include <QTreeWidgetItem>
#include <QStatusBar>
#include <QMenuBar>
#include <QStackedWidget>
#include <QToolBar>
#include <QActionGroup>


#include "../utils/ribbontoolbox.h"
#include "atoolbase.h"
#include "../dataItemViews/treeview/dataitemtreeview.h"
#include "../../general/LIISimException.h"
#include "../utils/calculationtoolbox.h"
#include "../utils/mrundetailswidget.h"

class Core;
class SignalManager;

/**
 * @brief The AnalysisTools class provides a GUI for the visualization
 * of measurement run statistics.
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class AnalysisTools : public QWidget
{
    Q_OBJECT

    public:
        explicit AnalysisTools(QWidget *parent = 0);

        ~AnalysisTools();

        QStatusBar* statusBar(){return new QStatusBar();}

        QToolBar* ribbonToolbar(){return m_ribbontoolbar;}

    private:
        QStackedWidget* atoolstack;
        DataItemTreeView* treeView;
        MRunDetailsWidget* runDetails;

        QSplitter* verticalSplitterLeft;
        QSplitter* horizontalSplitter;

        QList<AToolBase*> aToolList;

        QActionGroup* atoolActions;
        RibbonToolBox* rtbView;
        RibbonToolBox* rtbTool;

        QToolBar* m_ribbontoolbar;

        QLabel* labelToolIcon;
        QLabel *labelToolName;

        QHBoxLayout* m_checkBoxLayout;
        QToolBar* m_internalToolbar;
        QList<QCheckBox*>* checkboxes;
        QList<int> checkboxidx_to_chid;
        Signal::SType m_selectedSignalType;
        QList<int> m_selectedChannelIds;

        QList<int> m_lastChannelIDsRawAbs;
        QList<int> m_lastChannelIDsTemp;

        QWidget *selectionWidget;

        MRun* currentRun;

        CalculationToolbox *calcToolbox;

        bool firstRun;

        void addAtool(AToolBase* atool);

        void updateChannelCheckboxes();

        static const QString _identifierSplitterH;
        static const QString _identifierSplitterV;

    signals:

        void currentRunChanged(int run_id);
        void selectedRunsChanged(QList<int>& run_ids);
        void selectedStypeChanged(Signal::SType stype);
        void selectedChannelIdsChanged(QList<int>& ch_ids);

    private slots:

        // HANDLE PROGRAM STATE
        void handleSignalDataChanged(bool state);

        // HANDLE USER INTERACTION
        void onAtoolActionTriggered(QAction* action);
        void onTreeViewSelectionChanged(QList<QTreeWidgetItem*> selection);
        void onTreeViewCurrentItemChanged();
        void onSignalTypeSelectionChanged(int);
        void onChannelSelectionChanged();

        void onGUISettingsChanged();

        void onCalcToolboxRecalc(QList<Signal::SType> typeList);

        void onSplitterMoved();

public slots:
    void changeSignalType(Signal::SType signalType);
    void changeChannelIDs(QList<int> channelIDs);
};

#endif // ANALYSISTOOLS_H
