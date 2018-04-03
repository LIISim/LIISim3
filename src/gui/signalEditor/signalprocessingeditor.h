#ifndef SIGNALPROCESSINGVIEW_H
#define SIGNALPROCESSINGVIEW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QStackedWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QLabel>
#include <QToolBar>
#include <QSpinBox>
#include <QListWidget>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QDebug>
#include <QToolButton>
#include <QProgressBar>
#include <QRadioButton>
#include "gridablesplitter.h"
#include "../../models/datamodel.h"
#include "../utils/signalplotwidgetqwt.h"
#include "../dataItemViews/treeview/vistoggabletwi.h"
#include "../utils/materialcombobox.h"
#include "../utils/labeledcombobox.h"

class DataItemTreeView;
class MRun;
class SignalManager;
class MRunDetailsWidget;


/**
 * @brief The SignalProcessingEditor class. TODO: DOCUMENTATION
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class SignalProcessingEditor : public QWidget
{
    Q_OBJECT

    public:

        explicit SignalProcessingEditor(QWidget *parent = 0);

        /*~SignalProcessingEditor(){qDebug() << "~SignalProcessingEditor";}*/

        QToolBar* ribbonToolbar(){return m_ribbonToolbar;}

    private:

        DataModel* m_dataModel;
        SignalManager* m_sigManager;
        MRun* m_mrun;
        Signal::SType m_stype;

        int m_curMPoint;
        SignalPlotWidgetQwt* m_curPlot;

        bool m_mutePPTWIchanges;
        bool m_showUnprocessedSignalsRaw;
        bool m_showUnprocessedSignalsAbs;


        QList<bool> m_lastRawExpandedStatus;
        QList<bool> m_lastAbsExpandedStatus;
        QList<bool> m_lastTmpExpandedStatus;

        // COLORS
        QColor pcolor_green;
        QColor pcolor_blue;
        QColor pcolor_red;

        // ACTIONS
        QAction* actionPlotLink;

        // GUI ELEMENTS
        QVBoxLayout* layVmain;
        QVBoxLayout* layVbox;

        QSpinBox* sigSpin;

        // TABLEVIEW FOR MRUN DETAILS

        MRunDetailsWidget* mrunDetailsView;

        // DATAITEM TREEVIEWS
        DataItemTreeView* groupTree;
        DataItemTreeView* rawTree;
        DataItemTreeView* absTree;
        DataItemTreeView* tempTree;

        // PLOTS
        SignalPlotWidgetQwt* rawPlot;
        SignalPlotWidgetQwt* absPlot;
        SignalPlotWidgetQwt* tempPlot;

        // treeview items for unprocessed signal data
        VisToggableTWI* twiRawUnproc;
        VisToggableTWI* twiAbsUnproc;


        QLabel* runNameLabel;
        QLabel* sigCountLabel;

        // SPLITVIEWS
        GridableSplitter* mainHsplit;
        GridableSplitter* plotVsplit;
        GridableSplitter* plugVsplit;
        GridableSplitter* groupVsplit;
        QSplitter *leftVSplit;

        QToolBar* m_ribbonToolbar;

        // PRIVATE HELPERS
        void initToolBars();
        void plotCurrentPlugins();
        void replot();
        void updateItemColors();

        QList<QTreeWidgetItem*> mLastSelection;

        static const QString identifier_mainSplitter;
        static const QString identifier_plotSplitter;
        static const QString identifier_plugSplitter;
        static const QString identifier_treeDetailsSplitter;

    public slots:

         virtual void keyPressEvent(QKeyEvent* event);
         virtual void keyReleaseEvent(QKeyEvent* event);

    private slots:

        // SLOTS HANDLING USER INTERACTION
        void onMRunSelectionChanged(QList<QTreeWidgetItem *> selection, bool ignoreImport = false);
        void onSigIndexChanged(int idx);

        void onCalcToolboxRecalc(QList<Signal::SType> typeList);

        // SLOTS HANDLING DATAITEM/DATAMODEL CHANGES
        void onMRunAdded(MRun* mrun);
        void onMRunDestroyed();
        void onMRunDataChanged();
        void onMRunChannelCountChanged(Signal::SType stype, int count);

        // SLOTS HANDLING SIGNALMANAGER STATE CHANGES
        void handleImportStateChanged(bool state);
        void handleProcessingStateChanged(bool state);
        void handleExportStateChanged(bool state);

        // PLOT STUFF
        void onCurrentPlotToolChanged(BasePlotWidgetQwt::PlotZoomMode mode);
        void onXViewChanged(double xmin, double xmax);
        void onPlotLinkToggled(bool state);

        void onPlotTypeChanged(BasePlotWidgetQwt::PlotType type);

        void handlePPTreeItemModified(QTreeWidgetItem *item, int col);

        // HANDLE GLOBAL SETTINGS SIGNALS
        void onGuiSettingsChanged();

        void onSignalManagerImportFinished();

        void onSplitterMoved();
};

#endif // SIGNALPROCESSINGVIEW_H
