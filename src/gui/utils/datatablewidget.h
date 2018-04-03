#ifndef DATATABLEWIDGET_H
#define DATATABLEWIDGET_H

#include <QWidget>
#include <QList>
#include <QGridLayout>
#include <QTableWidget>
#include <QLabel>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
//#include "datatablerunnable.h"
#include <QStandardItemModel>
#include <QTableView>

#include "../../signal/signal.h"

#include "customQwtPlot/baseplotcurve.h"
#include "customQwtPlot/signalplotintervalcurve.h"

/**
 * @brief The DataTableWidget class provides a table representation of a
 * list of plot curves. (can be accessed by right-click on plot)
 * @ingroup GUI-Utilities
 */
class DataTableWidget : public QWidget
{
    Q_OBJECT

    /** @brief list of plot curves */
    QList<BasePlotCurve*>* curves;
    QList<SignalPlotIntervalCurve*> *stdevCurves;

    /** @brief colNames */
    QStringList colNames;

    /** @brief main layout */
    QGridLayout* layMain;

    /** @brief table widget */
    QTableWidget* table;
    QTableView *tableView;

    /** @brief action for right click context menu */
    QAction* actionCopyToClipboard;

    QMenuBar* menuBar;

    QMenu* mOptions;
    QAction *actionShowStdev;
    QAction *actionOmitHeader;
    QAction *actionInterpolate;
    QLabel *labelNoCurveData;

    bool mrunPostMode;

    Signal::SType stype;

    void setup();

    int _number_precision;

public:
    explicit DataTableWidget(QList<BasePlotCurve*>* curves, Signal::SType stype = Signal::RAW, QWidget *parent = 0);
    explicit DataTableWidget(QList<BasePlotCurve*> *curves, QList<SignalPlotIntervalCurve*> *stdevCurves, Signal::SType stype = Signal::RAW, QWidget *parent = 0);

    void updateView();

signals:

public slots:

private slots:
    void onCopySelection();
    void onContextMenuEvent(QPoint pos);
    void onActionShowStdev();
    void onActionOmitHeader(bool checked);
    void onActionInterpolate();

};

#endif // DATATABLEWIDGET_H
