#ifndef FITCREATOR_H
#define FITCREATOR_H

#include <QWidget>
#include <QAction>

#include <QList>
#include <QGridLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QTreeWidgetItem>
#include <QPalette>
#include <QBrush>

#include "../../signal/mrun.h"
#include "ft_runplot.h"
#include "ft_resultvisualization.h"
#include "ft_fitlist.h"

#include "../utils/minimizablewidget.h"

class DataItemTreeView;
class FT_ModelingSettingsTable;
class FT_FitParamTable;
class FT_NumericParamTable;
class FT_DataVisualization;
class FT_SimSettings;
class MRunDetailsWidget;

/**
 * @brief The FitCreator class provides a GUI for the generation
 * of FitRuns. It allows the selection of measurement runs and
 * fit parameters. Its shown as a subwindow in the FitTools GUI.
 */
class FitCreator : public QWidget
{
    Q_OBJECT

    class SettingsTree : public QTreeWidget
    {
    public:
        SettingsTree(QWidget *parent = 0) : QTreeWidget(parent)
        {
            QPushButton *button = new QPushButton();
            QPalette tpalette(palette());
            QPalette bpalette(button->palette());
            QBrush bbrush(bpalette.background());
            tpalette.setBrush(QPalette::Active, QPalette::Base, bbrush);
            setPalette(tpalette);
            delete button;
        }
    };

public:
    explicit FitCreator(QWidget *parent = 0);
    ~FitCreator();

    inline QAction* viewAction(){return m_viewAction;}

private:
    QString m_title;
    QString m_iconLocation;

    /** @brief action for parent gui, used for changed visibility of widget */
    QAction* m_viewAction;

    DataItemTreeView* treeView;
    MRunDetailsWidget *detailsView;

    MRun *currentRun;

    FT_RunPlot *runPlot;
    FT_ResultVisualization *resultVisualization;
    FT_FitList *fitList;
    FT_DataVisualization *dataVisualization;
    FT_SimSettings *simSettings;

    /// @brief a list of currently selected MRun IDs
    QList<int> m_selectedRunIDs;

    //QSplitter* m_splitV;
    QGridLayout* m_mainLayout;

    //QGridLayout* m_paramLayout;
    //QWidget* paramLayoutDummy;

    QSplitter *mainSplitter;
    QSplitter *treeDetailsSplitter;
    QSplitter *splitterFitListData;
    QSplitter *splitterPlotSettings;
    QSplitter *splitterListVisualization;
    QSplitter *splitterHPlSeFlRv;

    FT_ModelingSettingsTable* modelingTable;
    FT_FitParamTable* fitparamTable;
    FT_NumericParamTable* numparamTable;

    static const QString identifier_mainSplitter;
    static const QString identifier_treeDetailsSplitter;
    static const QString identifier_fitListDataSplitter;
    static const QString identifier_plotSettingsSplitter;
    static const QString identifier_listVisualizationSplitter;
    static const QString identifier_HPlSeFlRvSplitter;

public slots:
    void onCalcToolboxRecalc(QList<Signal::SType> typeList);

private slots:

    void onProcessingStateChanged(bool state);

    void onFitButtonReleased();
    void onSimButtonReleased();

    void onFitStateChanged(bool state);
    void onCancelButtonReleased();

    void onTreeViewSelectionChanged(QList<QTreeWidgetItem*> selection);

    void onSplitterMoved();

    void onGUISettingsChanged();
};

#endif // FITCREATOR_H
